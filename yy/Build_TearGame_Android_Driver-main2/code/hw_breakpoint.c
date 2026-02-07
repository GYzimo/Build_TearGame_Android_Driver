#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/perf_event.h>
#include <linux/hw_breakpoint.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/list.h>
#include <linux/spinlock.h>
#include <linux/sched/signal.h>
#include <linux/ktime.h>
#include <linux/version.h>

#include "hw_breakpoint.h"
#include "cvector.h"
#include "comm.h"
#include "version_control.h"

// Internal implementation of hw_breakpoint_init to avoid kernel symbol conflicts
int khack_hw_breakpoint_init(struct perf_event_attr *attr)
{
    // Initialize the perf_event_attr structure for hardware breakpoint
    memset(attr, 0, sizeof(*attr));

    attr->type = PERF_TYPE_BREAKPOINT;
    attr->size = sizeof(struct perf_event_attr);
    attr->pinned = 1;
    attr->disabled = 0;
    attr->exclude_kernel = 1;
    attr->exclude_hv = 1;
    attr->bp_len = 8; // 直接使用数值，避免依赖内核常量定义

    return 0;
}

// Internal struct to track our breakpoints
struct khack_hw_breakpoint {
    struct list_head list;
    struct perf_event *bp_event;
    pid_t pid; // Thread ID
    uintptr_t addr;
};

// Global list for our installed breakpoints
static LIST_HEAD(g_hw_breakpoints);
static DEFINE_MUTEX(g_hw_bp_mutex);

// Global buffer for hit events
static cvector g_hit_buffer = NULL;
static DEFINE_SPINLOCK(g_hit_buffer_lock);

// Forward declarations
static void breakpoint_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs);

static int add_hw_breakpoint(PX5 ctl) {
    struct task_struct *task;
    struct perf_event_attr attr;
    struct perf_event *bp_event;
    struct khack_hw_breakpoint *khack_bp;

    task = get_pid_task(find_vpid(ctl->a), PIDTYPE_PID);
    if (!task) {
        return -ESRCH;
    }

    khack_hw_breakpoint_init(&attr);
    attr.bp_addr = ctl->b;
    attr.bp_len = ctl->d;
    if (ctl->c == X3_A) {
        attr.bp_type = HW_BREAKPOINT_X;
    } else if (ctl->c == X3_B) {
        attr.bp_type = HW_BREAKPOINT_W;
    } else if (ctl->c == X3_C) {
        attr.bp_type = HW_BREAKPOINT_RW;
    } else {
        put_task_struct(task);
        return -EINVAL;
    }

    khack_bp = kmalloc(sizeof(*khack_bp), GFP_KERNEL);
    if (!khack_bp) {
        put_task_struct(task);
        return -ENOMEM;
    }

    bp_event = perf_event_create_kernel_counter(&attr, PERF_TYPE_BREAKPOINT, task, breakpoint_handler, NULL);
    if (IS_ERR(bp_event)) {
        int ret = PTR_ERR(bp_event);
        kfree(khack_bp);
        put_task_struct(task);
        return ret;
    }

    khack_bp->bp_event = bp_event;
    khack_bp->pid = ctl->a;
    khack_bp->addr = ctl->b;

    mutex_lock(&g_hw_bp_mutex);
    list_add_tail(&khack_bp->list, &g_hw_breakpoints);
    mutex_unlock(&g_hw_bp_mutex);

    put_task_struct(task);
    return 0;
}

static int remove_hw_breakpoint(PX5 ctl) {
    struct khack_hw_breakpoint *khack_bp, *tmp;
    bool found = false;

    mutex_lock(&g_hw_bp_mutex);
    list_for_each_entry_safe(khack_bp, tmp, &g_hw_breakpoints, list) {
        if (khack_bp->pid == ctl->a && khack_bp->addr == ctl->b) {
            perf_event_release_kernel(khack_bp->bp_event);
            list_del(&khack_bp->list);
            kfree(khack_bp);
            found = true;
            break;
        }
    }
    mutex_unlock(&g_hw_bp_mutex);

    if (found) {
        return 0;
    } else {
        return -ENOENT;
    }
}

