#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>

#define PROC_NAME "jiffies"

// 讀取 jiffies 值
static int jiffies_show(struct seq_file *m, void *v) {
    seq_printf(m, "%lu\n", jiffies);
    return 0;
}

static int jiffies_open(struct inode *inode, struct file *file) {
    return single_open(file, jiffies_show, NULL);
}

static const struct proc_ops jiffies_fops = {
    .proc_open = jiffies_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// 模組初始化
static int __init jiffies_init(void) {
    proc_create(PROC_NAME, 0, NULL, &jiffies_fops);
    printk(KERN_INFO "/proc/jiffies created\n");
    return 0;
}

// 模組卸載
static void __exit jiffies_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "/proc/jiffies removed\n");
}

module_init(jiffies_init);
module_exit(jiffies_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel module to display jiffies");
MODULE_AUTHOR("Your Name");
