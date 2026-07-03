#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#define DEV_MEM_SIZE 512

char device_buffer[DEV_MEM_SIZE];

dev_t device_number;  // This holds device number
		      
// cdev variable
struct cdev pcd_cdev;

// file operations of the driver
struct file_operations pcd_fops;


static int __init pcd_driver_init(void)
{
	// 1. Dynamically Allocate a device number
	alloc_chrdev_region(&device_number, 0, 1, "pcd");
	
	// 2. make a char device registratiom with the VFS 
	// 	initialize the cdev structure with fops
	
	cdev_init(&pcd_cdev,&pcd_fops);

	// 3. Register cdev structure with VFS
	pcd_cdev.owner = THIS_MODULE;
	cdev_add(&pcd_cdev, device_number, 1);

	return 0;
}

static void __exit pcd_driver_cleanup(void)
{

}

module_init(pcd_driver_init);
module_exit(pcd_driver_cleanup);

MODULE_LICENSE("GPL");
//MUDULE_AUTHOR("Prabin Kumar");
//MODULE_DESCRIPTION("A pseudo character driver");

