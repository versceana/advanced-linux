#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/device.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Diana Yakupova");
MODULE_DESCRIPTION("Integer stack kernel module");

#define DEVICE_NAME "int_stack"
#define CLASS_NAME "int_stack_class"

static dev_t dev_num;
static struct cdev cdev;
static struct class *dev_class;

static int *stack = NULL;
static int top = -1;
static int max_size = 0;
static DEFINE_MUTEX(stack_mutex);

static int dev_open(struct inode *inodep, struct file *filep) {
    return 0;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    return 0;
}

static ssize_t dev_read(struct file *filep, char __user *buf, size_t len, loff_t *off) {
    int value;
    if (mutex_lock_interruptible(&stack_mutex))
        return -ERESTARTSYS;

    if (top < 0) {
        mutex_unlock(&stack_mutex);
        printk(KERN_INFO "int_stack: pop from empty stack\n");
        return 0;
    }

    value = stack[top--];
    mutex_unlock(&stack_mutex);

    if (copy_to_user(buf, &value, sizeof(int)))
        return -EFAULT;
    return sizeof(int);
}

static ssize_t dev_write(struct file *filep, const char __user *buf, size_t len, loff_t *off) {
    int value;
    if (len != sizeof(int))
        return -EINVAL;

    if (copy_from_user(&value, buf, sizeof(int)))
        return -EFAULT;

    if (mutex_lock_interruptible(&stack_mutex))
        return -ERESTARTSYS;

    if (top >= max_size - 1) {
        mutex_unlock(&stack_mutex);
        printk(KERN_INFO "int_stack: push to full stack\n");
        return -ERANGE;
    }

    stack[++top] = value;
    mutex_unlock(&stack_mutex);
    return sizeof(int);
}

static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {
    int new_size;
    int *new_stack;

    if (cmd != 0)
        return -EINVAL;

    if (copy_from_user(&new_size, (int __user *)arg, sizeof(int)))
        return -EFAULT;

    if (new_size <= 0)
        return -EINVAL;

    if (mutex_lock_interruptible(&stack_mutex))
        return -ERESTARTSYS;

    new_stack = kmalloc_array(new_size, sizeof(int), GFP_KERNEL);
    if (!new_stack) {
        mutex_unlock(&stack_mutex);
        return -ENOMEM;
    }

    if (stack) {
        int copy_count = (top + 1 < new_size) ? top + 1 : new_size;
        memcpy(new_stack, stack, copy_count * sizeof(int));
        kfree(stack);
    }
    stack = new_stack;
    max_size = new_size;
    if (top >= max_size)
        top = max_size - 1;

    mutex_unlock(&stack_mutex);
    return 0;
}

static struct file_operations fops = {
    .open = dev_open,
    .release = dev_release,
    .read = dev_read,
    .write = dev_write,
    .unlocked_ioctl = dev_ioctl,
};

static int __init int_stack_init(void) {
    if (alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME) < 0)
        return -1;
    printk(KERN_INFO "int_stack: allocated major number %d\n", MAJOR(dev_num));

    cdev_init(&cdev, &fops);
    if (cdev_add(&cdev, dev_num, 1) < 0) {
        unregister_chrdev_region(dev_num, 1);
        return -1;
    }

    dev_class = class_create(CLASS_NAME);
    if (IS_ERR(dev_class)) {
        cdev_del(&cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(dev_class);
    }

    device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);
    mutex_init(&stack_mutex);
    return 0;
}

static void __exit int_stack_exit(void) {
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&cdev);
    unregister_chrdev_region(dev_num, 1);
    kfree(stack);
    printk(KERN_INFO "int_stack: module unloaded\n");
}

module_init(int_stack_init);
module_exit(int_stack_exit);
