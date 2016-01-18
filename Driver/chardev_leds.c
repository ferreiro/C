#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>	/* for copy_to_user */
#include <linux/cdev.h>

// modleds imports
#include <asm-generic/errno.h>
#include <linux/init.h>
#include <linux/tty.h>      /* For fg_console */
#include <linux/kd.h>       /* For KDSETLED */
#include <linux/vt_kern.h>
#include <linux/version.h> /* For LINUX_VERSION_CODE */
 
struct tty_driver* kbd_driver= NULL;

 
MODULE_LICENSE("GPL");

/*
 *  Prototypes
 */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev_leds" /* Dev name as it appears in /proc/devices   */
#define BUF_LEN 80		/* Max length of the message from the device */

#define ALL_LEDS_ON 0x7 // modLeds leds maks
#define ALL_LEDS_OFF 0 // modLeds leds maks

/*
 * Global variables are declared as static, so are global within the file.
 */

dev_t start;
struct cdev* chardev=NULL;
static int Device_Open = 0;	/* Is device open?
				 * Used to prevent multiple access to device */
static char msg[BUF_LEN];	/* The msg the device will give when asked */
static char *msg_Ptr;		/* This will be initialized every time the
				   device is opened successfully */
static int counter=0;		/* Tracks the number of times the character
				 * device has been opened */

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

/*
 * This function is called when the module is loaded
 */
int init_module(void)
{
    int major;	/* Major number assigned to our device driver */
    int minor;  /* Minor number assigned to the associated character device */

    // 1: Get available (major,minor) range for the driver
    // Reserves a range of (major, minor) pairs for the driver. 

    if (alloc_chrdev_region (&start, 0, 1, DEVICE_NAME)) {
        printk(KERN_INFO "Can't allocate chrdev_leds_region()");
        return -ENOMEM;
    }
    
    // 2: Allocate and initialize CHARACTER DEVICE (chardev) structure.

    if ((chardev=cdev_alloc())==NULL) {
        printk(KERN_INFO "cdev_alloc() failed ");
        return -ENOMEM;
    }

    cdev_init(chardev,&fops); // Add operations we're implementing
    
    // 3: Add CHARACTER DEVICE (chardev) device to the system.

    if (cdev_add(chardev,start,1)) {
        printk(KERN_INFO "cdev_add() failed ");
        return -ENOMEM;
    }

    major=MAJOR(start);
    minor=MINOR(start);
    
    printk(KERN_INFO "Welcome to CHARDEV_LEDS!!!!!!!!!");
    printk(KERN_INFO "I was assigned major number %d. To talk to\n", major);
    printk(KERN_INFO "the driver, create a dev file with\n");
    printk(KERN_INFO "'sudo mknod -m 666 /dev/%s c %d %d'.\n", DEVICE_NAME, major,minor);
    printk(KERN_INFO "Try to cat and echo to the device file.\n");
    printk(KERN_INFO "Remove the device file and module when done.\n");

    return SUCCESS;
}

/*
 * This function is called when the module is unloaded
 */
void cleanup_module(void)
{
    /* Destroy chardev struture */
    if (chardev)
        cdev_del(chardev);

    /*
     * Unregister the device.
     * Release the (major, minor) pair reserved on init_module
     */
    unregister_chrdev_region(start, 1);
}

 

/*******************************
****** For the LEDS stuff ****** 
********************************/

/* Get driver handler */
struct tty_driver* get_kbd_driver_handler(void)
{
    printk(KERN_INFO "modleds: loading\n");
    printk(KERN_INFO "modleds: fgconsole is %x\n", fg_console);
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32) )
    return vc_cons[fg_console].d->port.tty->driver;
#else
    return vc_cons[fg_console].d->vc_tty->driver;
#endif
}

/* Set led state to that specified by mask */
static inline int set_leds(struct tty_driver* handler, unsigned int mask)
{
#if ( LINUX_VERSION_CODE > KERNEL_VERSION(2,6,32) )
    return (handler->ops->ioctl) (vc_cons[fg_console].d->port.tty, KDSETLED,mask);
#else
    return (handler->ops->ioctl) (vc_cons[fg_console].d->vc_tty, NULL, KDSETLED, mask);
#endif
}

/****************************************
****** Character device operations ****** 
*****************************************/
 
/*
 * Called when a process tries to open the device file, like
 * "cat /dev/chardev"
 */
