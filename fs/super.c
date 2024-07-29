#include <ylos/fs.h>
#include <ylos/buffer.h>
#include <ylos/device.h>
#include <ylos/assert.h>
#include <ylos/string.h>
#include <ylos/stat.h>
#include <ylos/debug.h>
#include <ylos/stdlib.h>
#include <ylos/task.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

#define SUPER_NR 16

static super_block_t super_table[SUPER_NR]; // 超级块表
static super_block_t *root;                 // 根文件系统超级块


// 从超级块表中查找一个空闲块
static super_block_t *get_free_super()
{
    for (size_t i = 0; i < SUPER_NR; i++)
    {
        super_block_t *sb = &super_table[i];
        if (sb->dev == EOF)
        {
            return sb;
        }
    }
    panic("no more super block!!!");
}

// 获得设备 dev 的超级块
super_block_t *get_super(dev_t dev)
{
    for (size_t i = 0; i < SUPER_NR; i++)
    {
        super_block_t *sb = &super_table[i];
        if (sb->dev == dev)
        {
            return sb;
        }
    }
    return NULL;
}

void put_super(super_block_t *super)
{
    if (!super)
        return;
    assert(super->count > 0);
    super->count--;
    if (super->count)
        return;

    super->dev = EOF;
    iput(super->imount);
    iput(super->iroot);

    for (int i = 0; i < super->desc->imap_blocks; i++)
        brelse(super->imaps[i]);
    for (int i = 0; i < super->desc->zmap_blocks; i++)
        brelse(super->zmaps[i]);

    brelse(super->buf);
}


// 读设备 dev 的超级块
super_block_t *read_super(dev_t dev)
{
    super_block_t *sb = get_super(dev);
    if (sb)
    {
        sb->count++;
        return sb;
    }

    LOGK("Reading super block of device %d\n", dev);

    // 获得空闲超级块
    sb = get_free_super();
    //读取超级块
    buffer_t *buf = bread(dev, 1);

    sb->buf = buf;
    sb->desc = (super_desc_t *)buf->data;
    sb->dev = dev;
    sb->count = 1;

    assert(sb->desc->magic == MINIX1_MAGIC);

    memset(sb->imaps, 0, sizeof(sb->imaps));
    memset(sb->zmaps, 0, sizeof(sb->zmaps));

    //读取inode位图
    int idx = 2; //块位图从第 2 块开始， 第 0 块 引导块， 第 1 块 超级块
    for (int i = 0; i < sb->desc->imap_blocks; i++)
    {
        assert(i < IMAP_NR);
        if ((sb->imaps[i] = bread(dev, idx)))
        {
            idx++;
        }
        else
        {
            break;
        }
    }

    //读取 块位图
    for (int i = 0; i < sb->desc->zmap_blocks; i++)
    {
        assert(i < ZMAP_NR);
        if ((sb->zmaps[i] = bread(dev, idx)))
        {
            idx++;
        }
        else
        {
            break;
        }
    }
    return sb;
}

// 挂载根文件系统
static void mount_root()
{
    LOGK("Mount root file system...\n");
    // 假设主硬盘第一个分区是根文件系统
    device_t *device = device_find(DEV_IDE_PART, 0);
    assert(device);

    // 读根文件系统超级块
    root = read_super(device->dev);

    //初始化根目录 inode
    root->iroot = iget(device->dev, 1);   //获取根目录 inode
    root->imount = iget(device->dev, 1);  //根目录挂载 inode

}   

