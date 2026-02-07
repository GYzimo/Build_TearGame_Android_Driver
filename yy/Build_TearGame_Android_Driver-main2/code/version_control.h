#ifndef VERSION_CONTROL_H
#define VERSION_CONTROL_H

#include <linux/version.h>

// 硬件断点功能开关 - 启用硬件断点支持
#define CONFIG_HW_BREAKPOINT_MODE 1

// 版本信息
#define DRIVER_VERSION "1.0.0"
#define DRIVER_NAME "TearGame"

// 内核版本检查宏
#if LINUX_VERSION_CODE >= KERNEL_VERSION(6,6,0)
#define LINUX_6_6_OR_NEWER 1
#else
#define LINUX_6_6_OR_NEWER 0
#endif

// 调试打印宏
#ifdef DEBUG
#define PRINT_DEBUG(fmt, ...) printk(KERN_DEBUG "[TearGame-HWBP] " fmt, ##__VA_ARGS__)
#else
#define PRINT_DEBUG(fmt, ...) do { } while (0)
#endif

#endif // VERSION_CONTROL_H