static void breakpoint_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs) {
    unsigned long flags;
    X7 *hit_info;

    hit_info = kmalloc(sizeof(*hit_info), GFP_ATOMIC);
    if (!hit_info) {
        return;
    }

    hit_info->a = current->pid;
    hit_info->b = ktime_get_ns();
    hit_info->c = instruction_pointer(regs);
    
    // Copy register values
    memset(&hit_info->d, 0, sizeof(X6));
    
    // 复制寄存器值，仅保留关键寄存器
    hit_info->d.pc = regs->pc;
    hit_info->d.sp = regs->sp;
    hit_info->d.pstate = regs->pstate;
    
    spin_lock_irqsave(&g_hit_buffer_lock, flags);
    if (g_hit_buffer) {
        if (cvector_pushback(g_hit_buffer, &hit_info) != CVESUCCESS) {
            kfree(hit_info);
        }
    } else {
        kfree(hit_info);
    }
    spin_unlock_irqrestore(&g_hit_buffer_lock, flags);
}

int handle_hw_breakpoint_control(PX5 ctl) {
    switch (ctl->e) {
        case X4_A:
            return add_hw_breakpoint(ctl);
        case X4_B:
            return remove_hw_breakpoint(ctl);
        default:
            return -EINVAL;
    }
}

int handle_hw_breakpoint_get_hits(PX8 ctl, unsigned long arg) {
    unsigned long flags;
    size_t hits_to_copy, i;
    int ret = 0;

    spin_lock_irqsave(&g_hit_buffer_lock, flags);
    if (!g_hit_buffer) {
        spin_unlock_irqrestore(&g_hit_buffer_lock, flags);
        return -EINVAL;
    }

    hits_to_copy = cvector_length(g_hit_buffer);
    if (ctl->a < hits_to_copy) {
        hits_to_copy = ctl->a;
    }

    for (i = 0; i < hits_to_copy; ++i) {
        X7 **info_ptr;
        if (cvector_val_at(g_hit_buffer, i, &info_ptr) != 0) {
            ret = -EFAULT;
            goto out;
        }
        
        if (copy_to_user(&((PX7)ctl->b)[i], *info_ptr, sizeof(X7))) {
            ret = -EFAULT;
            // Don't clear buffer on partial copy failure
            goto out;
        }
    }

    // Clear the buffer after successful copy
    for (i = 0; i < cvector_length(g_hit_buffer); ++i) {
        X7 **info_ptr;
        if (cvector_val_at(g_hit_buffer, i, &info_ptr) == 0) {
            kfree(*info_ptr);
        }
    }
    cvector_destroy(g_hit_buffer);
    g_hit_buffer = cvector_create(sizeof(X7 *));

    ctl->a = hits_to_copy;
    if (copy_to_user((void __user *)arg, ctl, sizeof(*ctl))) {
        ret = -EFAULT;
    }

out:
    spin_unlock_irqrestore(&g_hit_buffer_lock, flags);
    return ret;
}

int khack_hw_bp_module_init(void) {
    unsigned long flags;
    spin_lock_irqsave(&g_hit_buffer_lock, flags);
    if (!g_hit_buffer) {
        g_hit_buffer = cvector_create(sizeof(X7 *));
    }
    spin_unlock_irqrestore(&g_hit_buffer_lock, flags);
    if (!g_hit_buffer) {
        return -ENOMEM;
    }
    return 0;
}

void khack_hw_bp_module_exit(void) {
    struct khack_hw_breakpoint *khack_bp, *tmp;
    unsigned long flags;

    mutex_lock(&g_hw_bp_mutex);
    list_for_each_entry_safe(khack_bp, tmp, &g_hw_breakpoints, list) {
        perf_event_release_kernel(khack_bp->bp_event);
        list_del(&khack_bp->list);
        kfree(khack_bp);
    }
    mutex_unlock(&g_hw_bp_mutex);

    spin_lock_irqsave(&g_hit_buffer_lock, flags);
    if (g_hit_buffer) {
        size_t i;
        for (i = 0; i < cvector_length(g_hit_buffer); ++i) {
            X7 **info_ptr;
            if (cvector_val_at(g_hit_buffer, i, &info_ptr) == 0) {
                kfree(*info_ptr);
            }
        }
        cvector_destroy(g_hit_buffer);
        g_hit_buffer = NULL;
    }
    spin_unlock_irqrestore(&g_hit_buffer_lock, flags);
}
