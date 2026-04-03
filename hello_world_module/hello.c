#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Janusz Wolak");
MODULE_DESCRIPTION("Hello world kernel module");
MODULE_VERSION("1.0");

static int __init hello_init(void)
{
  pr_info("hello_module: Hello from kernel space!\n");
  return 0;
}

static void __exit hello_exit(void)
{
  pr_info("hello_module: Goodbye from kernel space!\n");
}

module_init(hello_init);
module_exit(hello_exit);