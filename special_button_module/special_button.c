// http://people.ee.ethz.ch/~arkeller/linux/code/usermodehelper.c

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>

static int __init callmodule_init(void)
{
    int ret = 0;
    char userprog[] = "echo", "cosx", ">", "/dev/char_dev";
    char *argv[] = {userprog, "2", NULL };
    char *envp[] = {"HOME=/", "PATH=/sbin:/usr/sbin:/bin:/usr/bin", NULL };

    printk("callmodule: init %s\n", userprog);
    /* last parameter: 1 -> wait until execution has finished, 0 go ahead without waiting*/
    /* returns 0 if usermode process was started successfully, errorvalue otherwise*/
    /* no possiblity to get return value of usermode process*/
    ret = call_usermodehelper(userprog, argv, envp, UMH_WAIT_EXEC);
    if (ret != 0)
        printk("error in call to usermodehelper: %i\n", ret);
    else
        printk("everything all right\n");
        return 0;
}

static void __exit callmodule_exit(void)
{
    printk("callmodule: exit\n");
}

module_init(callmodule_init);
module_exit(callmodule_exit);
MODULE_LICENSE("GPL");