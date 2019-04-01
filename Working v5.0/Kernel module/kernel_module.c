/************************************************************************************************
* File name   : kernel_module.c                                                                 *
* Authors     : Puneet Bansal and Nachiket Kelkar                                               *
* Description : Kernel module                                                                   *
* Tools used  : GNU make, gcc, arm-linux-gcc                                                    *
************************************************************************************************/
/*Reference: http://derekmolloy.ie/writing-a-linux-kernel-module-part-2-a-character-device/
         : i2c read, write, ioctl, open functions referenced from i2c-dev.c/i2c-dev.handle_bytes
         : Books- Mastering Embedded linux Programming */

#include <linux/kernel.h>         
#include <linux/fs.h>             
#include <linux/uaccess.h>   
#include <linux/init.h>          
#include <linux/module.h>         
#include <linux/device.h>
#include <linux/slab.h>      
#include <linux/i2c.h> 
#include <linux/i2c-dev.h>  

#define  DEVNAME "myi2cdriver"    
#define  CLASSNAME "aesdProj1"   

#define I2C_SLAVE	0x0703

static int    majorno;                  
static struct class*  i2cClass  = NULL; 
static struct device* i2cDevice = NULL; 


/*Function prototypes for the driver*/
static int my_i2c_open(struct inode *, struct file *);
static int my_i2c_release(struct inode *, struct file *);
static ssize_t my_i2c_read(struct file *, char *, size_t, loff_t *);
static ssize_t my_i2c_write(struct file *, const char *, size_t, loff_t *);
static long my_i2c_ioctl(struct file *, unsigned int, unsigned long);


static struct file_operations fops =
{
   .open = my_i2c_open,
   .read = my_i2c_read,
   .write = my_i2c_write,
   .release = my_i2c_release,
   .unlocked_ioctl= my_i2c_ioctl,
};

static int __init Driver_init(void){
   printk(KERN_INFO "i2cDriver: Initializing the Driver LKM\n");
 
   majorno = register_chrdev(0, DEVNAME, &fops);
   if (majorno<0){
           return majorno;
   }
   printk(KERN_INFO "i2cDriver: registered correctly with major number %d\n", majorno);
 
   // Register the device class
   i2cClass = class_create(THIS_MODULE, CLASSNAME);
   if (IS_ERR(i2cClass)){                
      unregister_chrdev(majorno, DEVNAME);
      printk(KERN_ALERT "Failed to register device class\n");
      return PTR_ERR(i2cClass);          
   }
   printk(KERN_INFO "i2cDriver: device class registered correctly\n");
 
   // Register the device driver
   i2cDevice = device_create(i2cClass, NULL, MKDEV(majorno, 0), NULL, DEVNAME);
   if (IS_ERR(i2cDevice)){               
      class_destroy(i2cClass);         
      unregister_chrdev(majorno, DEVNAME);
      printk(KERN_ALERT "Failed to create the device\n");
      return PTR_ERR(i2cDevice);
   }
   printk(KERN_INFO "i2cDriver: device class created correctly\n"); // Made it! device was initialized
   return 0;
}

static int my_i2c_open(struct inode *inode, struct file *file)
{
	
	struct i2c_client *client;
	struct i2c_adapter *i2cadapter;
   unsigned int minorno = iminor(inode);

	i2cadapter = i2c_get_adapter(minorno);
	if (!i2cadapter)
		return -ENODEV;

	client = kzalloc(sizeof(*client), GFP_KERNEL);
	if (!client) {
		i2c_put_adapter(i2cadapter);
		return -ENOMEM;
	}
	snprintf(client->name, I2C_NAME_SIZE, "i2c-dev %d", i2cadapter->nr);

	client->adapter = i2cadapter;
	file->private_data = client;

	return 0;
}

static int my_i2c_release(struct inode *inode, struct file *file)
{
	struct i2c_client *client = file->private_data;

	i2c_put_adapter(client->adapter);
	kfree(client);
	file->private_data = NULL;

	return 0;
}

static ssize_t my_i2c_read(struct file *file, char __user *buf, size_t count,loff_t *offset)
{
	char *tmp;
	int ret;
   struct i2c_client *client = file->private_data;
   printk(KERN_INFO "i2cDriver :  Performing read\n");
	

	if (count > 8192)
		count = 8192;

	tmp = kmalloc(count, GFP_KERNEL);
	if (tmp == NULL)
		return -ENOMEM;

	pr_debug("i2c-dev: i2c-%d reading %zu bytes.\n",iminor(file_inode(file)), count);

	ret = i2c_master_recv(client, tmp, count);
	if (ret >= 0)
		ret = copy_to_user(buf, tmp, count) ? -EFAULT : ret;
	kfree(tmp);
	return ret;
}

static ssize_t my_i2c_write(struct file *file, const char __user *buf,size_t count, loff_t *offset)
{
	int ret;
	char *tmp;
   struct i2c_client *client = file->private_data;
     
   printk(KERN_INFO "i2cDriver: Performing write\n");
	if (count > 8192)
		count = 8192;
   
	tmp = memdup_user(buf, count);
   
	if (IS_ERR(tmp))
		return PTR_ERR(tmp);
    
	pr_debug("i2c-dev: i2c-%d writing %zu bytes.\n",iminor(file_inode(file)), count);
    
	ret = i2c_master_send(client, tmp, count);
   kfree(tmp);
  
	return ret;
}

static long my_i2c_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct i2c_client *client = file->private_data;
	dev_dbg(&client->adapter->dev, "ioctl, cmd=0x%02x, arg=0x%02lx\n",cmd, arg);
   printk(KERN_INFO "i2cDriver: Performing ioctl\n");
	switch (cmd)
   {
	case I2C_SLAVE:
   break;	
	}
	return 0;
}

static void __exit Driver_exit(void){
   device_destroy(i2cClass, MKDEV(majorno, 0));     // remove the device
   class_unregister(i2cClass);                          // unregister the device class
   class_destroy(i2cClass);                             // remove the device class
   unregister_chrdev(majorno, DEVNAME);             // unregister the major number
   printk(KERN_INFO "EBBChar: Goodbye from the LKM!\n");
}
 

module_init(Driver_init);
module_exit(Driver_exit);

MODULE_LICENSE("GPL");          
MODULE_AUTHOR("Puneet Bansal and Nachiket Kelkar");   
MODULE_DESCRIPTION("Linux driver to implement i2c function"); 
MODULE_VERSION("1");  
