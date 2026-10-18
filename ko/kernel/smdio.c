#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/uaccess.h>
#include <linux/mdio.h>
#include <linux/phy.h>
#include <linux/mutex.h>
#include <linux/ww_mutex.h>

#include "compat.h"

#define DEVICE_NAME "smdio"
#define CLASS_NAME  "smdio_class"

// 定义ioctl命令
#define SMDIO_MAGIC 's'
#define SMDIO_READ_PHY_REG  _IOWR(SMDIO_MAGIC, 1, struct smdio_reg_msg)
#define SMDIO_WRITE_PHY_REG _IOW (SMDIO_MAGIC, 2, struct smdio_reg_msg)

// 用于传递phy寄存器地址和数值的结构体
struct smdio_reg_msg
{
	char mdioBus[16];

    int phyAddr;
	int mmd;
    unsigned short regAddr;
    unsigned short regValue;
};

static int major_num;
static struct cdev smdio_cdev;
static struct class *smdio_class = NULL;

// mii_bus指针
static struct mii_bus *s_smdio_bus = NULL;

// 打开设备
static int smdio_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "smdio: device opened.\r\n");

    return 0;
}

// 释放设备
static int smdio_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "smdio: device closed.\r\n");

    return 0;
}

// 核心：unlocked_ioctl函数
static long smdio_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct smdio_reg_msg msg;
    int ret = 0;

    switch (cmd)
	{
	case SMDIO_READ_PHY_REG:
		// 从用户空间复制数据
		if (copy_from_user(&msg, (void __user *)arg, sizeof(msg)))
		{
			ret = -EFAULT;

			goto label_exit;
		}

		s_smdio_bus = mdio_find_bus(msg.mdioBus);
		if (s_smdio_bus == NULL)
		{
			printk(KERN_ERR "smdio: MDIO bus not initialized\n");

			ret = -ENODEV;

			goto label_exit;
		}
		
		mutex_lock(&s_smdio_bus->mdio_lock);
		if (msg.mmd == 0x00)
			ret = __mdiobus_read(s_smdio_bus, msg.phyAddr, msg.regAddr);
		else
			ret = __mdiobus_c45_read(s_smdio_bus, msg.phyAddr, msg.mmd, msg.regAddr);
		mutex_unlock(&s_smdio_bus->mdio_lock);
		if (ret < 0)
		{
			printk(KERN_ERR "smdio: Failed to read PHY register\n");

			goto label_exit;
		}
		msg.regValue = ret & 0xFFFF;
		// 将读取到的值传回用户空间
		if (copy_to_user((void __user *)arg, &msg, sizeof(msg)))
			ret = -EFAULT;
		break;

	case SMDIO_WRITE_PHY_REG:
		if (copy_from_user(&msg, (void __user *)arg, sizeof(msg)))
		{
			ret = -EFAULT;

			goto label_exit;
		}

		s_smdio_bus = mdio_find_bus(msg.mdioBus);
		if (s_smdio_bus == NULL)
		{
			printk(KERN_ERR "smdio: MDIO bus not initialized\n");

			ret = -ENODEV;

			goto label_exit;
		}
		
		mutex_lock(&s_smdio_bus->mdio_lock);
		if (msg.mmd == 0x00)
			ret = __mdiobus_write(s_smdio_bus, msg.phyAddr, msg.regAddr, msg.regValue);
		else
			ret = __mdiobus_c45_write(s_smdio_bus, msg.phyAddr, msg.mmd, 
													msg.regAddr, msg.regValue);
		mutex_unlock(&s_smdio_bus->mdio_lock);
		if (ret < 0)
			printk(KERN_ERR "smdio: Failed to write PHY register\n");
		break;

	default:
		ret = -EINVAL;
		break;
    }

label_exit:
	put_device(&s_smdio_bus->dev);

    return ret;
}

// 文件操作结构体
static struct file_operations smdio_fops = 
{
    .owner = THIS_MODULE,
    .open = smdio_open,
    .release = smdio_release,
    .unlocked_ioctl = smdio_ioctl,
};

// 模块初始化
static int __init __smdio_init(void)
{
    dev_t dev_num;

    // 1. 动态分配设备号
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0)
	{
        printk(KERN_ERR "smdio: Failed to allocate device number\n");

        return -1;
    }
    major_num = MAJOR(dev_num);
    printk(KERN_INFO "smdio: allocated major number %d\n", major_num);

    // 2. 初始化并添加cdev
    cdev_init(&smdio_cdev, &smdio_fops);
    if (cdev_add(&smdio_cdev, dev_num, 1) < 0)
	{
        printk(KERN_ERR "smdio: Failed to add cdev\n");
        unregister_chrdev_region(dev_num, 1);

        return -1;
    }

    // 3. 创建device class
    smdio_class = class_create(CLASS_NAME);
    if (IS_ERR(smdio_class))
	{
        printk(KERN_ERR "smdio: Failed to create class\n");
        cdev_del(&smdio_cdev);
        unregister_chrdev_region(dev_num, 1);

        return PTR_ERR(smdio_class);
    }

    // 4. 创建设备文件 /dev/smdio
    device_create(smdio_class, NULL, dev_num, NULL, DEVICE_NAME);

    // 5. 获取MDIO总线
    // 例如：s_smdio_bus = mdio_find_bus("mdio");
    printk(KERN_INFO "smdio: module loaded.\n");

    return 0;
}

// 模块退出
static void __exit __smdio_exit(void)
{
    dev_t dev_num = MKDEV(major_num, 0);

    device_destroy(smdio_class, dev_num);
    class_destroy(smdio_class);
    cdev_del(&smdio_cdev);
    unregister_chrdev_region(dev_num, 1);

    printk(KERN_INFO "smdio: module unloaded\n");
}

module_init(__smdio_init);
module_exit(__smdio_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Archer Chang");
MODULE_DESCRIPTION("A character device for ethernet PHY register access via MDIO");