void super_init()
{
    // device_t *device = device_find(DEV_IDE_PART, 0);
    // assert(device);

    // buffer_t *boot = bread(device->dev, 0);
    // buffer_t *super = bread(device->dev, 1);

    // super_desc_t *sb = (super_desc_t *)super->data;
    // assert(sb->magic == MINIX1_MAGIC);

    // // inode 位图
    // buffer_t *imap = bread(device->dev, 2);
    // // 块位图
    // buffer_t *zmap = bread(device->dev, 2 + sb->imap_blocks);

    // //读取第一个inode块
    // buffer_t *buf1 = bread(device->dev, 2 + sb->imap_blocks + sb->zmap_blocks);
    // inode_desc_t *inode = (inode_desc_t *)buf1->data;

    // buffer_t *buf2 = bread(device->dev, inode->zone[0]);

    // dentry_t *dir = (dentry_t *)buf2->data;
    // inode_desc_t *helloi = NULL;

    // while (dir->nr)
    // {
    //     LOGK("inode %04d, name %s\n", dir->nr, dir->name);
    //     if (!strcmp(dir->name, "hello.txt"))
    //     {
    //         helloi = &((inode_desc_t *)buf1->data)[dir->nr - 1];
    //     }
    //     dir++;
    // }

    // buffer_t *buf3 = bread(device->dev, helloi->zone[0]);
    // LOGK("content %s", buf3->data);

    // strcpy(buf3->data, "This is modified content!!!\n");
    // buf3->dirty = true;
    // bwrite(buf3);

    // helloi->size = strlen(buf3->data);
    // buf1->dirty = true;
    // bwrite(buf1);

    for (size_t i = 0; i < SUPER_NR; i++)
    {
        super_block_t *sb = &super_table[i];
        sb->dev = EOF;
        sb->desc = NULL;
        sb->buf = NULL;
        sb->iroot = NULL;
        sb->imount = NULL;
        list_init(&sb->inode_list);
    }
    mount_root();
}

int sys_mount(char *devname, char *dirname, int flags)
{
    LOGK("mount %s to %s\n", devname, dirname);

    inode_t *devinode = NULL;
    inode_t *dirinode = NULL;
    super_block_t *super = NULL;
    devinode = namei(devname);
    if (!devinode)
        goto rollback;

    if (!ISBLK(devinode->desc->mode))
        goto rollback;

    dev_t dev = devinode->desc->zone[0];

    dirinode = namei(dirname);
    if (!dirinode)
        goto rollback;

    if (!ISDIR(dirinode->desc->mode))
        goto rollback;

    if (dirinode->count != 1 || dirinode->mount)
        goto rollback;

    super = read_super(dev);
    if (super->imount)
        goto rollback;

    super->iroot = iget(dev, 1);
    super->imount = dirinode;
    dirinode->mount = dev;
    iput(devinode);
    return 0;

rollback:
    put_super(super);
    iput(devinode);
    iput(dirinode);
    return EOF;
}

int sys_umount(char *target)
{
    LOGK("umount %s\n", target);
    inode_t *inode = NULL;
    super_block_t *super = NULL;
    int ret = EOF;

    inode = namei(target);
    if (!inode)
        goto rollback;

    if (!ISBLK(inode->desc->mode) && (inode->nr != 1))
        goto rollback;

    if (inode == root->imount)
        goto rollback;

    dev_t dev = inode->dev;
    if (ISBLK(inode->desc->mode))
    {
        dev = inode->desc->zone[0];
    }

    super = get_super(dev);
    if (!super->imount)
        goto rollback;

    if (!super->imount->mount)
    {
        LOGK("warning super block mount = 0\n");
    }

    if (list_size(&super->inode_list) > 1)
        goto rollback;

    iput(super->iroot);
    super->iroot = NULL;

    super->imount->mount = 0;
    iput(super->imount);
    super->imount = NULL;
    ret = 0;

rollback:
    put_super(super);
    iput(inode);
    return ret;
}

