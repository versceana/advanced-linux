#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/mutex.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/usb.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Diana Yakupova");
MODULE_DESCRIPTION("Integer stack module gated by USB key");

#define DEVICE_NAME "int_stack"
#define CLASS_NAME  "int_stack_class"
#define USB_VENDOR_ID  0x058F
#define USB_PRODUCT_ID 0x6387

static dev_t dev_num;
static struct cdev cdev;
static struct class *dev_class;
static struct device *dev_device;
static int *stack = NULL;
static int top = -1;
static int max_size = 0;
static DEFINE_MUTEX(stack_mutex);
static bool usb_connected = false;

static struct kobject *int_stack_kobj;

static int dev_open(struct inode *inodep, struct file *filep)
{
    if (!usb_connected) return -ENODEV;
    return 0;
}
static int dev_release(struct inode *inodep, struct file *filep)
{
    return 0;
}
static ssize_t dev_read(struct file *filep, char __user *buf, size_t len, loff_t *off)
{
    int value;
    if (!usb_connected) return -ENODEV;
    if (mutex_lock_interruptible(&stack_mutex)) return -ERESTARTSYS;
    if (top < 0) {
        mutex_unlock(&stack_mutex);
        return 0;
    }
    value = stack[top--];
    mutex_unlock(&stack_mutex);
    if (copy_to_user(buf, &value, sizeof(int))) return -EFAULT;
    return sizeof(int);
}
static ssize_t dev_write(struct file *filep, const char __user *buf, size_t len, loff_t *off)
{
    int value;
    if (!usb_connected) return -ENODEV;
    if (len != sizeof(int)) return -EINVAL;
    if (copy_from_user(&value, buf, sizeof(int))) return -EFAULT;
    if (mutex_lock_interruptible(&stack_mutex)) return -ERESTARTSYS;
    if (top >= max_size - 1) {
        mutex_unlock(&stack_mutex);
        return -ERANGE;
    }
    stack[++top] = value;
    mutex_unlock(&stack_mutex);
    return sizeof(int);
}
static long dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg)
{
    int new_size;
    int *new_stack;
    if (!usb_connected) return -ENODEV;
    if (cmd != 0) return -EINVAL;
    if (copy_from_user(&new_size, (int __user *)arg, sizeof(int))) return -EFAULT;
    if (new_size <= 0) return -EINVAL;
    if (mutex_lock_interruptible(&stack_mutex)) return -ERESTARTSYS;
    new_stack = kmalloc_array(new_size, sizeof(int), GFP_KERNEL);
    if (!new_stack) {
        mutex_unlock(&stack_mutex);
        return -ENOMEM;
    }
    if (stack) {
        int copy = (top+1 < new_size) ? top+1 : new_size;
        memcpy(new_stack, stack, copy * sizeof(int));
        kfree(stack);
    }
    stack = new_stack;
    max_size = new_size;
    if (top >= max_size) top = max_size - 1;
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

static ssize_t control_store(struct kobject *kobj, struct kobj_attribute *attr,
                             const char *buf, size_t count)
{
    int cmd;
    if (sscanf(buf, "%d", &cmd) != 1) return -EINVAL;
    if (cmd == 0) {
        if (usb_connected) {
            device_destroy(dev_class, dev_num);
            class_destroy(dev_class);
            cdev_del(&cdev);
            unregister_chrdev_region(dev_num, 1);
            usb_connected = false;
            pr_info("int_stack: key removed (simulated)\n");
        }
    } else if (cmd == 1) {
        if (!usb_connected) {
            int ret;
            ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
            if (ret < 0) return ret;
            cdev_init(&cdev, &fops);
            ret = cdev_add(&cdev, dev_num, 1);
            if (ret < 0) {
                unregister_chrdev_region(dev_num, 1);
                return ret;
            }
            dev_class = class_create(CLASS_NAME);
            if (IS_ERR(dev_class)) {
                cdev_del(&cdev);
                unregister_chrdev_region(dev_num, 1);
                return PTR_ERR(dev_class);
            }
            dev_device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);
            if (IS_ERR(dev_device)) {
                class_destroy(dev_class);
                cdev_del(&cdev);
                unregister_chrdev_region(dev_num, 1);
                return PTR_ERR(dev_device);
            }
            usb_connected = true;
            pr_info("int_stack: key inserted (simulated)\n");
        }
    }
    return count;
}
static struct kobj_attribute control_attr = __ATTR_WO(control);

