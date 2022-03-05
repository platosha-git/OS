#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/fs.h>
#include <linux/errno.h>
#include <asm/uaccess.h>
#define MAX_COOKIE_LENGTH PAGE_SIZE


MODULE_LICENSE("GPL");


static struct proc_dir_entry *ent;
static struct proc_dir_entry *dir, *symlink;

static char *buff;
static int write_index = 0, read_index = 0;


static int fortune_open(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "+ fortune: open handler\n");
    return 0;
}

static int fortune_release(struct inode *inode, struct file *file)
{
    printk(KERN_INFO "+ fortune: release handler\n");
    return 0;
}

ssize_t fortune_write(struct file *file, const char __user *ubuff, size_t len, loff_t *ppos)
{
    printk(KERN_INFO "+ fortune: write handler\n");

    if (copy_from_user(&buff[write_index], ubuff, len) != 0) {
        return -EFAULT;
    }

    write_index += len;
    buff[write_index - 1] = '\n';

    return len;
}

ssize_t fortune_read(struct file *file, char __user *ubuff, size_t len, loff_t *ppos)
{
    printk(KERN_INFO "+ fortune: read handler\n");

    if (write_index == 0 || *ppos > 0) {
        return 0;
    }

    if (read_index >= write_index) {
        read_index = 0;
    }

    copy_to_user(ubuff, &buff[read_index], len);

    read_index += len;
    *ppos += len;

    return len;
}


static struct proc_ops ops = 
{
    .proc_open = fortune_open,
    .proc_release = fortune_release,

    .proc_read = fortune_read,
    .proc_write = fortune_write
};


static int init_fortune_module(void)
{
    buff = (char *)vmalloc(MAX_COOKIE_LENGTH);
    if (buff == NULL) {
        printk(KERN_INFO "+ error: Couldn't create buff\n");
        return -ENOMEM;
    }

    memset(buff, 0, MAX_COOKIE_LENGTH);

    ent = proc_create("fortune", 0666, NULL, &ops);
    dir = proc_mkdir("fortune_dir", NULL);
    symlink = proc_symlink("fortune_symlink", NULL, "/proc/fortune_dir");
    
    if ((ent == NULL) || (dir == NULL) || (symlink == NULL)) {
        vfree(buff);
        printk(KERN_INFO "+ error: Couldn't create proc entry, dir, symlink\n");
        return -ENOMEM;
    }

    write_index = 0;
    read_index = 0;

    printk(KERN_INFO "+ Module fortune loaded!\n");
    return 0;
}

void exit_fortune_module(void)
{
    remove_proc_entry("fortune", NULL);
    remove_proc_entry("fortune_symlink", NULL);
    remove_proc_entry("fortune_dir", NULL);
    
    vfree(buff);

    printk(KERN_INFO "+ Module fortune unloaded!\n");
}


module_init(init_fortune_module);
module_exit(exit_fortune_module);