int devmkfs(dev_t dev, u32 icount)
{
    super_block_t *super = NULL;
    buffer_t *buf = NULL;
    int ret = EOF;

    int total_black = device_ioctl(dev, DEV_CMD_SECTOR_COUNT, NULL, 0) / BLOCK_SECS;
    assert(total_black);
    assert(icount < total_black);

    if (!icount)
    {
        icount = total_black / 3;
    }

    super = get_free_super();
    super->dev = dev;
    super->count = 1;

    buf = bread(dev, 1);
    super->buf = buf;
    buf->dirty = true;

    super_desc_t *desc = (super_desc_t *)buf->data;
    super->desc = desc;

    int inode_blocks = div_round_up(icount * sizeof(inode_desc_t), BLOCK_SIZE);
    desc->inodes = icount;
    desc->zones = total_black;
    desc->imap_blocks = div_round_up(icount, BLOCK_BITS);

    int zcount = total_black - desc->imap_blocks - inode_blocks - 2;
    desc->zmap_blocks = div_round_up(zcount, BLOCK_BITS);

    desc->firstdatazone = 2 + desc->imap_blocks + desc->zmap_blocks + inode_blocks;
    desc->log_zone_size = 0;
    desc->max_size = BLOCK_SIZE * TOTAL_BLOCK;
    desc->magic = MINIX1_MAGIC;

    memset(super->imaps, 0, sizeof(super->imaps));
    memset(super->zmaps, 0, sizeof(super->zmaps));

    int idx = 2;
    for (int i = 0; i < super->desc->imap_blocks; i++)
    {
        if ((super->imaps[i] = bread(dev, idx)))
        {
            memset(super->imaps[i]->data, 0, BLOCK_SIZE);
            super->imaps[i]->dirty = true;
            idx++;
        }
        else
        {
            break;
        }
    }
    for (int i = 0; i < super->desc->zmap_blocks; i++)
    {
        if ((super->zmaps[i] = bread(dev, idx)))
        {
            memset(super->zmaps[i]->data, 0, BLOCK_SIZE);
            super->zmaps[i]->dirty = true;
            idx++;
        }
        else
        {
            break;
        }
    }

    idx = balloc(dev);

    idx = ialloc(dev);
    idx = ialloc(dev);

    int counts[] = {
        icount + 1,
        zcount,
    };

    buffer_t *maps[] = {
        super->imaps[super->desc->imap_blocks -1],
        super->zmaps[super->desc->zmap_blocks -1],
    };
    for (size_t i = 0; i < 2; i++)
    {
        int count = counts[i];
        buffer_t *map = maps[i];
        map->dirty = true;
        int offset = count % (BLOCK_BITS);
        int begin = (offset / 8);
        char *ptr = (char *)map->data + begin;
        memset(ptr + 1, 0XFF, BLOCK_SIZE - begin -1);
        int bits = 0x80;
        char data = 0;
        int remain = 8 - offset % 8;
        while (remain--)
        {
            data |= bits;
        }
        ptr[0] = data;
    }

    task_t *task = running_task();

    inode_t *iroot = new_inode(dev, 1);
    super->iroot = iroot;

    iroot->desc->mode = (0777 & ~task->umask) | IFDIR;
    iroot->desc->size = sizeof(dentry_t) * 2;
    iroot->desc->nlinks = 2;

    buf = bread(dev, bmap(iroot, 0, true));
    buf->dirty = true;

    dentry_t *entry = (dentry_t *)buf->data;
    memset(entry, 0,BLOCK_SIZE);

    strcpy(entry->name, ".");
    entry->nr = iroot->nr;

    entry++;
    strcpy(entry->name, "..");
    entry->nr = iroot->nr;
    
    brelse(buf);
    ret = 0;
rollback:
    put_super(super);
    return ret;
}

int sys_mkfs(char *devname, int icount)
{
    inode_t *inode = NULL;
    int ret = EOF;

    inode = namei(devname);
    if (!inode)
        goto rollback;

    if (!ISBLK(inode->desc->mode))
        goto rollback;

    dev_t dev = inode->desc->zone[0];
    assert(dev);
    ret = devmkfs(dev, icount);

rollback:
    iput(inode);
    return ret;
}