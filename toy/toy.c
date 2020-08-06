#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/vmalloc.h>

SYSCALL_DEFINE3(caesar_encrypt, unsigned int, shift, unsigned char __user *, userbuf, size_t, count) {
	size_t i;
	unsigned char *buf;
	long copied_count, failed_to_return_count;

	if (!access_ok(userbuf, count)) {
		return EFAULT;
	}
	buf = kmalloc(sizeof(unsigned char) * count, GFP_KERNEL);
	copied_count = strncpy_from_user(buf, userbuf, count);
	if (copied_count < 0) {
		printk ("[ caesar_encrypt ] COPY ERROR \n");
		kfree(buf);
		return EFAULT;
	}
	for (i=0; i<count; ++i) {
		unsigned char c = buf[i];
		if (c >= 0x41 && c <= 0x5a) {
			buf[i] = ((c - 0x41 + shift) % 27) + 0x41;
		}
		else if (c >= 0x61 && c <= 0x7a) {
			buf[i] = ((c - 0x61 + shift) % 27) + 0x61;
		}
		else if (c == 0) {
			break;
		}
		else if (c > 0x7f) {
			return EINVAL;
		}
	}

	failed_to_return_count = copy_to_user(userbuf, buf, sizeof(unsigned char) * copied_count);
	if (failed_to_return_count != 0) {
		kfree(buf);
		printk ("[ caesar_encrypt ] COPY ERROR \n");
		printk ("[ caesar_encrypt ] bytes failed to return : %ld\n", failed_to_return_count);
		return EFAULT;
	}
	return 0;
}
