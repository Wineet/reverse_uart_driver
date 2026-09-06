#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<asm/uaccess.h>
#include<linux/uaccess.h>

/* ioctl commands  'x' was ununsed magic number ioctl-number.rst
 *  */

#define R_UART_BAUD_SET _IOW('x',1,unsigned int)
#define R_UART_BAUD_GET _IOR('x',2,unsigned int)

MODULE_LICENSE("GPL");
static dev_t dev;
static struct cdev reverse_uart_dev;
static struct class *cls;
struct _r_uart_drv{
	unsigned int major;
	unsigned int minor;
	unsigned int baud_rate;
}; 
#define MAX_DEVICES 4
#define MAX_BUFFER_SIZE 25
#define EOF 0
/*static arrary to represent user buffer content Minor number wise*/
static char user_data[MAX_DEVICES][MAX_BUFFER_SIZE]={0};
static int reverse_uart_open(struct inode *inode,struct file *file){
	
	printk("R_UART open minor=%d \n",iminor(inode));
	struct _r_uart_drv *drv_ptr = NULL;
	drv_ptr = kmalloc(sizeof(struct _r_uart_drv),GFP_KERNEL);
	if(!drv_ptr)
	{
		printk("Memory Allocation failed \n");
		return -ENOMEM;
	}
	drv_ptr->major = imajor(inode);
	drv_ptr->minor = iminor(inode);
	file->private_data = drv_ptr;

	return 0;
}
/*
 * 
 * complete release function and release the memory allocated */
static ssize_t reverse_uart_write(struct file *file, const char __user *buf, size_t size,loff_t *offset){
	/*Ignoring offset for dummy buffer run*/
	
	struct _r_uart_drv *drv = file->private_data;
	printk("R_UART %s  size=%u offset=%lld ",__func__,size,*offset);
	if(size> MAX_BUFFER_SIZE)
	{
		size = MAX_BUFFER_SIZE;
	}
	printk("minor %u \n",drv->minor);
	unsigned long int ret = copy_from_user(user_data[drv->minor],buf,size);
	if(ret) //Returns number of byte NOT copied
	{
		return -EFAULT;
	}
	return size;
}

static ssize_t reverse_uart_read(struct file *file, char __user *buf, size_t size,loff_t *offset){
	/*Ignoring offset for dummy buffer run*/

	struct _r_uart_drv *drv = file->private_data;
	printk("R_UART %s  size=%u offset=%lld ",__func__,size,*offset);
	if(*offset >= MAX_BUFFER_SIZE)
	{
		return EOF;
	}
	if(size> MAX_BUFFER_SIZE)
	{
		size = MAX_BUFFER_SIZE;
	}
	printk("minor %u \n",drv->minor);
	unsigned long int ret = copy_to_user(buf,user_data[drv->minor],size);
	if(ret) //Returns number of byte NOT copied
	{
		return -EFAULT;
	}
	*offset += size;
	return size;
}
static int reverse_uart_release(struct inode *inode,struct file *file){

	printk("R_UART %s  \n",__func__);
	kfree(file->private_data);
	return 0;
}

static long reverse_uart_ioctl (struct file *file,unsigned int cmd, unsigned long arg)
{
/* for debug only*/
	struct _r_uart_drv *drv = file->private_data;
	printk("ioctl minor %u \n",drv->minor);

	switch(cmd)
	{
		case R_UART_BAUD_SET:
			if(copy_from_user(&drv->baud_rate,(unsigned int __user *)arg,sizeof(drv->baud_rate)))
			{
				return -EFAULT;
			}
			printk("R_UART_BAUD_SET %u\n",drv->baud_rate);
			break;

		case R_UART_BAUD_GET:
			if(!access_ok((void __user *)arg,_IOC_SIZE(cmd)))
			{
				return -EFAULT;
			}
			if(put_user(drv->baud_rate,(unsigned int __user *)arg))
			{
				return -EFAULT;
			}
			printk("R_UART_BAUD_GET %u\n",drv->baud_rate);
			break;
		default:
			return -ENOTTY;
	}
	return 0;
}
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = reverse_uart_open,
	.release = reverse_uart_release,
	.read = reverse_uart_read,
	.write = reverse_uart_write,
	.unlocked_ioctl = reverse_uart_ioctl,
};

static int __init uart_drv_init(void)
{
	printk("R_UART %s \n",__func__);
	alloc_chrdev_region(&dev,0,4,"reverse_uart");
/*
 	unsigned int major = 0;
	unsigned int minor = 0;
	major = MAJOR(dev);
	minor = MINOR(dev);
	printk("R_UART major = %u minor = %u \n",major,minor);*/
	cdev_init(&reverse_uart_dev,&fops);  //Initilize the cdev structure with fops
	cdev_add(&reverse_uart_dev,dev,4);
	cls = class_create("reverse_uart");
 /* creates  class struct under sys/class for this driver
  * which triggers udev to create device node under /dev
  * still to populate dev node we need to create the device
  */
      device_create(cls,NULL,MKDEV(MAJOR(dev),0),NULL,"reverse_uart0");
      device_create(cls,NULL,MKDEV(MAJOR(dev),1),NULL,"reverse_uart1");
      device_create(cls,NULL,MKDEV(MAJOR(dev),2),NULL,"reverse_uart2");
      device_create(cls,NULL,MKDEV(MAJOR(dev),3),NULL,"reverse_uart3");
	
	return 0;
}

static void __exit uart_drv_deinit(void)
{
	printk("Vinit %s \n",__func__);
	device_destroy(cls,MKDEV(MAJOR(dev),0));
	device_destroy(cls,MKDEV(MAJOR(dev),1));
	device_destroy(cls,MKDEV(MAJOR(dev),2));
	device_destroy(cls,MKDEV(MAJOR(dev),3));
	class_destroy(cls);
	cdev_del(&reverse_uart_dev);
	unregister_chrdev_region(dev,4);
}

module_init(uart_drv_init);
module_exit(uart_drv_deinit);
