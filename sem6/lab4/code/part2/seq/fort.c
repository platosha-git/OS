#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/errno.h>

#define MAX_COOKIE_LENGTH PAGE_SIZE
#define PROC_FILE_NAME "fortune"


MODULE_LICENSE("GPL");


static struct proc_dir_entry *ent;
static char *buff = NULL;


static int my_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "%s\n", buff);
	printk(KERN_INFO "+ fortune: show handler\n");
	return 0;
}

static ssize_t my_proc_write(struct file* file, const char __user *ubuff, size_t len, loff_t *ppos)
{
	char *tmp = vmalloc(MAX_COOKIE_LENGTH);
	if (!tmp) {
		return -ENOMEM;
	}
	
    if (copy_from_user(tmp, ubuff, len) != 0) {
		vfree(tmp);
		return -EFAULT;
	}
	
    vfree(buff);
	buff = tmp;
	printk(KERN_INFO "+ fortune: write handler\n");
	
	return len;
}

static int my_proc_open(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "+ fortune: open handler\n");
	return single_open(file, my_proc_show, NULL);
}

static struct proc_ops fortune_proc_ops = 
{
	.proc_open = my_proc_open,
	.proc_release = single_release,

	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
	.proc_write = my_proc_write
};


static int __init my_proc_init(void)
{
	ent = proc_create(PROC_FILE_NAME, 0666, NULL, &fortune_proc_ops);
	
	if (!ent) {
		return -ENOMEM;	
	}

    proc_mkdir("cookie_dir", NULL);
    proc_symlink("cookie_symlink", NULL, "/proc/fortune");

	printk(KERN_INFO "+ fort: Proc file created!\n");
	return 0;
}

static void __exit my_proc_exit(void)
{
	if (ent) {
		remove_proc_entry(PROC_FILE_NAME, NULL);
	}

	printk(KERN_INFO "+ fort: Module unloaded!\n");
}

module_init(my_proc_init);
module_exit(my_proc_exit);