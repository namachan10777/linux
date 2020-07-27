#define pr_fmt(fmt) KBUILD_MODNAME ": " fmt

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "json.h"
#include "eval.h"

#define DRIVER_MEJOR 63
#define DRIVER_NAME "lambda"

struct JsonValue *out=NULL, *read_hook=NULL;

static void **syscall_table = (void *) __SYSCALL_TABLE_ADDRESS__;
asmlinkage long (*orig_read)(int magic1, int magic2, unsigned int cmd, void __user *arg);
asmlinkage long syscall_replace_read(int magic1, int magic2, unsigned int cmd, void __user *arg) {
	if (read_hook != NULL) {
		exec(out, read_hook);
	}
}

static void save_original_syscall_address(void) {
	orig_read = syscall_table[__NR_read];
}

static void change_page_attr_to_rw(pte_t *pte) {
	set_pte_atomic(pte, pte_mkwrite(*pte));
}

static void change_page_attr_to_ro(pte_t *pte) {
	set_pte_atomic(pte, pte_clear_flags(*pte, _PAGE_RW));
}

static void replace_syscall(void *new) {
	unsigned int level = 0;
	pte_t *pte;
	pte = lookup_address((unsigned long) syscall_table, &level);
	change_page_attr_to_rw(pte);
	syscall_table[__NR_read] = syscall_replace_read;
	change_page_attr_to_ro(pte);
}

static int syscall_replace_init(void) {
	pr_info("sys_call_table address is 0x%p\n", syscall_table);
	save_original_syscall_address();
	pr_info("original sys_read address is 0x%p\n", orig_read);
	replace_syscall(syscall_replace_read);
	pr_info("system call replaced\n");
	return 0;
}

static void syscall_replace_cleanup(void) {
	pr_info("cleanup");
	if (orig_read)
		replace_syscall(orig_read);
}

char srcbuf[1024];
size_t len;

struct runtime_info_t {
	int tag;
	struct JsonValue json;
};

static int lambda_open(struct inode *inode, struct file *file) {
	struct runtime_info_t *info = kmalloc(sizeof(struct runtime_info_t), GFP_KERNEL);
	info->tag = 0;
	printk("lambda open\n");
	file->private_data = (void*)info;
	return 0;
}

static int lambda_release(struct inode *inode, struct file *file) {
	printk("lambda close\n");
	return 0;
}

static ssize_t lambda_write(struct file *file, const char __user *buf, size_t count , loff_t *f_pos) {
	struct runtime_info_t *info = file->private_data;
	ParseResult result;
	printk("lambda write\n");
	if (!access_ok(buf, count)) {
		return 0;
	}
	printk("parse start %s %ld\n", buf, count);
	result = parse(buf, count);
	if (result.type != SUCCESS) {
		printk("syntax error");
		return 0;
	}
	printk("parse success %d\n", result.value.type);
	info->json = result.value;
	info->tag = 42;
	return count;
}

static ssize_t lambda_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos) {
	struct runtime_info_t *info = file->private_data;
	long long len;
	printk("lambda read\n");
	if (!access_ok(buf, count)) {
		return 0;
	}
	len = stringify(buf, count, info->json);
	printk("stringified %lld %d %d\n", len, info->json.type, info->tag);
	if (len < 0) return 0;
	return 0;
}

struct file_operations s_lambda_fops = {
	.open		= lambda_open,
	.release	= lambda_release,
	.read		= lambda_read,
	.write		= lambda_write,
};

static int lambda_init(void) {
	printk("Hello lambda\n");
	register_chrdev(DRIVER_MEJOR, DRIVER_NAME, &s_lambda_fops);
	syscall_replace_init();
	return 0;
}

static void lambda_exit(void) {
	printk("Goodbye lambda\n");
	syscall_replace_cleanup();
	unregister_chrdev(DRIVER_MEJOR, DRIVER_NAME);
}

module_init(lambda_init);
module_exit(lambda_exit);

MODULE_DESCRIPTION("lambda");
MODULE_AUTHOR("namachan10777");
MODULE_LICENSE("GPL");


