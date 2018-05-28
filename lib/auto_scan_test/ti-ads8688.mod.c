#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x7109deb1, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x2b03a614, __VMLINUX_SYMBOL_STR(driver_unregister) },
	{ 0x3171e93, __VMLINUX_SYMBOL_STR(__spi_register_driver) },
	{ 0xc5414778, __VMLINUX_SYMBOL_STR(dev_err) },
	{ 0x8f849c16, __VMLINUX_SYMBOL_STR(iio_device_register) },
	{ 0xebecf975, __VMLINUX_SYMBOL_STR(iio_triggered_buffer_setup) },
	{ 0xd969361, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x2d50a73e, __VMLINUX_SYMBOL_STR(spi_get_device_id) },
	{ 0x24fd7ec, __VMLINUX_SYMBOL_STR(regulator_get_voltage) },
	{ 0xb1c8e54f, __VMLINUX_SYMBOL_STR(regulator_enable) },
	{ 0x116d824f, __VMLINUX_SYMBOL_STR(devm_regulator_get_optional) },
	{ 0xb6dbcdde, __VMLINUX_SYMBOL_STR(devm_iio_device_alloc) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0xe422ab4e, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0xec9248fd, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x63c4d61f, __VMLINUX_SYMBOL_STR(__bitmap_weight) },
	{ 0x9997e310, __VMLINUX_SYMBOL_STR(iio_trigger_notify_done) },
	{ 0xa578297f, __VMLINUX_SYMBOL_STR(iio_push_to_buffers) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0x917869eb, __VMLINUX_SYMBOL_STR(spi_sync) },
	{ 0x343a1a8, __VMLINUX_SYMBOL_STR(__list_add) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0x5f754e5a, __VMLINUX_SYMBOL_STR(memset) },
	{ 0x8f678b07, __VMLINUX_SYMBOL_STR(__stack_chk_guard) },
	{ 0x818a217c, __VMLINUX_SYMBOL_STR(regulator_disable) },
	{ 0xc4d0f7f1, __VMLINUX_SYMBOL_STR(iio_triggered_buffer_cleanup) },
	{ 0x23c84df7, __VMLINUX_SYMBOL_STR(iio_device_unregister) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

MODULE_ALIAS("of:N*T*Cti,ads8684");
MODULE_ALIAS("of:N*T*Cti,ads8684C*");
MODULE_ALIAS("of:N*T*Cti,ads8688");
MODULE_ALIAS("of:N*T*Cti,ads8688C*");
MODULE_ALIAS("spi:ads8684");
MODULE_ALIAS("spi:ads8688");
