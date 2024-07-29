#include <ylos/interrupt.h>
#include <ylos/syscall.h>
#include <ylos/debug.h>
#include <ylos/arena.h>
#include <ylos/task.h>
#include <ylos/memory.h>
#include <ylos/stdio.h>
#include <ylos/fs.h>
#include <ylos/string.h>

#define LOGK(fmt, args...) DEBUGK(fmt, ##args)

void idle_thread()
{
    set_interrupt_state(true);
    u32 counter = 0;
    while (true)
    {
        // LOGK("idle task.... %d\n", counter++);
        asm volatile(
            "sti\n" // 开中断
            "hlt\n" // 关闭 CPU，进入暂停状态，等待外中断的到来
        );
        yield(); // 放弃执行权，调度执行其他任务
    }
}


extern void dev_init();

void init_thread()
{
    char temp[100];
    dev_init();
    task_to_user_mode();
}

void test_thread()
{
    set_interrupt_state(true);
    while (true)
    {
        sleep(10);
    }
}

