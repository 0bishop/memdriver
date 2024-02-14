#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

// IOCTL CODES
#define RD_MEMORY _IOR(0x666, 0x667, struct memory_data*)
#define WR_MEMORY _IOW(0x666, 0x668, struct memory_data*)

#define __always_inline  inline __attribute__((__always_inline__))

// Structure for ioctl
typedef struct {
    void* address;
    void* buffer;
    size_t len;
} memory_data;

// Global variables about the device
dev_t dev = 0;
static struct class *dev_class;
static struct cdev cdev;

static int __init _driver_init(void);
static void __exit _driver_exit(void);
static __always_inline long _ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops = {
    .owner          = THIS_MODULE,
    .unlocked_ioctl = _ioctl,
};

static __always_inline int is_user_address(const void* addr) {
    return ((uintptr_t)addr > TASK_SIZE) ? 0 : 1;
}

static __always_inline long copy_from_kernel_to_user_nofault(void *dst, void *src, size_t size) {
    long ret;

    pagefault_disable();
    ret = __copy_to_user_inatomic((__force void __user *)dst, src, size);
    pagefault_enable();
    return ret ? -EFAULT : 0;
}

static __always_inline long copy_from_user_to_kernel_nofault(void *dst, void *src, size_t size) {
    long ret;

    pagefault_disable();
    ret = __copy_from_user_inatomic(dst, (__force const void __user *)src, size);  // Corrected
    pagefault_enable();
    return ret ? -EFAULT : 0;
}

static __always_inline long _ioctl(struct file *file, unsigned int cmd, unsigned long arg) {
    memory_data data;

    if (copy_from_user(&data, (memory_data *)arg, sizeof(data))) {
        pr_err("copy_from_user: Copy from user failed\n");
        goto efault;
    }

    if (is_user_address(data.address) && !access_ok(data.address, data.len)) {
        pr_err("Memory Error: Invalid memory access\n");
        goto efault;
    }

    if (!is_user_address(data.address)) {
        if (cmd == RD_MEMORY) {
            if (copy_from_kernel_to_user_nofault(data.buffer, data.address, data.len)) {
                pr_err("Read Memory: Copy to kernel failed\n");
                goto efault;
            }
        } else if (cmd == WR_MEMORY) {
            // Disable write protection
            write_cr0(read_cr0() & ~0x10000);
            if (copy_from_user_to_kernel_nofault(data.address, data.buffer, data.len)) {
                pr_err("Write Memory: Copy from kernel failed\n");
                goto efault;
            }
            write_cr0(read_cr0() | 0x10000);
        } else
            goto einval;
    } else {
        // Address is in user space
        if (cmd == RD_MEMORY) {
            if (copy_from_user(data.buffer, data.address, data.len)) {
                pr_err("Read Memory: Copy from user failed\n");
                goto efault;
            }
        } else if (cmd == WR_MEMORY) {
            if (copy_to_user((void *)data.address, data.buffer, data.len)) {
                pr_err("Write Memory: Copy to user failed\n");
                goto efault;
            }
        } else
            goto einval;
    }

    return 0;

    einval:
        pr_err("Bad ioctl command\n");
        return -EINVAL;

    efault:
        return -EFAULT;
}

static int __init _driver_init(void) {
    // identifying major and minor number
    if ((alloc_chrdev_region(&dev, 0, 1, "mem_dev")) < 0) {
        pr_err("Cannot allocate major number\n");
        return -1;
    }

    // Init the cdev structure and pass the file operations
    cdev_init(&cdev, &fops);

    // Adding character device to the system
    if ((cdev_add(&cdev, dev, 1)) < 0) {
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }

    // Creating struct class for the device
    if (IS_ERR(dev_class = class_create("mem_class"))) {
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }

    // Creating device /dev/memop
    if (IS_ERR(device_create(dev_class, NULL, dev, NULL, "memdriver"))) {
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }

    pr_info("Device Driver Started : Major = %d Minor = %d \n", MAJOR(dev), MINOR(dev));
    return 0;

    r_device:
        class_destroy(dev_class);
    r_class:
        unregister_chrdev_region(dev, 1);
        return -1;
}

static void __exit _driver_exit(void) {
    device_destroy(dev_class, dev);
    class_destroy(dev_class);
    cdev_del(&cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Killed\n");
}

module_init(_driver_init);
module_exit(_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prouted");
MODULE_DESCRIPTION("Read & Write Mem kernel module");
MODULE_VERSION("0.1");
