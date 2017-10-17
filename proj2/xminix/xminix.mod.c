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

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x99ed5c78, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x6506dc7d, __VMLINUX_SYMBOL_STR(kmem_cache_destroy) },
	{ 0xf77d324b, __VMLINUX_SYMBOL_STR(iget_failed) },
	{ 0x33d53fbc, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0x405c1144, __VMLINUX_SYMBOL_STR(get_seconds) },
	{ 0x143ca23a, __VMLINUX_SYMBOL_STR(drop_nlink) },
	{ 0xa955e9d8, __VMLINUX_SYMBOL_STR(mark_buffer_dirty_inode) },
	{ 0xc90eeb4d, __VMLINUX_SYMBOL_STR(generic_file_llseek) },
	{ 0x423a3d97, __VMLINUX_SYMBOL_STR(__mark_inode_dirty) },
	{ 0x6bf1c17f, __VMLINUX_SYMBOL_STR(pv_lock_ops) },
	{ 0x754d539c, __VMLINUX_SYMBOL_STR(strlen) },
	{ 0x60a13e90, __VMLINUX_SYMBOL_STR(rcu_barrier) },
	{ 0xf75bb387, __VMLINUX_SYMBOL_STR(from_kuid_munged) },
	{ 0x6401e12e, __VMLINUX_SYMBOL_STR(block_write_begin) },
	{ 0xb32a0225, __VMLINUX_SYMBOL_STR(pagecache_get_page) },
	{ 0x25820c64, __VMLINUX_SYMBOL_STR(fs_overflowuid) },
	{ 0x179651ac, __VMLINUX_SYMBOL_STR(_raw_read_lock) },
	{ 0xa40bc239, __VMLINUX_SYMBOL_STR(__lock_page) },
	{ 0x9e6d807c, __VMLINUX_SYMBOL_STR(__lock_buffer) },
	{ 0x1f7fa600, __VMLINUX_SYMBOL_STR(inc_nlink) },
	{ 0x48d5aa47, __VMLINUX_SYMBOL_STR(block_read_full_page) },
	{ 0xb39af84a, __VMLINUX_SYMBOL_STR(mount_bdev) },
	{ 0xed0424f7, __VMLINUX_SYMBOL_STR(generic_read_dir) },
	{ 0x8358c3ad, __VMLINUX_SYMBOL_STR(__getblk_gfp) },
	{ 0xe719dabd, __VMLINUX_SYMBOL_STR(unlock_buffer) },
	{ 0x6e3d42e4, __VMLINUX_SYMBOL_STR(truncate_setsize) },
	{ 0x608ca1a7, __VMLINUX_SYMBOL_STR(from_kgid_munged) },
	{ 0x1bc84e15, __VMLINUX_SYMBOL_STR(make_kgid) },
	{ 0xe3ddff5a, __VMLINUX_SYMBOL_STR(__insert_inode_hash) },
	{ 0xfb578fc5, __VMLINUX_SYMBOL_STR(memset) },
	{ 0xb479afa, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0xab6981c4, __VMLINUX_SYMBOL_STR(__bread_gfp) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x2ba80568, __VMLINUX_SYMBOL_STR(d_rehash) },
	{ 0x449ad0a7, __VMLINUX_SYMBOL_STR(memcmp) },
	{ 0xa1c76e0a, __VMLINUX_SYMBOL_STR(_cond_resched) },
	{ 0x64372238, __VMLINUX_SYMBOL_STR(kmem_cache_free) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0x4aedf651, __VMLINUX_SYMBOL_STR(set_nlink) },
	{ 0xe8db8dd2, __VMLINUX_SYMBOL_STR(_raw_write_lock) },
	{ 0xf78e6758, __VMLINUX_SYMBOL_STR(setattr_copy) },
	{ 0xa6c58264, __VMLINUX_SYMBOL_STR(page_symlink) },
	{ 0x3380573e, __VMLINUX_SYMBOL_STR(sync_dirty_buffer) },
	{ 0xe0676377, __VMLINUX_SYMBOL_STR(truncate_pagecache) },
	{ 0x23b7ee52, __VMLINUX_SYMBOL_STR(unlock_page) },
	{ 0x8c4b3631, __VMLINUX_SYMBOL_STR(generic_file_read_iter) },
	{ 0x37627b92, __VMLINUX_SYMBOL_STR(__brelse) },
	{ 0xf11543ff, __VMLINUX_SYMBOL_STR(find_first_zero_bit) },
	{ 0xeeab0dae, __VMLINUX_SYMBOL_STR(inode_init_once) },
	{ 0xd06433d4, __VMLINUX_SYMBOL_STR(page_follow_link_light) },
	{ 0xc99b6fdb, __VMLINUX_SYMBOL_STR(invalidate_inode_buffers) },
	{ 0x90af41f3, __VMLINUX_SYMBOL_STR(kmem_cache_alloc) },
	{ 0xa916b694, __VMLINUX_SYMBOL_STR(strnlen) },
	{ 0xce04c3cc, __VMLINUX_SYMBOL_STR(generic_file_mmap) },
	{ 0x6930d22, __VMLINUX_SYMBOL_STR(bdevname) },
	{ 0xfab9107b, __VMLINUX_SYMBOL_STR(block_write_full_page) },
	{ 0x546353db, __VMLINUX_SYMBOL_STR(block_write_end) },
	{ 0xaa1960d, __VMLINUX_SYMBOL_STR(truncate_inode_pages_final) },
	{ 0x50ef55be, __VMLINUX_SYMBOL_STR(make_kuid) },
	{ 0xdb7305a1, __VMLINUX_SYMBOL_STR(__stack_chk_fail) },
	{ 0xc77e7c99, __VMLINUX_SYMBOL_STR(generic_write_end) },
	{ 0x8ad3b77d, __VMLINUX_SYMBOL_STR(unlock_new_inode) },
	{ 0x3a1f90c9, __VMLINUX_SYMBOL_STR(kill_block_super) },
	{ 0xc2766075, __VMLINUX_SYMBOL_STR(inode_newsize_ok) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
	{ 0x18d0c698, __VMLINUX_SYMBOL_STR(inode_change_ok) },
	{ 0x3f79aa78, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0xe259ae9e, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0xb0d332f2, __VMLINUX_SYMBOL_STR(kmem_cache_create) },
	{ 0x9617f892, __VMLINUX_SYMBOL_STR(d_tmpfile) },
	{ 0xde0af099, __VMLINUX_SYMBOL_STR(register_filesystem) },
	{ 0xfbcbce0b, __VMLINUX_SYMBOL_STR(generic_file_write_iter) },
	{ 0xd24c85da, __VMLINUX_SYMBOL_STR(iput) },
	{ 0x3aa45e82, __VMLINUX_SYMBOL_STR(read_cache_page) },
	{ 0x3f0bb71f, __VMLINUX_SYMBOL_STR(generic_file_fsync) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x605db5a0, __VMLINUX_SYMBOL_STR(ihold) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0x643e0ce5, __VMLINUX_SYMBOL_STR(call_rcu_sched) },
	{ 0x6128b5fc, __VMLINUX_SYMBOL_STR(__printk_ratelimit) },
	{ 0x28551944, __VMLINUX_SYMBOL_STR(sync_filesystem) },
	{ 0xc5f46499, __VMLINUX_SYMBOL_STR(block_truncate_page) },
	{ 0xf569373e, __VMLINUX_SYMBOL_STR(sb_set_blocksize) },
	{ 0xcac0980, __VMLINUX_SYMBOL_STR(generic_readlink) },
	{ 0x1ecdebbd, __VMLINUX_SYMBOL_STR(put_page) },
	{ 0x37e1a9d7, __VMLINUX_SYMBOL_STR(__bforget) },
	{ 0x686cb96e, __VMLINUX_SYMBOL_STR(d_make_root) },
	{ 0x74c134b9, __VMLINUX_SYMBOL_STR(__sw_hweight32) },
	{ 0x8ece24c9, __VMLINUX_SYMBOL_STR(__block_write_begin) },
	{ 0xe6d14e07, __VMLINUX_SYMBOL_STR(mark_buffer_dirty) },
	{ 0x243e8455, __VMLINUX_SYMBOL_STR(unregister_filesystem) },
	{ 0xdeaf7daf, __VMLINUX_SYMBOL_STR(write_one_page) },
	{ 0xec7c20c6, __VMLINUX_SYMBOL_STR(init_special_inode) },
	{ 0x517d9dd5, __VMLINUX_SYMBOL_STR(new_inode) },
	{ 0x3753b957, __VMLINUX_SYMBOL_STR(generic_file_splice_read) },
	{ 0x34728bce, __VMLINUX_SYMBOL_STR(clear_inode) },
	{ 0x107deb5, __VMLINUX_SYMBOL_STR(page_put_link) },
	{ 0xd811704c, __VMLINUX_SYMBOL_STR(d_instantiate) },
	{ 0xab2c03bf, __VMLINUX_SYMBOL_STR(generic_block_bmap) },
	{ 0x3a74c908, __VMLINUX_SYMBOL_STR(iget_locked) },
	{ 0xb0a98dfd, __VMLINUX_SYMBOL_STR(generic_fillattr) },
	{ 0x566b14df, __VMLINUX_SYMBOL_STR(inode_init_owner) },
	{ 0xdf929370, __VMLINUX_SYMBOL_STR(fs_overflowgid) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "95088DAF6F9DC0E9E0A0364");
