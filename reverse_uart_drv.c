#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>

MODULE_LICENSE("GPL");
static dev_t dev;
static struct cdev reverse_uart_dev;
static struct class *cls;

static int reverse_uart_open(struct inode *inode,struct file *file){
	return 0;
}

static int reverse_uart_release(struct inode *inode,struct file *file){
	return 0;
}
static struct file_operations fops = {
	.owner = THIS_MODULE,
	.open = reverse_uart_open,
	.release = reverse_uart_release,
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