static int device_open(struct inode *inode, struct file *file)
{
    if (Device_Open)
        return -EBUSY;

    Device_Open++;

    /* Initialize msg */
    sprintf(msg, "I already told you %d times Hello world!\n", counter++);

    /* Initially, this points to the beginning of the message */
    msg_Ptr = msg;

    /* Increase the module's reference counter */
    try_module_get(THIS_MODULE);

    return SUCCESS;
}

/*
 * Called when a process closes the device file.
 */
static int device_release(struct inode *inode, struct file *file)
{
    Device_Open--;		/* We're now ready for our next caller */

    /*
     * Decrement the usage count, or else once you opened the file, you'll
     * never get get rid of the module.
     */
    module_put(THIS_MODULE);

    return 0;
}

/*
 * Called when a process, which already opened the dev file, attempts to
 * read from it.
 */
static ssize_t device_read(struct file *filp,	/* see include/linux/fs.h   */
                           char *buffer,	/* buffer to fill with data */
                           size_t length,	/* length of the buffer     */
                           loff_t * offset)
{
    /*
     * Number of bytes actually written to the buffer
     */
    int bytes_to_read = length;

    /*
     * If we're at the end of the message,
     * return 0 -> end of file
     */
    if (*msg_Ptr == 0)
        return 0;

    /* Make sure we don't read more chars than
     * those remaining to read
     */
    if (bytes_to_read > strlen(msg_Ptr))
        bytes_to_read=strlen(msg_Ptr);

    /*
     * Actually transfer the data onto the userspace buffer.
     * For this task we use copy_to_user() due to security issues
     *
     * Copy the msg_Ptr (source that exists on kernel-space)
     *   to buffer (destination that must exist in user_space) 
     */
    if (copy_to_user(buffer,msg_Ptr,bytes_to_read))
        return -EFAULT;

    /* Update the pointer for the next read operation */
    msg_Ptr+=bytes_to_read;

    /*
     * The read operation returns the actual number of bytes
     * we copied  in the user's buffer
     */
    return bytes_to_read;
}

/*
 * Called when a process writes to dev file: echo "hi" > /dev/chardev
 * len = number of characters a user introduces to our function
 * buff is the text inputed by the user ’\0’) is not included at the end of the array.
 * In implementing the write operation (device_write()) on the special file, characters in the buff parameter1 must not be accessed directly.
 */

// Returns -EFAULT if fails
// https://www.quora.com/Linux-Kernel/How-does-copy_to_user-work
// int copy_to_user(void *dst, const void *src, unsigned int size);

void printDebug(int mask, int numlock, int capslock, int scrolllock) {
    printk(KERN_ALERT "-- Welcome to the write operation.\n");  
    printk(KERN_ALERT "MAsk %d", mask);
    printk(KERN_ALERT "numLock %d\n", numlock);
    printk(KERN_ALERT "capsLock %d\n", capslock);
    printk(KERN_ALERT "scrollLock %d\n", scrolllock);  
}

// REcibimos del usuario, un array de numeros (que crea usando sudo echo 123)
static ssize_t
device_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	char kbuf[len]; // Creating user space array
    int numLock, capsLock, scrollLock; // 0= off | 1 = on 
	int i, bytes_to_write, ledsMask = 0, tmp;
	
	numLock 	= 0; // Turn off (0=off | 1=on)
	capsLock 	= 0; // Turn off
	scrollLock 	= 0; // Turn off

	bytes_to_write = len;
	
	// Transfer date onto the userspace buffer. use copy_to_user() for security issues.
	if (copy_from_user(kbuf,buff,bytes_to_write) != 0) {
		printk(KERN_ALERT "Problems copying....\n"); 
		return -EFAULT;
	} 
	
	for (i = 0; i < bytes_to_write; i++) {
		tmp = kbuf[i];
		if 		(tmp == '1') numLock 	= 1; 
		else if (tmp == '2') capsLock 	= 1;
		else if (tmp == '3') scrollLock = 1;
	}
	
	if (numLock == 1)      
        ledsMask = ledsMask |= 0x2;
	if (capsLock == 1)     
        ledsMask = ledsMask |= 0x4;
	if (scrollLock == 1)   
        ledsMask = ledsMask |= 0x1;

	kbd_driver= get_kbd_driver_handler(); // Create a new driver handler
    set_leds(kbd_driver, ledsMask); // Setting keyboard leds using the mask
    
    printDebug(mask, numlock, capslock, scrolllock);
    
	return bytes_to_write;
}