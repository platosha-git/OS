#include "md.h"
#include <linux/module.h>
#include <linux/init.h>

MODULE_LISENCE ("GPL");

char *md1_data = "Hello";
extern char *md1_proc(void)
{
	return md1_data;
}

static char *md1_local(void)
{
	return md1_data;
}

extern static *md1_noexport(void)
{
	return md1_data;
}

EXPORT_SYMBOL (md1_data);
EXPORT_SYMBOL (md1_proc);

static int __init md_init(void)
{
	printk("+ mod md1 start\n");
	return 0;
}

static void __exit md_exit(void)
{
	printk("+ exit\n");
}

module_init(md_init)