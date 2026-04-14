// SPDX-License-Identifier: GPL-2.0
#include <linux/device.h>
#include <linux/device/class.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Janusz Wolak");
MODULE_DESCRIPTION("Simple char driver with auto /dev node and read/write");

#define DRIVER_NAME "driver_class_create_read_write"
#define DEVICE_NAME "mydevice"
#define SIMPLE_BUF_SIZE 1024

static int device_major;
static struct class *simple_class;
static struct device *simple_device;

static char simple_buf[SIMPLE_BUF_SIZE];
static size_t simple_len;
static DEFINE_MUTEX(simple_lock);

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

static ssize_t driver_write(struct file *instance, const char __user *user_buf, size_t count,
			    loff_t *ppos)
{
	pr_info("simple_device_driver: write count=%zu ppos=%lld flags=0x%x pid=%d comm=%s\n",
		count, *ppos, instance->f_flags, current->pid, current->comm);

	size_t to_copy;
	size_t not_copied;

	if (!user_buf) {
		return -EINVAL;
	}

	if (count == 0) {
		return 0;
	}

	to_copy = count;
	if (to_copy > SIMPLE_BUF_SIZE - 1) {
		to_copy = SIMPLE_BUF_SIZE - 1;
	}

	mutex_lock(&simple_lock);
	not_copied = copy_from_user(simple_buf, user_buf, to_copy);
	if (not_copied) {
		pr_err("simple_device_driver: copy_from_user failed not_copied=%zu\n", not_copied);
		mutex_unlock(&simple_lock);
		return -EFAULT;
	}

	simple_buf[to_copy] = '\0';
	simple_len = to_copy;
	mutex_unlock(&simple_lock);

	pr_info("simple_device_driver: write %zu bytes pid=%d comm=%s\n", to_copy, current->pid,
		current->comm);

	return to_copy;
}

static ssize_t driver_read(struct file *instance, char __user *user_buf, size_t count, loff_t *ppos)
{
	pr_info("simple_device_driver: read count=%zu ppos=%lld flags=0x%x pid=%d comm=%s\n", count,
		*ppos, instance->f_flags, current->pid, current->comm);

	size_t to_copy;
	size_t not_copied;
	size_t available;

	if (!user_buf) {
		return -EINVAL;
	}

	if (count == 0) {
		return 0;
	}

	mutex_lock(&simple_lock);

	if (*ppos >= simple_len) {
		mutex_unlock(&simple_lock);
		return 0;
	}

	available = simple_len - *ppos;
	to_copy = (count < available) ? count : available;

	not_copied = copy_to_user(user_buf, simple_buf + *ppos, to_copy);
	if (not_copied) {
		pr_err("simple_device_driver: copy_to_user failed not_copied=%zu\n", not_copied);
		mutex_unlock(&simple_lock);
		return -EFAULT;
	}

	*ppos += to_copy;

	mutex_unlock(&simple_lock);

	pr_info("simple_device_driver: read %zu bytes pid=%d comm=%s\n", to_copy, current->pid,
		current->comm);

	return to_copy;
}

static const struct file_operations simple_driver_fops = {
	.owner = THIS_MODULE,
	.open = driver_open,
	.release = driver_release,
	.write = driver_write,
	.read = driver_read,
};

static int __init simple_driver_init(void)
{
	device_major = register_chrdev(0, DRIVER_NAME, &simple_driver_fops);
	if (device_major < 0) {
		pr_err("simple_device_driver: register_chrdev failed: %d\n", device_major);
		return device_major;
	}

	simple_class = class_create(DRIVER_NAME);
	if (IS_ERR(simple_class)) {
		pr_err("simple_device_driver: class_create failed: %ld\n", PTR_ERR(simple_class));
		unregister_chrdev(device_major, DRIVER_NAME);
		return PTR_ERR(simple_class);
	}

	simple_device =
		device_create(simple_class, NULL, MKDEV(device_major, 0), NULL, DEVICE_NAME);
	if (IS_ERR(simple_device)) {
		pr_err("simple_device_driver: device_create failed: %ld\n", PTR_ERR(simple_device));
		class_destroy(simple_class);
		unregister_chrdev(device_major, DRIVER_NAME);
		return PTR_ERR(simple_device);
	}

	mutex_lock(&simple_lock);
	simple_len = 0;
	simple_buf[0] = '\0';
	mutex_unlock(&simple_lock);

	pr_info("simple_device_driver: registered with major=%d\n", device_major);
	pr_info("simple_device_driver: created /dev/%s\n", DEVICE_NAME);

	return 0;
}

static void __exit simple_driver_exit(void)
{
	device_destroy(simple_class, MKDEV(device_major, 0));
	class_destroy(simple_class);
	unregister_chrdev(device_major, DRIVER_NAME);

	pr_info("simple_device_driver: unregistered major=%d\n", device_major);
}

module_init(simple_driver_init);
module_exit(simple_driver_exit);
