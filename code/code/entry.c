#include <linux/module.h>
#include <linux/tty.h>
#include <linux/miscdevice.h>
#include <linux/random.h>
#include "comm.h"
#include "memory.h"
#include "process.h"
#include "hw_breakpoint.h"
#include "version_control.h"

// 随机设备名长度
#define DEV_NAME_LEN 16

// 全局设备名缓冲区
static char g_dev_name[DEV_NAME_LEN + 1];

// 随机字符串生成函数
static void gen_rand_name(char *buf, size_t len)
{
	const char charset[] = "abcdefghijklmnopqrstuvwxyz0123456789";
	unsigned int i;
	
	for (i = 0; i < len; i++) {
		unsigned int idx = get_random_u32() % (sizeof(charset) - 1);
		buf[i] = charset[idx];
	}
	buf[len] = '\0';
}

// 授权用户标识
#define AUTHORIZED_UID 0 // 这里可以设置为特定用户的UID，0表示root

int dev_open(struct inode *inode, struct file *file)
{
	// 检查调用进程的UID
	if (current->cred->uid.val != AUTHORIZED_UID) {
		// 拒绝非授权用户的访问
		return -EPERM;
	}
	
	// 可以添加更多的访问控制检查，例如：
	// - 检查进程名称
	// - 检查进程路径
	// - 检查特定的环境变量或标识
	
	return 0;
}

int dev_close(struct inode *inode, struct file *file)
{
	return 0;
}

// 浮点寄存器修改函数
static int set_reg_f(pid_t pid, uintptr_t addr, int idx, uint32_t val)
{
	struct task_struct *task;

	task = get_pid_task(find_vpid(pid), PIDTYPE_PID);
	if (!task) {
		return -ESRCH;
	}

	put_task_struct(task);
	return 0;
}

// 双精度浮点寄存器修改函数
static int set_reg_d(pid_t pid, uintptr_t addr, int idx, uint64_t val)
{
	struct task_struct *task;

	task = get_pid_task(find_vpid(pid), PIDTYPE_PID);
	if (!task) {
		return -ESRCH;
	}

	put_task_struct(task);
	return 0;
}

// 浮点寄存器读取函数
static int get_reg_f(pid_t pid, uintptr_t addr, int idx, uint32_t *val)
{
	struct task_struct *task;

	task = get_pid_task(find_vpid(pid), PIDTYPE_PID);
	if (!task) {
		return -ESRCH;
	}

	*val = 0;
	
	put_task_struct(task);
	return 0;
}

// 双精度浮点寄存器读取函数
static int get_reg_d(pid_t pid, uintptr_t addr, int idx, uint64_t *val)
{
	struct task_struct *task;

	task = get_pid_task(find_vpid(pid), PIDTYPE_PID);
	if (!task) {
		return -ESRCH;
	}

	*val = 0;
	
	put_task_struct(task);
	return 0;
}

// 预定义的授权密钥
static const char *AUTH_KEY = "your_secure_key_here";

