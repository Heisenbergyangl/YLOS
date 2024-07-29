#include <ylos/list.h>
#include <ylos/assert.h>

// 初始化链表
void list_init(list_t *list)
{
    list->head.prev = NULL;
    list->tail.next = NULL;
    list->head.next = &list->tail;
    list->tail.prev = &list->head;
}

// 在 anchor 结点前插入结点 node
void list_insert_before(list_node_t *anchor, list_node_t *node)
{
    node->prev = anchor->prev;
    node->next = anchor;

    anchor->prev->next = node;
    anchor->prev = node;
}

// 在 anchor 结点后插入结点 node
void list_insert_after(list_node_t *anchor, list_node_t *node)
{
    node->prev = anchor;
    node->next = anchor->next;

    anchor->next->prev = node;
    anchor->next = node;
}

// 插入到头结点后
void list_push(list_t *list, list_node_t *node)
{
    assert(!list_search(list, node));
    list_insert_after(&list->head, node);
}

// 移除头结点后的结点
list_node_t *list_pop(list_t *list)
{
    assert(!list_empty(list));

    list_node_t *node = list->head.next;
    list_remove(node);

    return node;
}

// 插入到尾结点前
void list_pushback(list_t *list, list_node_t *node)
{
    assert(!list_search(list, node));
    list_insert_before(&list->tail, node);
}

// 移除尾结点前的结点
list_node_t *list_popback(list_t *list)
{
    assert(!list_empty(list));

    list_node_t *node = list->tail.prev;
    list_remove(node);

    return node;
}

// 查找链表中结点是否存在
bool list_search(list_t *list, list_node_t *node)
{
    list_node_t *next = list->head.next;
    while (next != &list->tail)
    {
        if (next == node)
            return true;
        next = next->next;
    }
    return false;
}

// 从链表中删除结点
void list_remove(list_node_t *node)
{
    assert(node->prev != NULL);
    assert(node->next != NULL);

    node->prev->next = node->next;
    node->next->prev = node->prev;
    node->next = NULL;
    node->prev = NULL;
}

// 判断链表是否为空
bool list_empty(list_t *list)
{
    return (list->head.next == &list->tail);
}

// 获得链表长度
u32 list_size(list_t *list)
{
    list_node_t *next = list->head.next;
    u32 size = 0;
    while (next != &list->tail)
    {
        size++;
        next = next->next;
    }
    return size;
}

// 链表插入排序
void list_insert_sort(list_t *list, list_node_t *node, int offset)
{
    // 从链表找到第一个比当前节点 key 点更大的节点，进行插入到前面
    list_node_t *anchor = &list->tail;

    int key = element_node_key(node, offset);
    for (list_node_t *ptr = list->head.next; ptr != &list->tail; ptr = ptr->next)
    {
        int compare = element_node_key(ptr, offset);
        if (compare > key)
        {
            anchor = ptr;
            break;
        }
    }

    assert(node->next == NULL);
    assert(node->prev == NULL);

    // 插入链表
    list_insert_before(anchor, node);
}


//进行链表的测试
// #include <ylos/memory.h>
// #include <ylos/debug.h>

// #define LOGK(fmt, args...) DEBUGK(fmt, ##args)

// void list_test()
// {
//     u32 count = 3;
//     list_t holder;

//     list_t *list = &holder;

//     list_init(list);

//     list_node_t *node;

//     while (count--)
//     {
//         node = (list_node_t *)alloc_kpage(1);
//         list_push(list, node);
//     }

//     while (!list_empty(list))
//     {
//         node = list_pop(list);
//         free_kpage((u32)node, 1);
//     }

//     count = 3;

//     while (count--)
//     {
//         node = (list_node_t *)alloc_kpage(1);
//         list_pushback(list, node);
//     }

//     LOGK("list size %d\n", list_size(list));

//     while (!list_empty(list))
//     {
//         node = list_popback(list);
//         free_kpage((u32)node, 1);
//     }

//     node = (list_node_t *)alloc_kpage(1);
//     list_pushback(list, node);

//     LOGK("search node 0x%p --> %d\n", node, list_search(list, node));
//     LOGK("search node 0x%p --> %d\n", 0, list_search(list, 0));

//     list_remove(node);
//     free_kpage((u32)node, 1);
// }