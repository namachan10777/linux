#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>
#include "json.h"

#define DRIVER_MEJOR 63
#define DRIVER_NAME "lambda"

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
	printk("parse start %s %d\n", buf, count);
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
	printk("stringified %d %d %d\n", len, info->json.type, info->tag);
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
	return 0;
}

static void lambda_exit(void) {
	printk("Goodbye lambda\n");
	unregister_chrdev(DRIVER_MEJOR, DRIVER_NAME);
}

module_init(lambda_init);
module_exit(lambda_exit);
MODULE_LICENSE("GPL");
