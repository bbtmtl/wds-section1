
/*
****** 2013-06-27*****************************
**This is the first version Hello world modules
** No other function except printk
** No system call
***** Zhou Song *****************************
*/

#include <linux/init.h>
#include <linux/module.h>

static int __init sam_init(void)
{
	printk(KERN_ALERT"Hello I am coming.\n");
	return 0;
}

static void __exit sam_exit(void)
{
	printk(KERN_ALERT"Bye, I am away.\n");
} 

module_init(sam_init);
module_exit(sam_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("sam");
