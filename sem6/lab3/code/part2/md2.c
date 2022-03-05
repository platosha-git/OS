#include <linux/init.h> 
#include <linux/module.h>
#include "md.h"

MODULE_LICENSE("GPL");

static int __init md_init(void) { 
    printk("+ module md2 loaded!\n");
    printk("+ data string exported from md1: %s\n", md1_data); 
    printk("+ string returned md1_proc() is: %s\n", md1_proc());
    
    return 0; 
} 


static void __exit md_exit(void) { 
   printk("+ module md2 unloaded!\n"); 
} 


module_init(md_init); 
module_exit(md_exit);


//printk("md1_local() from md1: %s\n", md1_local()); 		// implicit declaration of function ‘md1_local’
//printk("md1_noexport() from md1: %s\n", md1_noexport()); 	// ERROR: "md1_noexport" undefined!