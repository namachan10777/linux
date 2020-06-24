#include <linux/module.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/slab.h>

#define DRIVER_MEJOR 64
#define DRIVER_NAME "unrandom"

char srcbuf[1024];
size_t len;

struct runtime_info_t {
	char *srcbuf;
	size_t bufsize;
};

static int lambda_open(struct inode *inode, struct file *file) {
	char *srcbuf = kmalloc(sizeof(char) * 1024, GFP_KERNEL);
	struct runtime_info_t *info = kmalloc(sizeof(struct runtime_info_t), GFP_KERNEL);
	info->srcbuf = srcbuf;
	file->private_data = (void*)info;
	return 0;
}

static int lambda_release(struct inode *inode, struct file *file) {
	return 0;
}

static ssize_t lambda_write(struct file *file, const char __user *buf, size_t count , loff_t *f_pos) {
	printk("Write unrandom\n");
	size_t i;
	struct runtime_info_t *info = file->private_data;
	if (!access_ok(buf, count)) {
		return 0;
	}
	if (count > 1024)
		count = 1024;
	for (i=0; i<count; ++i) {
		info->srcbuf[i] = buf[i];
	}
	info->bufsize = count;
	return count;
}

static ssize_t lambda_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos) {
	printk("Read unrandom\n");
	size_t i;
	struct runtime_info_t *info = file->private_data;
	if (!access_ok(buf, count)) {
		return 0;
	}
	for (i=0; i<count && *f_pos < info->bufsize; ++i) {
		buf[i] = info->srcbuf[(*f_pos)++];
	}
	return info->bufsize - *f_pos;
}

struct file_operations s_lambda_fops = {
	.open		= lambda_open,
	.release	= lambda_release,
	.read		= lambda_read,
	.write		= lambda_write,
};

static int lambda_init(void) {
	printk("Hello unrandom\n");
	register_chrdev(DRIVER_MEJOR, DRIVER_NAME, &s_lambda_fops);
	return 0;
}

static void lambda_exit(void) {
	printk("Goodbye unrandom\n");
	unregister_chrdev(DRIVER_MEJOR, DRIVER_NAME);
}

module_init(lambda_init);
module_exit(lambda_exit);
