#include<linux/module.h>
#include<linux/fs.h>
#include<linux/cdev.h>
#include<linux/device.h>
#include<linux/kdev_t.h>
#include<linux/uaccess.h>
#include <linux/platform_device.h>
#include<linux/slab.h>
#include<linux/mod_devicetable.h>
#include<linux/of.h>
#include<linux/of_device.h>
//#include "platform.h"
#include <linux/gpio/consumer.h>    // <-- Add this


#undef pr_fmt
#define pr_fmt(fmt) "%s : " fmt,__func__

static int gpio_probe(struct platform_device *pdev);

/********************** File Operations Functions ***********************************/

static int gpio_remove(struct platform_device *pdev);
static ssize_t gpio_write(struct file *file, const char __user *buf, size_t len, loff_t *off);
static int gpio_open(struct inode *inode, struct file *file);
static ssize_t gpio_read(struct file *file, char __user *buf, size_t len, loff_t *off);

//static const struct file_operations gpio_fops;

static struct file_operations gpio_fops =
{
    .owner = THIS_MODULE,
    .open  = gpio_open,
    .read = gpio_read,
    .write = gpio_write
};


/*
struct of_device_id gpio_dt_match[] =
{
        {.compatible = "pcdev-A1x",.data = (void*)PCDEVA1X},
        {.compatible = "pcdev-B1x",.data = (void*)PCDEVB1X},
        {.compatible = "pcdev-C1x",.data = (void*)PCDEVC1X},
        {.compatible = "pcdev-D1x",.data = (void*)PCDEVD1X}, 
 	{ } //Null termination

};*/

static const struct of_device_id gpio_dt_ids[] =
{
    {
        .compatible = "custom,mygpio",
    },
    { }
};

MODULE_DEVICE_TABLE(of, gpio_dt_ids);

struct gpio_dev
{
	struct platform_device *pdev;
    	struct gpio_desc *gpiod;

    	struct cdev cdev;

    	dev_t devt;

    	struct class *class;

    	struct device *device;
};



static struct platform_driver gpio_driver =
{
    .probe = gpio_probe,
    .remove = gpio_remove,

    .driver =
    {
        .name = "mygpio",

        .of_match_table = gpio_dt_ids,
    },
};


/*
 * *******************************************************
 *
static int gpio_probe(struct platform_device *pdev)
{
    struct gpio_dev *gdev;

    gdev = devm_kzalloc(&pdev->dev,
                        sizeof(*gdev),
                        GFP_KERNEL);

    if (!gdev)
        return -ENOMEM;

    platform_set_drvdata(pdev, gdev);

    gdev->gpiod =
        devm_gpiod_get(&pdev->dev,
                       "led",
                       GPIOD_OUT_LOW);

    if (IS_ERR(gdev->gpiod))
        return PTR_ERR(gdev->gpiod);

    printk("GPIO acquired\n");

    return 0;
}

*******************************************************
*/

static int gpio_probe(struct platform_device *pdev)
{
    struct gpio_dev *gdev;
    unsigned int ret=0;


    gdev = devm_kzalloc(&pdev->dev, sizeof(*gdev), GFP_KERNEL);
    if (!gdev)
    {
        return -ENOMEM;
    }
    platform_set_drvdata(pdev, gdev);

    gdev->gpiod = devm_gpiod_get(&pdev->dev, "led", GPIOD_OUT_LOW);

    if (IS_ERR(gdev->gpiod)) 
    {
    	dev_err(&pdev->dev, "Failed to get GPIO\n");
    	return PTR_ERR(gdev->gpiod);
    }

    dev_info(&pdev->dev, "GPIO acquired\n");

    printk("GPIO acquired\n");

    /* Character device registration starts here */

    ret = alloc_chrdev_region(&gdev->devt, 0, 1, "mygpio");
    if (ret)
    {
        return ret;
    }
    cdev_init(&gdev->cdev, &gpio_fops);

    ret = cdev_add(&gdev->cdev, gdev->devt, 1);
    if (ret)
    {
        goto err_unregister;
    }
    gdev->class = class_create(THIS_MODULE, "mygpio_class");
    if (IS_ERR(gdev->class)) 
    {
        ret = PTR_ERR(gdev->class);
        goto err_cdev;
    }

    device_create(gdev->class, NULL, gdev->devt, NULL, "mygpio");

    return 0;

err_cdev:
    cdev_del(&gdev->cdev);

err_unregister:
    unregister_chrdev_region(gdev->devt, 1);

    return ret;
}






static ssize_t gpio_write(struct file *file, const char __user *buf, size_t len, loff_t *off)
{
    char value;
    struct gpio_dev *gdev = file->private_data;

    if(copy_from_user(&value, buf, 1))
    {
	    return -EFAULT;
    }
    if (value == '1')
    {
        gpiod_set_value(gdev->gpiod, 1);
    }
    else
    {
        gpiod_set_value(gdev->gpiod, 0);
    }
    return len;
}

static int gpio_open(struct inode *inode, struct file *file)
{
    struct gpio_dev *gdev;

    gdev = container_of(inode->i_cdev, struct gpio_dev, cdev);

    file->private_data = gdev;

    return 0;
}

static ssize_t gpio_read(struct file *file, char __user *buf, size_t len, loff_t *off)
{
    char value;
    struct gpio_dev *gdev = file->private_data;
    value = gpiod_get_value(gdev->gpiod);

    value += '0';

    if(copy_to_user(buf, &value, 1))
    {
	    return -EFAULT;
    }
    return 1;
}

/*static struct file_operations gpio_fops =
{
    .owner = THIS_MODULE,
    .open  = gpio_open,
    .read = gpio_read,
    .write = gpio_write
};*/


static int gpio_remove(struct platform_device *pdev)
{
    struct gpio_dev *gdev = platform_get_drvdata(pdev);

    device_destroy(gdev->class, gdev->devt);
    class_destroy(gdev->class);
    cdev_del(&gdev->cdev);
    unregister_chrdev_region(gdev->devt, 1);
    
    printk("Driver removed\n");
    return 0;
}

/********************************************
static int gpio_remove(struct platform_device *pdev)
{
    device_destroy(...);

    class_destroy(...);

    cdev_del(...);

    unregister_chrdev_region(...);

    printk("Driver removed\n");

    return 0;
}
*****************************************/

static int __init gpio_init(void)
{
	pr_info("gpio_dt_driver Initialize");
    return platform_driver_register(&gpio_driver);
}

module_init(gpio_init);

static void __exit gpio_exit(void)
{
	pr_info("gpio_dt_driver exiting");
    platform_driver_unregister(&gpio_driver);
}

module_exit(gpio_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Prabin Kumar");
MODULE_DESCRIPTION("Module is for single GPIO on OFF");
