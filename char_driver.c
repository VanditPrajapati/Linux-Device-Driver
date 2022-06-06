#include<linux/init.h>
#include<linux/module.h>
#include<linux/kdev_t.h>
#include<linux/cdev.h>
#include<linux/kernel.h>
#include<linux/uaccess.h>
#include<linux/version.h>
#include<linux/fs.h>
#include<linux/version.h>
#include<linux/slab.h>
#include<linux/ioctl.h>

#define WR_DATA _IOW('a','a',char *)
#define RD_DATA _IOR('a','b',char *)

char *val ;

dev_t first;
static struct cdev char_dev;
static struct class *cl;

uint8_t *kernel_buffer;
size_t memsize = 1024;

static int chardriver_uevent(struct device *dev, struct kobj_uevent_env *env)
{
    add_uevent_var(env, "DEVMODE=%#o", 0666);
    return 0;
}

static int char_open(struct inode *inode, struct file *file)
{
	if((kernel_buffer=kmalloc(memsize, GFP_KERNEL)) == 0)
	{
		printk(KERN_INFO "Cannot allocate the memory\n");
		return -1;
	}
	val = kmalloc(500*sizeof(char), GFP_KERNEL);
	printk(KERN_INFO "Device Driver Opened..\n");
	return 0;
}

static int char_release(struct inode *inode, struct file *file)
{
	kfree(kernel_buffer);
	kfree(val);
	printk(KERN_INFO "Device Driver Closed..\n");
	return 0;
}

static long char_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd)
	{
		case WR_DATA:
			printk(KERN_INFO "Data Writing...\n");
			copy_from_user(val,(char *)arg,sizeof(val));
			printk(KERN_INFO "Data Written Done\n");
			printk(KERN_INFO "Data Written: %s\n",val);
			printk(KERN_INFO " ");
			break;

		case RD_DATA:
			printk(KERN_INFO "Data Reading..\n");
			copy_to_user((char *)arg,val,sizeof(val));
			printk(KERN_INFO "Data Reading Done\n");
			printk(KERN_INFO "Data Read: %s\n",(char *)arg);
			printk(KERN_INFO " ");
			break;

		default:
			break;
	}
	return 0;
}


static ssize_t char_read(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
	copy_to_user(buf, kernel_buffer, count);
	printk(KERN_INFO "Device Driver Read Data Done..\n");
	printk(KERN_INFO "Data Read: %s\n", buf);
	return memsize;
}

static ssize_t char_write(struct file *file, const char __user *buf, size_t count, loff_t *offset)
{
	if(count < memsize)
	{
		memsize = count;
	}
	copy_from_user(kernel_buffer, buf, memsize);
	printk(KERN_INFO "Device Driver Write Data Done..\n");
	printk(KERN_INFO "Data Written: %s\n", kernel_buffer);
	kernel_buffer[memsize] = 0;
	return count;
}


static struct file_operations char_fops = {
	.owner = THIS_MODULE,
	.open = char_open,
	.release = char_release,
	.unlocked_ioctl = char_ioctl,
	.read = char_read,
	.write = char_write
};

static int __init function_init(void)
{
	if(alloc_chrdev_region(&first, 0, 1, "Char Driver") < 0)
	{
		printk(KERN_INFO "Device not able to register\n");
		return -1;
	}

	cdev_init(&char_dev, &char_fops);
	
	if(cdev_add(&char_dev, first, 1) < 0)
	{
		printk(KERN_INFO "Cannot add device to the system\n");
		goto r_class;
	}

	if((cl=class_create(THIS_MODULE, "Char Driver")) == NULL)
	{
		printk(KERN_INFO "Cannot create the struct class for device\n");
		goto r_class;
	}

	cl->dev_uevent = chardriver_uevent;

	if((device_create(cl,NULL, first, NULL, "chardriver")) == NULL)
	{
		printk(KERN_INFO "Device cannot be created\n");
		goto r_device;
	}

	printk(KERN_INFO " ");
	printk("Device Driver Inserted Succesfully..\n");
	printk(KERN_INFO "Major No.: %d and Minor No.: %d", MAJOR(first), MINOR(first));
	return 0;

r_device:
	class_destroy(cl);

r_class:
	unregister_chrdev_region(first, 1);
	return -1;
}

static void __exit function_exit(void)
{
	device_destroy(cl, first);
	class_destroy(cl);
	cdev_del(&char_dev);
	unregister_chrdev_region(first, 1);
	printk("Device Driver Removed Successfully\n");
	printk(KERN_INFO " ");
}


module_init(function_init);
module_exit(function_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Vandit Prajapati");
