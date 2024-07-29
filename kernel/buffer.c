#include <ylos/buffer.h>
#include <ylos/memory.h>
#include <ylos/arena.h>
#include <ylos/debug.h>
#include <ylos/assert.h>
#include <ylos/device.h>
#include <ylos/string.h>
#include <ylos/task.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define BUFFER_DESC_NR 3 // 描述符数量 1024 2048 4096

#define HASH_COUNT 31 //应该是个素数

static buffer_t *buffer_start = (buffer_t *)KERNEL_BUFFER_MEM;
static u32 buffer_count = 0;

// 记录当前buffer_t 结构体位置
static buffer_t *buffer_ptr = (buffer_t *)KERNEL_BUFFER_MEM;

//记录当前数据缓冲区的位置
static void *buffer_data = (void *)(KERNEL_BUFFER_MEM + KERNEL_BUFFER_SIZE - BLOCK_SIZE);

static list_t free_list;    //缓存链表，被释放的块
static list_t wait_list;    //等待进程链表
static list_t hash_table[HASH_COUNT];    //缓存哈希表


// 哈希函数
u32 hash(dev_t dev, idx_t block)
{
    return (dev ^ block) % HASH_COUNT;
}

// 从哈希表中查找 buffer
static buffer_t *get_from_hash_table(dev_t dev, idx_t block)
{
    u32 idx = hash(dev, block);
    list_t *list = &hash_table[idx];
    buffer_t *buf = NULL;

    for (list_node_t *node = list->head.next; node != &list->tail; node = node->next)
    {
        buffer_t *ptr = element_entry(buffer_t, hnode, node);
        if (ptr->dev == dev && ptr->block == block)
        {
            buf = ptr;
            break;
        }
    }

    if (!buf)
    {
        return NULL;
    }

    // 如果 buf 在空闲列表中，则移除
    if (list_search(&free_list, &buf->rnode))
    {
        list_remove(&buf->rnode);
    }

    return buf;
}

// 将 buf 放入哈希表
static void hash_locate(buffer_t *buf)
{
    u32 idx = hash(buf->dev, buf->block);
    list_t *list = &hash_table[idx];
    assert(!list_search(list, &buf->hnode));
    list_push(list, &buf->hnode);
}

// 将 buf 从哈希表中移除
static void hash_remove(buffer_t *buf)
{
    u32 idx = hash(buf->dev, buf->block);
    list_t *list = &hash_table[idx];
    assert(!list_search(list, &buf->hnode));
    list_remove(&buf->hnode);
 
}


// // 初始化缓冲
static buffer_t *get_new_buffer()
{
    buffer_t *buf = NULL;


    if ((u32)buffer_ptr + sizeof(buffer_t) < (u32)buffer_data)
    {
        buf = buffer_ptr;
        buf->data = buffer_data;
        buf->dev = EOF;
        buf->block = 0;
        buf->count = 0;
        buf->dirty = false;
        buf->valid = false;
        lock_init(&buf->lock);
        buffer_count++;
        buffer_ptr++;
        buffer_data -= BLOCK_SIZE;
        LOGK("buffer count %d\n", buffer_count);
    }

    return buf;
}


// 获得空闲的 buffer
static buffer_t *get_free_buffer()
{
    buffer_t *buf = NULL;
    while (true)
    {
        //如果不空， 从空闲列表中获得缓存
        buf = get_new_buffer();
        if (buf)
        {
            return buf;
        }

        // 否则，从空闲列表里取得
        if (!list_empty(&free_list))
        {
            // 取最远未访问过的块
            buf = element_entry(buffer_t, rnode, list_popback(&free_list));
            hash_remove(buf);
            buf->valid = false;
            return buf;
        }
        task_block(running_task(), &wait_list, TASK_BLOCKED);
    }
}

// 获得设备 dev，第 block 对应的缓冲
buffer_t *getblk(dev_t dev, idx_t block)
{
    buffer_t *buf = get_from_hash_table(dev, block);
    if (buf)
    {
        buf->count++;
        return buf;
    }

    buf = get_free_buffer();
    assert(buf->count == 0);
    assert(buf->dirty == 0);

    buf->count = 1;
    buf->dev = dev;
    buf->block = block;
    hash_locate(buf);
    return buf;
}

// 读取 dev 的 block 块
buffer_t *bread(dev_t dev, idx_t block)
{
    buffer_t *buf = getblk(dev, block);

    assert(buf != NULL);
    if (buf->valid)
    {
        return buf;
    }

    lock_acquire(&buf->lock);

    if(!buf->valid)
    {
        device_request(
        buf->dev,
        buf->data,
        BLOCK_SECS,
        buf->block * BLOCK_SECS,
        0,
        REQ_READ);

        buf->dirty = false;
        buf->valid = true;
    }
    
    lock_release(&buf->lock);
    return buf;
}

// 写缓冲
void bwrite(buffer_t *buf)
{
    assert(buf);
    if (!buf->dirty)
        return;

    device_request(
        buf->dev, 
        buf->data, 
        BLOCK_SECS,
        buf->block * BLOCK_SECS, 
        0, 
        REQ_WRITE);


    buf->dirty = false;
    buf->valid = true;
}

// 释放缓冲
void brelse(buffer_t *buf)
{
    if (!buf)
        return;

    // buf->count--;
    // assert(buf->count >= 0);

    // if (!buf->count)
    // {
    //     if (buf->rnode.next)
    //     {
    //         list_remove(&buf->rnode);
    //     }
    //     list_push(&free_list, &buf->rnode);
    // }

    // if (buf->dirty) 
    // {
    //     bwrite(buf);
    // }

    if (buf->dirty)
    {
        bwrite(buf);
    }
    buf->count--;
    assert(buf->count >= 0);

    if (buf->count)
    {
        return;
    }

    assert(!buf->rnode.next);
    assert(!buf->rnode.prev);

    list_push(&free_list, &buf->rnode);
    
    if (!list_empty(&wait_list))
    {
        task_t *task = element_entry(task_t, node, list_popback(&wait_list));
        task_unblock(task);
    }
}

// err_t bdirty(buffer_t *buf, bool dirty)
// {
//     buf->dirty = dirty;
// }

void buffer_init()
{
    LOGK("buffer_t size is %d\n", sizeof(buffer_t));

    // 初始化空闲链表
    list_init(&free_list);
    // 初始化等待进程链表
    list_init(&wait_list);

    // 初始化哈希表
    for (size_t i = 0; i < HASH_COUNT; i++)
    {
        list_init(&hash_table[i]);
    }

}
