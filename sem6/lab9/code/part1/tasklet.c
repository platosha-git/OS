#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/string.h>
#include <linux/vmalloc.h>
#include <linux/errno.h>

#define MY_IRQ_NUM 1
#define PROC_FILE_NAME "tasklet"

MODULE_LICENSE("GPL");

static struct proc_dir_entry *ent;
static long work_state = 0;
char my_tasklet_data[] = "Tasklet was called!";


void tasklet(unsigned long data);

DECLARE_TASKLET(my_tasklet, tasklet, (unsigned long) &my_tasklet_data);

void tasklet(unsigned long data) 
{
	printk(KERN_INFO "+ Tasklet state: %ld, count: %d, data: %s\n",
		my_tasklet.state, my_tasklet.count, my_tasklet.data);
	work_state = my_tasklet.state;
}


//seq file
static int my_proc_show(struct seq_file *m, void *v)
{
	seq_printf(m, "+ Tasklet old state: %ld, state: %ld, count: %d, data: %s\n", 
		work_state, my_tasklet.state, my_tasklet.count, my_tasklet.data);

	return 0;
}

static int my_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, my_proc_show, NULL);
}

static struct proc_ops tasklet_proc_ops = 
{
	.proc_open = my_proc_open,
	.proc_release = single_release,

	.proc_read = seq_read,
	.proc_lseek = seq_lseek,
};


//tasklet
static irqreturn_t my_irq_handler(int irq_num, void *dev_id) 
{
	if (irq_num == MY_IRQ_NUM) {
		tasklet_schedule(&my_tasklet);
		printk(KERN_INFO "+ Interrupt. Tasklet was scheduled!\n");
	
		return IRQ_HANDLED;
	}

	return IRQ_NONE;
}

static int __init my_tasklet_init(void) 
{
	ent = proc_create(PROC_FILE_NAME, 0666, NULL, &tasklet_proc_ops);
	
	if (!ent) {
		return -ENOMEM;	
	}

	printk(KERN_INFO "+ Seq file created!\n");


	if (request_irq(MY_IRQ_NUM, my_irq_handler, IRQF_SHARED, "my_irq_handler", (void *)my_irq_handler)) {
		return -1;
	}

	printk(KERN_INFO "+ Module tasklet loaded!\n");
	printk(KERN_INFO "+ Irq handler registered!\n");
	
	return 0;
}

static void __exit my_tasklet_exit(void) 
{
	tasklet_kill(&my_tasklet);
	free_irq(MY_IRQ_NUM, &my_irq_handler);

	if (ent) {
		remove_proc_entry(PROC_FILE_NAME, NULL);
	}

	printk(KERN_INFO "+ Module tasklet unloaded!\n");
}

module_init(my_tasklet_init);
module_exit(my_tasklet_exit);