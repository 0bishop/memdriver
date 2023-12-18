#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>


// The target variable
static int target_variable = 42;

// Module initialization function
static int __init mymodule_init(void) {
    printk(KERN_INFO "Execute This:\n\t./usermod target/driver.ko 0x%lx\n", (unsigned long int)&target_variable);
    return 0;
}

// Module cleanup function
static void __exit mymodule_exit(void) {
    printk(KERN_INFO "Exiting the module\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);

// Module parameters: target_variable
module_param(target_variable, int, S_IRUGO);
MODULE_PARM_DESC(target_variable, "An integer variable to print its address");
MODULE_LICENSE("Prout");
MODULE_AUTHOR("Prout");
MODULE_DESCRIPTION("Prout");
MODULE_VERSION("Prout");