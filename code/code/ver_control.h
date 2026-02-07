#ifndef VERSION_CONTROL_H_
#define VERSION_CONTROL_H_

// 独立内核模块入口模式
#define CONFIG_MODULE_GUIDE_ENTRY

// 生成proc用户层交互节点文件
#define CONFIG_USE_PROC_FILE_NODE
// 隐蔽通信密钥
#define CONFIG_PROC_NODE_AUTH_KEY "c2a2b5792edd296763fdfc72cff44380"

// 打印内核调试信息
//#define CONFIG_DEBUG_PRINTK

#ifndef KERNEL_VERSION
#define KERNEL_VERSION(a,b,c) (((a) << 16) + ((b) << 8) + (c))
#endif

// 使用内核自带的 LINUX_VERSION_CODE，而不是硬编码
#include <linux/version.h>
#ifndef MY_LINUX_VERSION_CODE 
#define MY_LINUX_VERSION_CODE LINUX_VERSION_CODE
#endif

// 为了兼容性，定义版本范围检查宏
#define KERNEL_VERSION_RANGE(min_a,min_b,max_a,max_b) \
    (MY_LINUX_VERSION_CODE >= KERNEL_VERSION(min_a,min_b,0) && \
     MY_LINUX_VERSION_CODE < KERNEL_VERSION(max_a,max_b,0))

#ifdef CONFIG_DEBUG_PRINTK
#define printk_debug printk
#else
static inline void printk_debug(char *fmt, ...) {}
#endif

#endif /* VERSION_CONTROL_H_ */
