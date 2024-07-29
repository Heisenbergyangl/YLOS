#ifndef YLOS_BUFFER_H
#define YLOS_BUFFER_H

#include <ylos/types.h>
#include <ylos/list.h>
#include <ylos/mutex.h>

// #define HASH_COUNT 31      // 应该是个素数
// #define MAX_BUF_COUNT 4096 // 最大缓冲数量

#define BLOCK_SIZE 1024 //块大小
#define SECTOR_SIZE 512 //扇区大小
#define BLOCK_SECS (BLOCK_SIZE / SECTOR_SIZE) //一块占两个扇区

// typedef struct bdesc_t
// {
//     u32 count; // 缓存数量
//     u32 size;  // 缓存大小

//     list_t free_list;              // 已经申请未使用的块
//     list_t idle_list;              // 缓存链表，被释放的块
//     list_t wait_list;              // 等待进程链表
//     list_t hash_table[HASH_COUNT]; // 缓存哈希表
// } bdesc_t;

typedef struct buffer_t
{
    char *data;        // 数据区
    // bdesc_t *desc;     // 描述符指针
    dev_t dev;         // 设备号
    idx_t block;       // 块号
    int count;         // 引用计数
    list_node_t hnode; // 哈希表拉链节点
    list_node_t rnode; // 缓冲节点
    lock_t lock;       // 锁
    bool dirty;        // 是否与磁盘不一致
    bool valid;        // 是否有效
} buffer_t;

buffer_t *getblk(dev_t dev, idx_t block);
buffer_t *bread(dev_t dev, idx_t block);
void bwrite(buffer_t *buf);
void brelse(buffer_t *buf);
// void bdirty(buffer_t *buf, bool dirty);

#endif