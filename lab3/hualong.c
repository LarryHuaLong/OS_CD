#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>

#define    MY_MAJOR     	231
#define    DEVICE_NAME     	"HuaLong"

static int hualong_open(struct inode *inode, struct file *file){
    printk(KERN_ALERT "hualong is opened.\n");
    return 0;
}

static int hualong_read(struct file *file, char __user * buf, size_t count, loff_t *ppos){
    printk(KERN_ALERT "hualong is being read.\n");
    return 0;
}
static int hualong_write(struct file *file, const char __user * buf, size_t count, loff_t *ppos){
    printk(KERN_ALERT "hualong is being writede.\n");
    return 0;
}

static struct file_operations hualong_flops = {
    .owner  =   THIS_MODULE,
    .open   =   hualong_open,
    .read   =   hualong_read,
    .write  =   hualong_write,
};

static int __init hualong_init(void){
    int ret;
    ret = register_chrdev(MY_MAJOR,DEVICE_NAME, &hualong_flops);
    if (ret < 0) {
      printk(KERN_ALERT DEVICE_NAME " can't register MY_MAJOR number.\n");
      return ret;
    }
    printk(KERN_ALERT DEVICE_NAME " initialized.\n");
    return 0;
}

static void __exit hualong_exit(void){
    unregister_chrdev(MY_MAJOR, DEVICE_NAME);
    printk(KERN_ALERT DEVICE_NAME " removed.\n");
}

module_init(hualong_init);
module_exit(hualong_exit);
MODULE_LICENSE("GPL");
