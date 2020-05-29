#include <linux/module.h>
#include <linux/fs.h>

#define DRIVER_MEJOR 63
#define DRIVER_NAME "lambda"

static int lambda_open(struct inode *inode, struct file *file) {
	return 0;
}

static int lambda_release(struct inode *inode, struct file *file) {
	return 0;
}

static ssize_t lambda_write(struct file *file, const char __user *buf, size_t count , loff_t *f_pos) {
	return 0;
}

static ssize_t lambda_read(struct file *file, char __user *buf, size_t count, loff_t *f_pos) {
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
