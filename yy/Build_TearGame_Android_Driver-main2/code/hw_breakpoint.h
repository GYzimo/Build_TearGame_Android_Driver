#ifndef HW_BREAKPOINT_H
#define HW_BREAKPOINT_H

#include <linux/perf_event.h>
#include "comm.h"
#include "version_control.h"

// 移除对CONFIG_HW_BREAKPOINT_MODE的依赖，直接声明函数
int khack_hw_breakpoint_init(struct perf_event_attr *attr);
int khack_hw_bp_module_init(void);
void khack_hw_bp_module_exit(void);
int handle_hw_breakpoint_control(PX5 ctl);
int handle_hw_breakpoint_get_hits(PX8 ctl, unsigned long arg);

#endif // HW_BREAKPOINT_H
