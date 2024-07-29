#include <ylos/task.h>

// extern task_t *task_table[TASK_NR];

mode_t sys_umask(mode_t mask)
{
    task_t *task = running_task();
    mode_t old = task->umask;
    task->umask = mask & 0777;
    return old;
}