long dev_ioctl(struct file *const file, unsigned int const cmd, unsigned long const arg)
{
	static X1 x1;
	static X2 x2;
	static char key[0x100] = {0};
	static char name[0x100] = {0};
	static bool auth = false;
	static X5 x5;
	static X8 x8;
	static X9 x9;
	static X10 x10;
	static X11 x11;
	static X12 x12;

	switch (cmd)
	{
	case OP_A:
	{
		if (copy_from_user(key, (void __user *)arg, sizeof(key) - 1) != 0)
		{
			return -EFAULT;
		}
		key[sizeof(key) - 1] = '\0';
		
		if (strcmp(key, AUTH_KEY) == 0) {
			auth = true;
		} else {
			auth = false;
			return -EPERM;
		}
		break;
	}
	case OP_B:
	{
		if (!auth)
			return -EPERM;
		if (copy_from_user(&x1, (void __user *)arg, sizeof(x1)) != 0)
		{
			return -EFAULT;
		}
		if (read_process_memory(x1.a, x1.b, x1.c, x1.d) == false)
		{
			return -EFAULT;
		}
		break;
	}
	case OP_C:
	{
		if (!auth)
			return -EPERM;
		if (copy_from_user(&x1, (void __user *)arg, sizeof(x1)) != 0)
		{
			return -EFAULT;
		}
		if (write_process_memory(x1.a, x1.b, x1.c, x1.d) == false)
		{
			return -EFAULT;
		}
		break;
	}
	case OP_D:
	{
		if (!auth)
			return -EPERM;
		if (copy_from_user(&x2, (void __user *)arg, sizeof(x2)) != 0 || copy_from_user(name, (void __user *)x2.b, sizeof(name) - 1) != 0)
		{
			return -EFAULT;
		}
		name[sizeof(name) - 1] = '\0';
		x2.c = get_module_base(x2.a, name);
		if (copy_to_user((void __user *)arg, &x2, sizeof(x2)) != 0)
		{
			return -EFAULT;
		}
		break;
	}
	case OP_E:
	{
		if (!auth)
			return -EPERM;
		if (copy_from_user(&x5, (void __user *)arg, sizeof(x5)) != 0)
		{
			return -EFAULT;
		}
		if (handle_hw_breakpoint_control(&x5) != 0)
		{
			return -EFAULT;
		}
		break;
	}
	case OP_F:
	{
		if (!auth)
			return -EPERM;
		if (copy_from_user(&x8, (void __user *)arg, sizeof(x8)) != 0)
		{
			return -EFAULT;
		}
		if (handle_hw_breakpoint_get_hits(&x8, arg) != 0)
		{
			return -EFAULT;
		}
		break;
	}
	case OP_G:
	{
		if (!auth)
			return -EPERM;
		if (copy_from_user(&x9, (void __user *)arg, sizeof(x9)) != 0)
		{
			return -EFAULT;
		}
		if (set_reg_f(x9.a, x9.b, x9.c, x9.d) != 0)
		{
			return -EFAULT;
		}
		break;
	}
	case OP_H:
	{
		if (!auth)
			return -EPERM;
		if (copy_from_user(&x10, (void __user *)arg, sizeof(x10)) != 0)
		{
			return -EFAULT;
		}
		if (set_reg_d(x10.a, x10.b, x10.c, x10.d) != 0)
		{
			return -EFAULT;
		}
		break;
	}
	case OP_I:
	{
		if (!auth)
			return -EPERM;
		if (copy_from_user(&x11, (void __user *)arg, sizeof(x11)) != 0)
		{
			return -EFAULT;
		}
		if (get_reg_f(x11.a, x11.b, x11.c, &x11.d) != 0)
		{
			return -EFAULT;
		}
		if (copy_to_user((void __user *)arg, &x11, sizeof(x11)) != 0)
		{
			return -EFAULT;
		}
		break;
	}
	case OP_J:
	{
		if (!auth)
			return -EPERM;
		if (copy_from_user(&x12, (void __user *)arg, sizeof(x12)) != 0)
		{
			return -EFAULT;
		}
		if (get_reg_d(x12.a, x12.b, x12.c, &x12.d) != 0)
		{
			return -EFAULT;
		}
		if (copy_to_user((void __user *)arg, &x12, sizeof(x12)) != 0)
		{
			return -EFAULT;
		}
		break;
	}
	default:
		return -EINVAL;
	}
	return 0;
}

struct file_operations dev_fops = {
	.owner = THIS_MODULE,
	.open = dev_open,
	.release = dev_close,
	.unlocked_ioctl = dev_ioctl,
};

// 全局 miscdevice 结构
static struct miscdevice dev_misc;

int __init init_mod(void)
{
	int ret;
	
	// 生成随机设备名
	gen_rand_name(g_dev_name, DEV_NAME_LEN);
	
	// 初始化 miscdevice 结构
	dev_misc.minor = MISC_DYNAMIC_MINOR;
	dev_misc.name = g_dev_name;
	dev_misc.fops = &dev_fops;
	
	// 初始化硬件断点模块
	ret = khack_hw_bp_module_init();
	if (ret != 0) {
		return ret;
	}
	
	ret = misc_register(&dev_misc);
	if (ret != 0) {
		khack_hw_bp_module_exit();
		return ret;
	}
	
	// 添加节点隐藏代码
	// 使用 unlink 移除 /dev 中的节点，但保持设备注册状态
	char dev_path[64];
	snprintf(dev_path, sizeof(dev_path), "/dev/%s", g_dev_name);
	unlink(dev_path);
	
	return 0;
}

void __exit exit_mod(void)
{
	// 清理硬件断点模块
	khack_hw_bp_module_exit();
	
	misc_deregister(&dev_misc);
}

module_init(init_mod);
module_exit(exit_mod);

MODULE_DESCRIPTION("Generic Driver Module");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Unknown");
