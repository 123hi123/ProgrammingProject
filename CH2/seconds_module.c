#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>

#define PROC_NAME "seconds"

static unsigned long start_jiffies;

// 讀取經過的秒數
static int seconds_show(struct seq_file *m, void *v) {
    unsigned long elapsed = (jiffies - start_jiffies) / HZ;
    seq_printf(m, "%lu\n", elapsed);
    return 0;
}

static int seconds_open(struct inode *inode, struct file *file) {
    return single_open(file, seconds_show, NULL);
}

static const struct proc_ops seconds_fops = {
    .proc_open = seconds_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

// 模組初始化
static int __init seconds_init(void) {
    start_jiffies = jiffies;
    proc_create(PROC_NAME, 0, NULL, &seconds_fops);
    printk(KERN_INFO "/proc/seconds created\n");
    return 0;
}

// 模組卸載
static void __exit seconds_exit(void) {
    remove_proc_entry(PROC_NAME, NULL);
    printk(KERN_INFO "/proc/seconds removed\n");
}

module_init(seconds_init);
module_exit(seconds_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Kernel module to display seconds since loaded");
MODULE_AUTHOR("Your Name");