static struct attribute *attrs[] = {
    &control_attr.attr,
    NULL,
};
static struct attribute_group attr_group = {
    .attrs = attrs,
};

static int int_stack_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    int ret;
    printk(KERN_INFO "int_stack: USB key detected (VID=0x%04X, PID=0x%04X)\n",
           id ? id->idVendor : USB_VENDOR_ID, id ? id->idProduct : USB_PRODUCT_ID);
    if (usb_connected) return 0;

    ret = alloc_chrdev_region(&dev_num, 0, 1, DEVICE_NAME);
    if (ret < 0) return ret;
    cdev_init(&cdev, &fops);
    ret = cdev_add(&cdev, dev_num, 1);
    if (ret < 0) {
        unregister_chrdev_region(dev_num, 1);
        return ret;
    }
    dev_class = class_create(CLASS_NAME);
    if (IS_ERR(dev_class)) {
        cdev_del(&cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(dev_class);
    }
    dev_device = device_create(dev_class, NULL, dev_num, NULL, DEVICE_NAME);
    if (IS_ERR(dev_device)) {
        class_destroy(dev_class);
        cdev_del(&cdev);
        unregister_chrdev_region(dev_num, 1);
        return PTR_ERR(dev_device);
    }
    if (!stack) {
        max_size = 128;
        stack = kmalloc_array(max_size, sizeof(int), GFP_KERNEL);
        if (!stack) {
            device_destroy(dev_class, dev_num);
            class_destroy(dev_class);
            cdev_del(&cdev);
            unregister_chrdev_region(dev_num, 1);
            return -ENOMEM;
        }
        top = -1;
    }
    usb_connected = true;
    return 0;
}
static void int_stack_disconnect(struct usb_interface *interface)
{
    printk(KERN_INFO "int_stack: USB key removed, hiding device\n");
    if (!usb_connected) return;
    device_destroy(dev_class, dev_num);
    class_destroy(dev_class);
    cdev_del(&cdev);
    unregister_chrdev_region(dev_num, 1);
    usb_connected = false;
}

static struct usb_device_id int_stack_table[] = {
    { USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
    { }
};
MODULE_DEVICE_TABLE(usb, int_stack_table);

static struct usb_driver int_stack_usb_driver = {
    .name       = "int_stack",
    .id_table   = int_stack_table,
    .probe      = int_stack_probe,
    .disconnect = int_stack_disconnect,
};

static int __init int_stack_init(void)
{
    int ret;
    ret = usb_register(&int_stack_usb_driver);
    if (ret) return ret;
    int_stack_kobj = kobject_create_and_add("int_stack", kernel_kobj);
    if (!int_stack_kobj) {
        usb_deregister(&int_stack_usb_driver);
        return -ENOMEM;
    }
    ret = sysfs_create_group(int_stack_kobj, &attr_group);
    if (ret) {
        kobject_put(int_stack_kobj);
        usb_deregister(&int_stack_usb_driver);
        return ret;
    }
    pr_info("int_stack: forcing probe to simulate USB key insertion\n");
    int_stack_probe(NULL, NULL);
    return 0;
}
static void __exit int_stack_exit(void)
{
    int_stack_disconnect(NULL);
    sysfs_remove_group(int_stack_kobj, &attr_group);
    kobject_put(int_stack_kobj);
    usb_deregister(&int_stack_usb_driver);
    if (stack) {
        kfree(stack);
        stack = NULL;
    }
    printk(KERN_INFO "int_stack: module unloaded\n");
}

module_init(int_stack_init);
module_exit(int_stack_exit);
