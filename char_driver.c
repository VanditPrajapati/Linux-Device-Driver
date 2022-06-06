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
#include<linux/proc_fs.h>

#define WR_DATA _IOW('a','a',char *)
#define RD_DATA _IOR('a','b',char *)
#define SET_VARIABLE _IOW('a','c',int *)
#define GET_VARIABLE _IOR('a','d',int *)

char *val;
unsigned long int variable;

volatile int value;

dev_t first;
static struct cdev char_dev;
static struct class *cl;

static struct kobject *kobj_ref;

static struct proc_dir_entry *parent;

static char array[20] = "Vandit Prajapati\n";
static int len = 1;

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

		case SET_VARIABLE:
			printk(KERN_INFO "Setting the variable\n");
			variable = arg;
			printk(KERN_INFO "New value of Variable is: %ld", variable);
			printk(" ");
			break;

		case GET_VARIABLE:
			printk(KERN_INFO "Reading the value of variable\n");
			arg = variable;
			printk(KERN_INFO "The value of variable is: %ld\n", arg);
			printk(" ");
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

static ssize_t sysfs_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk(KERN_INFO "Sysfs Read Done\n");
	return sprintf(buf, "%d\n", value);
}

static ssize_t sysfs_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	printk(KERN_INFO "Sysfs Write Done\n");
	sscanf(buf, "%d", &value);
	return count;
}

static int open_proc(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "proc file opened\n");
	return 0;
}

static int release_proc(struct inode *inode, struct file *file)
{
	printk(KERN_INFO "proc file closed\n");
	return 0;
}

static ssize_t read_proc(struct file *file, char __user *buf, size_t count, loff_t *offset)
{
	printk(KERN_INFO "proc file read..\n");
	if(len)
	{
		len = 0;
	}
	else
	{
		len = 1;
		return 0;
	}

	if(copy_to_user(buf,array,20))
	{
		printk(KERN_INFO "Data read error\n");
	}

	return count;
}

static ssize_t write_proc(struct file *file, const char *buf, size_t count, loff_t *offset)
{
	printk(KERN_INFO "Data Writing\n");
	copy_from_user(array,buf,count);
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

static struct file_operations proc_fops = {
	.open = open_proc,
	.release = release_proc,
	.read = read_proc,
	.write = write_proc
};

static struct kobj_attribute chardriver_attr = __ATTR(value, 0660, sysfs_show, sysfs_store);

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
 
	parent = proc_mkdir("chardriver", NULL);
	
	if(parent == NULL)
	{
		printk(KERN_INFO "Error creating proc entry\n");
		goto r_device;
	}

	proc_create("chardriver_proc", 0666, parent, &proc_fops);

	kobj_ref = kobject_create_and_add("chardriver_sysfs", NULL);

	if(sysfs_create_file(kobj_ref, &chardriver_attr.attr))
	{
		printk(KERN_INFO "sysfs file cannot be created\n");
		goto r_sysfs;
	}

	printk(KERN_INFO " ");
	printk("Device Driver Inserted Succesfully..\n");
	printk(KERN_INFO "Major No.: %d and Minor No.: %d", MAJOR(first), MINOR(first));
	return 0;

r_sysfs:
	kobject_put(kobj_ref);
	sysfs_remove_file(NULL, &chardriver_attr.attr);	

r_device:
	class_destroy(cl);

r_class:
	unregister_chrdev_region(first, 1);
	return -1;
}

static void __exit function_exit(void)
{
	remove_proc_entry("chardriver/chardriver_proc",parent);
	kobject_put(kobj_ref);
	sysfs_remove_file(NULL, &chardriver_attr.attr);
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
