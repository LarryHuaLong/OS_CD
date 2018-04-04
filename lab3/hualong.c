#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <asm/uaccess.h>

#define MY_MAJOR 231
#define DEVICE_NAME "HuaLong"
#define buf_size 50
static char buffer[buf_size] = "";
int pos;
static int hualong_open(struct inode *inode, struct file *file)
{
    printk(KERN_ALERT "hualong is opened.\n");
    pos = 0;
    return 0;
}
static int hualong_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    printk(KERN_ALERT "hualong is being read.\n");
    if (*ppos >= buf_size)
        return 0;
    if (count > (buf_size - *ppos))
        count = buf_size - *ppos;
    if (copy_to_user((void *)buf, (void *)(buffer + *ppos), count))
    {
        return -EFAULT;
    }
    *ppos = *ppos + count;
    return count;
}
static int hualong_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    printk(KERN_ALERT "hualong is being writede.\n");
    if (*ppos >= buf_size)
        return 0;
    if (count > (buf_size - *ppos))
        count = buf_size - *ppos;
    if (copy_from_user((void *)(buffer + *ppos), (void *)buf, count))
    {
        return -EINVAL;
    }
    *ppos = *ppos + count;
    return count;
}
loff_t hualong_llseek(struct file *filp, loff_t off, int whence)
{
    loff_t newpos;
    switch (whence)
    {
    case 0: /* SEEK_SET */
        newpos = off;
        break;
    case 1: /* SEEK_CUR */
        newpos = filp->f_pos + off;
        break;
    case 2: /* SEEK_END */
        newpos = buf_size + off;
        break;
    default: /* can't happen */
        return -EINVAL;
    }
    if (newpos < 0 || newpos > buf_size)
        return -EINVAL;
    filp->f_pos = newpos;
    return newpos;
}
/*关闭设备文件*/
int hualong_close(struct inode *inode, struct file *filp)
{
    printk(KERN_ALERT "hualong is closed.\n");
    return 0;
}
static struct file_operations hualong_flops = {
    .owner = THIS_MODULE,
    .open = hualong_open,
    .read = hualong_read,
    .write = hualong_write,
    .llseek = hualong_llseek,
    .release = hualong_close,
};
static int __init hualong_init(void)
{
    int ret;
    ret = register_chrdev(MY_MAJOR, DEVICE_NAME, &hualong_flops);
    if (ret < 0)
    {
        printk(KERN_ALERT DEVICE_NAME " can't register MY_MAJOR number.\n");
        return ret;
    }
    printk(KERN_ALERT DEVICE_NAME " initialized.\n");
    return 0;
}
static void __exit hualong_exit(void)
{
    unregister_chrdev(MY_MAJOR, DEVICE_NAME);
    printk(KERN_ALERT DEVICE_NAME " removed.\n");
}
module_init(hualong_init);
module_exit(hualong_exit);
MODULE_LICENSE("GPL");
