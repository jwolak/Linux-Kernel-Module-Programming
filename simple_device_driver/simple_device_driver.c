// SPDX-License-Identifier: GPL-2.0
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/sched.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Janusz Wolak");
MODULE_DESCRIPTION("Simple device registered and implemented some callback functions");

static int device_major;

static int driver_release(struct inode *device_file, struct file *instance)
{
	pr_info("simple_device_driver: release inode=%lu major=%u minor=%u pid=%d comm=%s\n",
		device_file->i_ino, imajor(device_file), iminor(device_file), current->pid,
		current->comm);

	return 0;
}

static int driver_open(struct inode *device_file, struct file *instance)
{
	pr_info("simple_device_driver: open inode=%lu major=%u minor=%u mode=0%o flags=0x%x pid=%d "
		"comm=%s\n",
		device_file->i_ino, imajor(device_file), iminor(device_file), device_file->i_mode,
		instance->f_flags, current->pid, current->comm);

	return 0;
}

static const struct file_operations simple_driver_fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_release,
};

static int __init simple_driver_init(void)
{
	device_major = register_chrdev(0, "simple_device_driver", &simple_driver_fops);
	if (device_major < 0) {
		pr_err("simple_device_driver: register_chrdev failed: %d\n", device_major);
		return device_major;
	}

	pr_info("simple_device_driver: registered with major=%d\n", device_major);
	return 0;
}

static void __exit simple_driver_exit(void)
{
	unregister_chrdev(device_major, "simple_device_driver");
	pr_info("simple_device_driver: unregistered major=%d\n", device_major);
}

module_init(simple_driver_init);
module_exit(simple_driver_exit);
