#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif



static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xc086c679, "cdev_del" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x122c3a7e, "_printk" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x9a65111a, "cdev_init" },
	{ 0x8665d240, "cdev_add" },
	{ 0xa0cca538, "class_create" },
	{ 0xb01b7c06, "device_create" },
	{ 0x7cd94d92, "sysfs_remove_group" },
	{ 0xe600137a, "kobject_put" },
	{ 0x7e22fbde, "usb_deregister" },
	{ 0x37a0cba, "kfree" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x69acdf38, "memcpy" },
	{ 0x40ce34f4, "kmalloc_caches" },
	{ 0xba975bb6, "kmalloc_trace" },
	{ 0x7e2b390, "usb_register_driver" },
	{ 0xd2dcf4b2, "kernel_kobj" },
	{ 0xa2d9b94e, "kobject_create_and_add" },
	{ 0x2f2fb3dc, "sysfs_create_group" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x54b1fac6, "__ubsan_handle_load_invalid_value" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0x89940875, "mutex_lock_interruptible" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0xf0fdf6cb, "__stack_chk_fail" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0xbcab6ee6, "sscanf" },
	{ 0xa1a416b0, "device_destroy" },
	{ 0xf9c5db89, "class_destroy" },
	{ 0x6eb018e5, "module_layout" },
};

MODULE_INFO(depends, "");

MODULE_ALIAS("usb:v058Fp6387d*dc*dsc*dp*ic*isc*ip*in*");

MODULE_INFO(srcversion, "F490C3710BD17A219444363");
