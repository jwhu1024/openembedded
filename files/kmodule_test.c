/*
 * Skeleton Linux Kernel Module
 *
 * PUBLIC DOMAIN
 */


#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/version.h>

MODULE_DESCRIPTION("Kernel Module Test");
MODULE_AUTHOR("Lester Hu <lester_hu@bandrich.com>");
MODULE_LICENSE("BandRich Proprietary license");

static int kernel_module_init (void)
{
	printk(KERN_INFO "----------Kernel Module Test Init---------\n");
	return 0;
}


static void kernel_module_exit (void)
{
	printk(KERN_INFO "----------Kernel Module Test Exit----------\n");
	return;
}

module_init(kernel_module_init);
module_exit(kernel_module_exit);
