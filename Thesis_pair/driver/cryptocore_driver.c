#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/delay.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/proc_fs.h>
#include <linux/fcntl.h>

#include "../include/cryptocore_ioctl_header.h"

#define GPIO1_BASE				0xFF709000
#define GPIO1_SPAN				0x00000078

#define H2F_BRIDGE_BASE			0xC0000000

#define MWMAC_RAM_BASE			0x00000000
#define MWMAC_RAM_SPAN			0x00000FFF

#define LW_H2F_BRIDGE_BASE		0xFF200000

#define LEDR_BASE          		0x00000000
#define LEDR_SPAN				0x0000000F

#define HEX3_HEX0_BASE			0x00000020
#define HEX3_HEX0_SPAN			0x0000000F

#define HEX5_HEX4_BASE			0x00000030
#define HEX5_HEX4_SPAN			0x0000000F

#define SW_BASE					0x00000040
#define SW_SPAN					0x0000000F

#define KEY_BASE				0x00000050
#define KEY_SPAN				0x0000000F

#define MWMAC_CMD_BASE			0x00000060
#define MWMAC_CMD_SPAN			0x00000007

#define MWMAC_IRQ_BASE         	0x00000070
#define MWMAC_IRQ_SPAN			0x0000000F

#define TRNG_CMD_BASE			0x00000080
#define TRNG_CMD_SPAN			0x0000000F

#define TRNG_CTR_BASE			0x00000090
#define TRNG_CTR_SPAN			0x0000000F

#define TRNG_TSTAB_BASE			0x000000A0
#define TRNG_TSTAB_SPAN			0x0000000F

#define TRNG_TSAMPLE_BASE		0x000000B0
#define TRNG_TSAMPLE_SPAN		0x0000000F

#define TRNG_IRQ_BASE			0x000000C0
#define TRNG_IRQ_SPAN			0x0000000F

#define TRNG_FIFO_BASE			0x00001000
#define TRNG_FIFO_SPAN			0x00000FFF

#define TIMER_BASE            	0x00002000
#define TIMER_SPAN				0x0000001F

#define KEY_IRQ	 			73
#define MWMAC_IRQ	 		74
#define TRNG_IRQ			75

#define DRIVER_NAME "cryptocore" /* Name des Moduls */


static dev_t cryptocore_dev_number;
static struct cdev *driver_object;
static struct class *cryptocore_class;
static struct device *cryptocore_dev;

volatile u32 *HPS_GPIO1_ptr;
volatile u32 *LEDR_ptr;
volatile u32 *KEY_ptr;
volatile u32 *MWMAC_RAM_ptr;
volatile u32 *MWMAC_CMD_ptr;
volatile u32 *MWMAC_IRQ_ptr;
volatile u32 *TRNG_CMD_ptr;
volatile u32 *TRNG_CTR_ptr;
volatile u32 *TRNG_TSTAB_ptr;
volatile u32 *TRNG_TSAMPLE_ptr;
volatile u32 *TRNG_IRQ_ptr;
volatile u32 *TRNG_FIFO_ptr;

volatile u32 mwmac_irq_var;

volatile u32 trng_words_available;

// CryptoCore Driver Supported Precisions:
static u32 PRIME_PRECISIONS[13][2]={ {192,0x0}, {224,0x1}, {256,0x2}, {320,0x3}, {384,0x4}, {512,0x5}, {768,0x6}, {1024,0x7}, {1536,0x8}, {2048,0x9}, {3072,0xA}, {4096,0xB}, {448, 0xD}};
static u32 BINARY_PRECISIONS[16][2]={ {131,0x0}, {163,0x1}, {176,0x2}, {191,0x3}, {193,0x4}, {208,0x5}, {233,0x6}, {239,0x7}, {272,0x8}, {283,0x9}, {304,0xA}, {359,0xB}, {368,0xC}, {409,0xD}, {431,0xE}, {571,0xF} };

// CryptoCore Driver Function Prototyps:
static void Clear_MWMAC_RAM(void);
static void MWMAC_MontMult(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_MontR(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_MontExp(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_ModAdd(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_ModSub(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_MontMult1(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_MontExpFull(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_ModExp(ModExp_params_t *ModExp_params_ptr);
static void MWMAC_ModRed(ModRed_params_t *ModRed_params_ptr);
// Add further Function Prototypes here...
static void MWMAC_Montgomerize(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_Write_RAM(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_Read_RAM(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_Copy(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_ECC_Point_Add(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_ECC_Point_Double(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_ECC_Point_Multiplication(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_ECC_Point_Add2(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_ECC_Point_Double2(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_DeJacobian(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_Invert_Elem(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_Verification(Core_CMD_t *Core_CMD_ptr);
static void MWMAC_Signcryption(Core_CMD_t *Core_CMD_ptr);


irq_handler_t key_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 led_val;
	led_val = ioread32(LEDR_ptr);
	iowrite32((led_val ^ 0x00000001), LEDR_ptr); // Toggle LEDR[0]

	iowrite32(0x0000000F, KEY_ptr+3);

	return (irq_handler_t) IRQ_HANDLED;
}

irq_handler_t mwmac_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	u32 led_val;
	led_val = ioread32(LEDR_ptr);
	iowrite32((led_val ^ 0x00000001), LEDR_ptr); // Toggle LEDR[0]

	iowrite32(0x00000001, MWMAC_IRQ_ptr+3);
	mwmac_irq_var = 1;

	return (irq_handler_t) IRQ_HANDLED;
}

irq_handler_t trng_irq_handler(int irq, void *dev_id, struct pt_regs *regs)
{
	iowrite32(0x00000001, TRNG_IRQ_ptr+3);

	trng_words_available = 1024;

	return (irq_handler_t) IRQ_HANDLED;
}

static int cryptocore_driver_open ( struct inode *inode, struct file *instance )
{
	dev_info( cryptocore_dev, "cryptocore_driver_open called\n" );
	return 0;
}

static int cryptocore_driver_close ( struct inode *inode, struct file *instance )
{
	dev_info( cryptocore_dev, "cryptocore_driver_close called\n" );
	return 0;
}

static long cryptocore_driver_ioctl( struct file *instance, unsigned int cmd, unsigned long arg)
{
	// Add CryptoCore Structs here and allocate kernel memory...
	Core_CMD_t *Core_CMD_ptr = kmalloc(sizeof(Core_CMD_t), GFP_DMA);
	ModExp_params_t *ModExp_params_ptr = kmalloc(sizeof(ModExp_params_t), GFP_DMA);
	ModRed_params_t *ModRed_params_ptr = kmalloc(sizeof(ModRed_params_t), GFP_DMA);

	int rc;
	u32 i;
	u32 trng_val = 0;

	mwmac_irq_var = 0;

	dev_info( cryptocore_dev, "cryptocore_driver_ioctl called 0x%4.4x %p\n", cmd, (void *) arg );

	switch(cmd) {
		case IOCTL_SET_TRNG_CMD:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_CMD_ptr);
			if(trng_val | 0x00000001) {
				for(i=0; i<60; i++){
					udelay(1000); // Give TRNG FIFO time to fill
				}
			}
			break;
		case IOCTL_SET_TRNG_CTR:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_CTR_ptr);
			break;
		case IOCTL_SET_TRNG_TSTAB:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_TSTAB_ptr);
			break;
		case IOCTL_SET_TRNG_TSAMPLE:
			get_user(trng_val, (u32 *)arg);
			iowrite32(trng_val, TRNG_TSAMPLE_ptr);
			break;
		case IOCTL_READ_TRNG_FIFO:
			trng_val = ioread32(TRNG_FIFO_ptr);
			trng_words_available--;
			put_user(trng_val, (u32 *)arg);
			if(trng_words_available == 0) {
				for(i=0; i<60; i++){
					udelay(1000); // Give TRNG FIFO time to fill
				}
			}
			break;
		case IOCTL_MWMAC_MONTMULT:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_MontMult(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_MONTR:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_MontR(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_WRITE_RAM:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_Write_RAM(Core_CMD_ptr);
			break;
		case IOCTL_MWMAC_READ_RAM:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_Read_RAM(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_MONTEXP:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_MontExp(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_MODADD:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_ModAdd(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_MODSUB:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_ModSub(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_COPY:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_Copy(Core_CMD_ptr);
			break;
		case IOCTL_MWMAC_CLEAR:
			Clear_MWMAC_RAM();
			break;
		case IOCTL_MWMAC_MONTMULT1:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_MontMult1(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_MONTEXP_FULL:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_MontExpFull(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_MODEXP:
			rc = copy_from_user(ModExp_params_ptr, (void *)arg, sizeof(ModExp_params_t));
			MWMAC_ModExp(ModExp_params_ptr);
			rc = copy_to_user((void *)arg, ModExp_params_ptr, sizeof(ModExp_params_t));
			break;
		case IOCTL_MWMAC_MODRED:
			rc = copy_from_user(ModRed_params_ptr, (void *)arg, sizeof(ModRed_params_t));
			MWMAC_ModRed(ModRed_params_ptr);
			rc = copy_to_user((void *)arg, ModRed_params_ptr, sizeof(ModRed_params_t));
			break;
		case IOCTL_MWMAC_ECC_POINT_ADD:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_ECC_Point_Add(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_ECC_POINT_DOUBLE:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_ECC_Point_Double(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_ECC_POINT_MULTIPLICATION:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_ECC_Point_Multiplication(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_DEJACOBIAN:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_DeJacobian(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_INVERT_ELEM:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_Invert_Elem(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_VERIFICATION:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_Verification(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_SIGNCRYPTION:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_Signcryption(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;
		case IOCTL_MWMAC_MONTGOMERIZE:
			rc = copy_from_user(Core_CMD_ptr, (void *)arg, sizeof(Core_CMD_t));
			MWMAC_Montgomerize(Core_CMD_ptr);
			rc = copy_to_user((void *)arg, Core_CMD_ptr, sizeof(Core_CMD_t));
			break;

		// Add further CryptoCore commands here

		default:
			printk("unknown IOCTL 0x%x\n", cmd);

			// Free allocated kernel memory for defined CryptoCore Structs here...
			kfree(Core_CMD_ptr);
			kfree(ModExp_params_ptr);
			kfree(ModRed_params_ptr);

			return -EINVAL;
	}

	// Free allocated kernel memory for defined CryptoCore Structs here...
	kfree(Core_CMD_ptr);
	kfree(ModExp_params_ptr);
	kfree(ModRed_params_ptr);
	return 0;

}

static struct file_operations fops = {
   .owner = THIS_MODULE,
   .open = cryptocore_driver_open,
   .release = cryptocore_driver_close,
   .unlocked_ioctl = cryptocore_driver_ioctl,
};

static int __init cryptocore_init( void )
{
   int value;

   if( alloc_chrdev_region(&cryptocore_dev_number, 0, 1, DRIVER_NAME) < 0 )
      return -EIO;

   driver_object = cdev_alloc(); /* Anmeldeobject reservieren */

   if( driver_object == NULL )
      goto free_device_number;

   driver_object->owner = THIS_MODULE;
   driver_object->ops = &fops;

   if( cdev_add(driver_object, cryptocore_dev_number, 1) )
      goto free_cdev;

   cryptocore_class = class_create( THIS_MODULE, DRIVER_NAME );

   if( IS_ERR( cryptocore_class ) ) {
      pr_err( "cryptocore: no udev support\n");
      goto free_cdev;
   }

   cryptocore_dev = device_create(cryptocore_class, NULL, cryptocore_dev_number, NULL, "%s", DRIVER_NAME );
   dev_info( cryptocore_dev, "cryptocore_init called\n" );

   if(!(request_mem_region(GPIO1_BASE, GPIO1_SPAN, DRIVER_NAME))) {
      pr_err( "timer: request mem_region (GPIO1) failed!\n");
      goto fail_request_mem_region_1;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (LEDR) failed!\n");
      goto fail_request_mem_region_2;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (KEY) failed!\n");
      goto fail_request_mem_region_3;
   }

   if(!(request_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_RAM) failed!\n");
      goto fail_request_mem_region_4;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_CMD) failed!\n");
      goto fail_request_mem_region_5;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (MWMAC_IRQ) failed!\n");
      goto fail_request_mem_region_6;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_CMD) failed!\n");
      goto fail_request_mem_region_7;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_CTR) failed!\n");
      goto fail_request_mem_region_8;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_TSTAB) failed!\n");
      goto fail_request_mem_region_9;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_TSAMPLE) failed!\n");
      goto fail_request_mem_region_10;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_IRQ) failed!\n");
      goto fail_request_mem_region_11;
   }

   if(!(request_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN, DRIVER_NAME))) {
      pr_err( "cryptocore: request mem_region (TRNG_FIFO) failed!\n");
      goto fail_request_mem_region_12;
   }

   if(!(HPS_GPIO1_ptr = ioremap(GPIO1_BASE, GPIO1_SPAN))) {
      pr_err( "cryptocore: ioremap (GPIO1) failed!\n");
      goto fail_ioremap_1;
   }

   if(!(LEDR_ptr = ioremap(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN))) {
      pr_err( "cryptocore: ioremap (LEDR) failed!\n");
      goto fail_ioremap_2;
   }

   if(!(KEY_ptr = ioremap(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN))) {
      pr_err( "cryptocore: ioremap (KEY) failed!\n");
      goto fail_ioremap_3;
   }

   if(!(MWMAC_RAM_ptr = ioremap(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_RAM) failed!\n");
      goto fail_ioremap_4;
   }

   if(!(MWMAC_CMD_ptr = ioremap(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_CMD) failed!\n");
      goto fail_ioremap_5;
   }

   if(!(MWMAC_IRQ_ptr = ioremap(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN))) {
      pr_err( "cryptocore: ioremap (MWMAC_IRQ) failed!\n");
      goto fail_ioremap_6;
   }

   if(!(TRNG_CMD_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_CMD) failed!\n");
      goto fail_ioremap_7;
   }

   if(!(TRNG_CTR_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_CTR) failed!\n");
      goto fail_ioremap_8;
   }

   if(!(TRNG_TSTAB_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_TSTAB) failed!\n");
      goto fail_ioremap_9;
   }

   if(!(TRNG_TSAMPLE_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_TSAMPLE) failed!\n");
      goto fail_ioremap_10;
   }

   if(!(TRNG_IRQ_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_IRQ) failed!\n");
      goto fail_ioremap_11;
   }

   if(!(TRNG_FIFO_ptr = ioremap(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN))) {
      pr_err( "cryptocore: ioremap (TRNG_FIFO) failed!\n");
      goto fail_ioremap_12;
   }

   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x00000001, MWMAC_IRQ_ptr+3);
   // Enable IRQ generation for the CryptoCore
   iowrite32(0x00000001, MWMAC_IRQ_ptr+2);

   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x00000001, TRNG_IRQ_ptr+3);
   // Enable IRQ generation for the CryptoCore TRNG
   iowrite32(0x00000001, TRNG_IRQ_ptr+2);


   // Clear the PIO edgecapture register (clear any pending interrupt)
   iowrite32(0x0000000F, KEY_ptr+3);
   // Enable IRQ generation for the 4 buttons
   iowrite32(0x0000000F, KEY_ptr+2);

   value = request_irq (KEY_IRQ, (irq_handler_t) key_irq_handler, IRQF_SHARED, "cryptocore_key_irq_handler", (void *) (key_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (KEY) failed!\n");
      goto fail_irq_1;
   }

   value = request_irq (MWMAC_IRQ, (irq_handler_t) mwmac_irq_handler, IRQF_SHARED, "cryptocore_mwmac_irq_handler", (void *) (mwmac_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (MWMAC) failed!\n");
      goto fail_irq_2;
   }

   value = request_irq (TRNG_IRQ, (irq_handler_t) trng_irq_handler, IRQF_SHARED, "cryptocore_trng_irq_handler", (void *) (trng_irq_handler));
   if(value) {
      pr_err( "cryptocore: request_irq (TRNG) failed!\n");
      goto fail_irq_3;
   }

   // Turn on green User LED
   iowrite32(0x01000000, (HPS_GPIO1_ptr+1));
   iowrite32(0x01000000, (HPS_GPIO1_ptr));

   return 0;

fail_irq_3:
   free_irq (MWMAC_IRQ, (void*) mwmac_irq_handler);

fail_irq_2:
   free_irq (KEY_IRQ, (void*) key_irq_handler);

fail_irq_1:
   iounmap(TRNG_FIFO_ptr);

fail_ioremap_12:
   iounmap(TRNG_IRQ_ptr);

fail_ioremap_11:
   iounmap(TRNG_TSAMPLE_ptr);

fail_ioremap_10:
   iounmap(TRNG_TSTAB_ptr);

fail_ioremap_9:
   iounmap(TRNG_CTR_ptr);

fail_ioremap_8:
   iounmap(TRNG_CMD_ptr);

fail_ioremap_7:
   iounmap(MWMAC_IRQ_ptr);

fail_ioremap_6:
   iounmap(MWMAC_CMD_ptr);

fail_ioremap_5:
   iounmap(MWMAC_RAM_ptr);

fail_ioremap_4:
   iounmap(KEY_ptr);

fail_ioremap_3:
   iounmap(LEDR_ptr);

fail_ioremap_2:
   iounmap(HPS_GPIO1_ptr);

fail_ioremap_1:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN);

fail_request_mem_region_12:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN);

fail_request_mem_region_11:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN);

fail_request_mem_region_10:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN);

fail_request_mem_region_9:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN);

fail_request_mem_region_8:
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN);

fail_request_mem_region_7:
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN);

fail_request_mem_region_6:
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN);

fail_request_mem_region_5:
   release_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN);

fail_request_mem_region_4:
   release_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN);

fail_request_mem_region_3:
   release_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN);

fail_request_mem_region_2:
   release_mem_region(GPIO1_BASE, GPIO1_SPAN);

fail_request_mem_region_1:
   device_destroy( cryptocore_class, cryptocore_dev_number );
   class_destroy( cryptocore_class );

free_cdev:
   kobject_put( &driver_object->kobj );

free_device_number:
   unregister_chrdev_region( cryptocore_dev_number, 1 );
   return -EIO;
}

static void __exit cryptocore_exit( void )
{
   dev_info( cryptocore_dev, "cryptocore_exit called\n" );

   iowrite32(0x00000000, (LEDR_ptr));
   iowrite32(0x00000000, (HPS_GPIO1_ptr));
   iowrite32(0x00000000, (HPS_GPIO1_ptr+1));

   free_irq (KEY_IRQ, (void*) key_irq_handler);
   free_irq (MWMAC_IRQ, (void*) mwmac_irq_handler);
   free_irq (TRNG_IRQ, (void*) trng_irq_handler);

   iounmap(TRNG_FIFO_ptr);
   iounmap(TRNG_IRQ_ptr);
   iounmap(TRNG_TSAMPLE_ptr);
   iounmap(TRNG_TSTAB_ptr);
   iounmap(TRNG_CTR_ptr);
   iounmap(TRNG_CMD_ptr);
   iounmap(MWMAC_IRQ_ptr);
   iounmap(MWMAC_CMD_ptr);
   iounmap(MWMAC_RAM_ptr);
   iounmap(KEY_ptr);
   iounmap(LEDR_ptr);
   iounmap(HPS_GPIO1_ptr);

   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_FIFO_BASE, TRNG_FIFO_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_IRQ_BASE, TRNG_IRQ_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSAMPLE_BASE, TRNG_TSAMPLE_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_TSTAB_BASE, TRNG_TSTAB_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CTR_BASE, TRNG_CTR_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + TRNG_CMD_BASE, TRNG_CMD_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_IRQ_BASE, MWMAC_IRQ_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + MWMAC_CMD_BASE, MWMAC_CMD_SPAN);
   release_mem_region(H2F_BRIDGE_BASE + MWMAC_RAM_BASE, MWMAC_RAM_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + KEY_BASE, KEY_SPAN);
   release_mem_region(LW_H2F_BRIDGE_BASE + LEDR_BASE, LEDR_SPAN);
   release_mem_region(GPIO1_BASE, GPIO1_SPAN);

   device_destroy( cryptocore_class, cryptocore_dev_number );
   class_destroy( cryptocore_class );

   cdev_del( driver_object );
   unregister_chrdev_region( cryptocore_dev_number, 1 );

   return;
}

static void Clear_MWMAC_RAM(void)
{
	u32 value;
	u32 i;

	value = 0x00000000;

	for(i=0; i<128; i++){
		// Clear B
		iowrite32(value, (MWMAC_RAM_ptr+0x3+i*0x4));
		// Clear P
		iowrite32(value, (MWMAC_RAM_ptr+0x2+i*0x4));
		// Clear TS
		iowrite32(value, (MWMAC_RAM_ptr+0x1+i*0x4));
		// Clear TC
		iowrite32(value, (MWMAC_RAM_ptr+0x0+i*0x4));
		// Clear A
		iowrite32(value, (MWMAC_RAM_ptr+0x200+i));
		// Clear E
		iowrite32(value, (MWMAC_RAM_ptr+0x280+i));
		// Clear X
		iowrite32(value, (MWMAC_RAM_ptr+0x300+i));
	}

}

static void MWMAC_MontMult(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	// MontMult(A1, B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (Core_CMD_ptr->sec_calc << 3) | (mwmac_cmd_prec << 4) | (Core_CMD_ptr->cmd << 8)
	//			src_addr     			dest_addr    		src_addr_e   src_addr_x
			| (Core_CMD_ptr->src << 12) | (Core_CMD_ptr->dest << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
}

static void MWMAC_MontR(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	//u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	// MontR(P1, B1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (Core_CMD_ptr->sec_calc << 3) | (mwmac_cmd_prec << 4) | (Core_CMD_ptr->cmd << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (Core_CMD_ptr->src << 12) | (Core_CMD_ptr->dest << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

}

static void MWMAC_Write_RAM(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	//Clear_MWMAC_RAM();
	switch(Core_CMD_ptr->dest) {
		case MWMAC_RAM_B:
			for(i=0; i<rw_prec/32; i++){
				iowrite32(Core_CMD_ptr->B[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
			} break;
		case MWMAC_RAM_P:
			for(i=0; i<rw_prec/32; i++){
				iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
			} break;
		case MWMAC_RAM_TS:
			for(i=0; i<rw_prec/32; i++){
				iowrite32(Core_CMD_ptr->TS[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x1+i*0x4));
			} break;
		case MWMAC_RAM_TC:
			for(i=0; i<rw_prec/32; i++){
				iowrite32(Core_CMD_ptr->TC[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x0+i*0x4));
			} break;
		case MWMAC_RAM_A:
			for(i=0; i<rw_prec/32; i++){
				iowrite32(Core_CMD_ptr->A[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x200+i));
			} break;
		case MWMAC_RAM_E:
			for(i=0; i<rw_prec/32; i++){
				iowrite32(Core_CMD_ptr->E[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
			} break;
		case MWMAC_RAM_X:
			for(i=0; i<rw_prec/32; i++){
				iowrite32(Core_CMD_ptr->X[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+i));
			} break;
	}
}


static void MWMAC_Read_RAM(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	//Clear_MWMAC_RAM();
	switch(Core_CMD_ptr->dest) {
		case MWMAC_RAM_B:
			for(i=0; i<rw_prec/32; i++){
				Core_CMD_ptr->B[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
			} break;
		case MWMAC_RAM_P:
			for(i=0; i<rw_prec/32; i++){
				Core_CMD_ptr->P[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x2+i*0x4);
			} break;
		case MWMAC_RAM_TS:
			for(i=0; i<rw_prec/32; i++){
				Core_CMD_ptr->TS[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x1+i*0x4);
			} break;
		case MWMAC_RAM_TC:
			for(i=0; i<rw_prec/32; i++){
				Core_CMD_ptr->TC[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x0+i*0x4);
			} break;
		case MWMAC_RAM_A:
			for(i=0; i<rw_prec/32; i++){
				Core_CMD_ptr->A[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x200+i);
			} break;
		case MWMAC_RAM_E:
			for(i=0; i<rw_prec/32; i++){
				Core_CMD_ptr->E[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x280+i);
			} break;
		case MWMAC_RAM_X:
			for(i=0; i<rw_prec/32; i++){
				Core_CMD_ptr->X[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x300+i);
			} break;
	}
}

static void MWMAC_MontExp(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	if(Core_CMD_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}
	
	//CLEAR RAM just in case
	//Clear_MWMAC_RAM();
	
	// Write Parameters to Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
		iowrite32(Core_CMD_ptr->E[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
		iowrite32(Core_CMD_ptr->X[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x320+i));
		//iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x42+i*0x4));
	}

	// MontR(P1, B1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8)
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// MontExp(A1, B1, E1, X1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTEXP << 8)
	//			src_addr      			dest_addr    		src_addr_e   			src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (MWMAC_RAM_E1 << 22) | (MWMAC_RAM_X1 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

}

static void MWMAC_ModAdd(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	// ModAdd(TS1, TC1)
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (Core_CMD_ptr->sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODADD << 8)
	//			src_addr      			dest_addr    src_addr_e   src_addr_x
			| (Core_CMD_ptr->src << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
}

static void MWMAC_ModSub(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	if(Core_CMD_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}

	// ModSub(TS1, TC1)
	//            Start     Abort       f_sel     sec_calc        precision              operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MODSUB << 8)
	//			src_addr      			dest_addr    src_addr_e   src_addr_x
			| (Core_CMD_ptr->src << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
}

static void MWMAC_MontMult1(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	if(Core_CMD_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}

	//Clear_MWMAC_RAM();

	/*// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// Write Parameter b to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->B[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}*/

	// MontMult1(B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (Core_CMD_ptr->cmd << 8)
	//			src_addr      dest_addr    src_addr_e   src_addr_x
			| (Core_CMD_ptr->src << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// Read Result c from B Register Memory
 	/*for(i=0; i<rw_prec/32; i++){
		Core_CMD_ptr->B[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	}*/
}

static void MWMAC_MontExpFull(Core_CMD_t *Core_CMD_ptr)
{
	/*
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(MontExp_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==MontExp_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(MontExp_params_ptr->prec % 32) {
					rw_prec = (MontExp_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = MontExp_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==MontExp_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = MontExp_params_ptr->prec;
			}
		}
	}

	if(MontExp_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}

	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// Write Parameter e to E Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->e[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
	}

	// Write Parameter b to X Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(MontExp_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x300+i));
	}

	// MontR(P1, B1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8)
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// MontExp(A1, B1, E1, X1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTEXPFULL << 8)
	//			src_addr      			dest_addr    		src_addr_e   			src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (MWMAC_RAM_E1 << 22) | (MWMAC_RAM_X1 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// Read Result c from B Register Memory
 	for(i=0; i<rw_prec/32; i++){
		MontExp_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	} */
}

static void MWMAC_ModExp(ModExp_params_t *ModExp_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_prec = 0;

	if(ModExp_params_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==ModExp_params_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(ModExp_params_ptr->prec % 32) {
					rw_prec = (ModExp_params_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = ModExp_params_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ModExp_params_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = ModExp_params_ptr->prec;
			}
		}
	}

	if(ModExp_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}

	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModExp_params_ptr->n[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// MontR2(P1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR2 << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8)
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// Write Parameter b to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModExp_params_ptr->b[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}

	// Write Parameter e to E Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(ModExp_params_ptr->e[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
	}

	// MontMult(A1, B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT << 8)
	//			src_addr     			dest_addr    		src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, X1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_X1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// MontR(P1, B1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTR << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_B1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (COPYH2V << 8)
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// MontExp(A1, B1, E1, X1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTEXP << 8)
	//			src_addr      			dest_addr    		src_addr_e   			src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (MWMAC_RAM_E1 << 22) | (MWMAC_RAM_X1 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// MontMult1(B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_prec << 4) | (MONTMULT1 << 8)
	//			src_addr      dest_addr    src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (0x0 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// Read Result c from B Register Memory
 	for(i=0; i<rw_prec/32; i++){
		ModExp_params_ptr->c[rw_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	}
}

static void MWMAC_ModRed(ModRed_params_t *ModRed_params_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_full_prec = 0;
	u32 mwmac_cmd_half_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 mwmac_sec_calc = 0;
	u32 rw_full_prec = 0;
	u32 rw_half_prec = 0;

	if(ModRed_params_ptr->f_sel == 0) {
		// Modular Reduction is not supported in GF(2^m)
		mwmac_f_sel = 0;
//		for(i=0; i<16; i++){
//			if(BINARY_PRECISIONS[i][0]==ModRed_params_ptr->prec){
//				mwmac_cmd_full_prec = BINARY_PRECISIONS[i][1];
//			}
//			if(BINARY_PRECISIONS[i][0]==ModRed_params_ptr->prec/2){
//				mwmac_cmd_half_prec = BINARY_PRECISIONS[i][1];
//			}
//		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==ModRed_params_ptr->prec){
				mwmac_cmd_full_prec = PRIME_PRECISIONS[i][1];
				rw_full_prec = ModRed_params_ptr->prec;
			}
			if(PRIME_PRECISIONS[i][0]==ModRed_params_ptr->prec/2){
				mwmac_cmd_half_prec = PRIME_PRECISIONS[i][1];
				rw_half_prec = ModRed_params_ptr->prec/2;
			}
		}
	}

	if(ModRed_params_ptr->sec_calc == 0) {
		mwmac_sec_calc = 0;
	}
	else {
		mwmac_sec_calc = 1;
	}

	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_half_prec/32; i++){
		iowrite32(ModRed_params_ptr->n[rw_half_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
	}

	// MontR2(P1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_half_prec << 4) | (MONTR2 << 8)
	//			src_addr      			dest_addr    		src_addr_e   	src_addr_x
			| (MWMAC_RAM_P1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | 	(0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_half_prec << 4) | (COPYH2V << 8)
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// Write Parameter a to B Register Memory
	for(i=0; i<rw_full_prec/32; i++){
		iowrite32(ModRed_params_ptr->a[rw_full_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
	}

	// MontMult(A1, B1, P1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (mwmac_sec_calc << 3) | (mwmac_cmd_full_prec << 4) | (MONTMULT << 8)
	//			src_addr     			dest_addr    		src_addr_e   src_addr_x
			| (MWMAC_RAM_B1 << 12) | (MWMAC_RAM_A1 << 17) | (0x0 << 22) | (0x0 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;

	// Read Result c from B Register Memory
 	for(i=0; i<rw_full_prec/32; i++){
		ModRed_params_ptr->c[rw_full_prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	}
}

// Add further CryptoCore Functions here...

static void MWMAC_Montgomerize(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	//u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}
	
	//CLEAR RAM just in case
	Clear_MWMAC_RAM();
	
	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x42+i*0x4));
	}
	
	//Calculate R2
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->src = MWMAC_RAM_P1;
	Core_CMD_ptr->cmd = MONTR2;
	MWMAC_MontR(Core_CMD_ptr);
	
	//Copy R2 -> A1
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);	
	

	// Write Parameters to X Register Memory
	// Write X to Register
	for(i=0; i < Core_CMD_ptr->prec/32; i++){
		iowrite32(Core_CMD_ptr->B[(rw_prec/32-1-i)], (MWMAC_RAM_ptr+0x3+i*0x4));
		iowrite32(Core_CMD_ptr->B[(rw_prec/32-1-i)+16], (MWMAC_RAM_ptr+0x43+i*0x4));
	}
	
	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);
	
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		Core_CMD_ptr->B[(Core_CMD_ptr->prec/32-1-i)] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
		Core_CMD_ptr->B[(Core_CMD_ptr->prec/32-1-i)+16] = ioread32(MWMAC_RAM_ptr+0x43+i*0x4);
	}
	
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->src = MWMAC_RAM_P1;
	Core_CMD_ptr->cmd = MONTR;
	MWMAC_MontR(Core_CMD_ptr);
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		Core_CMD_ptr->B[(Core_CMD_ptr->prec/32-1-i)+32] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	}
	
}


static void MWMAC_Copy(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd = 0;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	// CopyH2V(B1, A1)
	//            Start     Abort       f_sel     sec_calc        precision         operation
	mwmac_cmd = (1 << 0) | (0 << 1) | (mwmac_f_sel << 2) | (Core_CMD_ptr->sec_calc << 3) | (mwmac_cmd_prec << 4) | (Core_CMD_ptr->cmd << 8)
	//			src_addr      				dest_addr   	 src_addr_e   src_addr_x
			| (Core_CMD_ptr->src << 12) | (Core_CMD_ptr->dest << 17) | (0x0 << 22) | (0x1 << 27);
	iowrite32(mwmac_cmd, MWMAC_CMD_ptr);
	while(!mwmac_irq_var);
	mwmac_irq_var = 0;
}

static void MWMAC_ECC_Point_Add(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 help = 0;

	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x42+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x82+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0xC2+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x102+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x142+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x182+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x1C2+i*0x4));
	}

	// Write Parameters to X Register Memory
	// Write X to Register
	for(i=0; i < Core_CMD_ptr->prec/32; i++){
		iowrite32(Core_CMD_ptr->X[(Core_CMD_ptr->prec/32-1-i)+80], (MWMAC_RAM_ptr+0x350+i));
		iowrite32(Core_CMD_ptr->X[(Core_CMD_ptr->prec/32-1-i)+96], (MWMAC_RAM_ptr+0x360+i));
	}
	iowrite32(Core_CMD_ptr->X[0], (MWMAC_RAM_ptr+0x310));
	// Write Parameters to E Register Memory
	for(i=0; i < Core_CMD_ptr->prec/32; i++){
		iowrite32(Core_CMD_ptr->E[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+16], (MWMAC_RAM_ptr+0x290+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+32], (MWMAC_RAM_ptr+0x2A0+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+48], (MWMAC_RAM_ptr+0x2B0+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+64], (MWMAC_RAM_ptr+0x2C0+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+80], (MWMAC_RAM_ptr+0x2D0+i));
	}

	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->src = MWMAC_RAM_P1;
	Core_CMD_ptr->cmd = MONTR2;
	MWMAC_MontR(Core_CMD_ptr);

	//Copy R2 -> X1
	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	//Montgomery 2
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->src = MWMAC_RAM_X2;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X2;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	//U1 = X1 * Z2^2
	Core_CMD_ptr->src = MWMAC_RAM_E6;
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E6;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	MWMAC_MontMult(Core_CMD_ptr);

	//U2 = X2 * Z1^2
	Core_CMD_ptr->src = MWMAC_RAM_E3;
	Core_CMD_ptr->dest = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E3;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E4;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	//S1 = Y1 * Z2^3
	Core_CMD_ptr->src = MWMAC_RAM_E6;
	Core_CMD_ptr->dest = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E6;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	MWMAC_MontMult(Core_CMD_ptr);
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E2;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	MWMAC_MontMult(Core_CMD_ptr);

	//S2 = Y2 * Z1^3
	Core_CMD_ptr->src = MWMAC_RAM_E3;
	Core_CMD_ptr->dest = MWMAC_RAM_B4;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E3;
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	MWMAC_MontMult(Core_CMD_ptr);
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E5;
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	MWMAC_MontMult(Core_CMD_ptr);

	//H = U2 - U1
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_TS2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_TC2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS2;
	MWMAC_ModSub(Core_CMD_ptr);

	//CHECK IF U1 == U2
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		if (0 == ioread32(MWMAC_RAM_ptr+0x43+i*0x4))
		{
			help = 1;
		}
		else
		{
			help = 0;
			break;
		}
	}
	if (help == 1) {
		//Point Double
				MWMAC_ECC_Point_Double(Core_CMD_ptr);
				return;
	}
	
	//HH = H^2
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	//HHH = H * HH

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	//r = S2 - S1

	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_TC4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_TS4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS4;
	MWMAC_ModSub(Core_CMD_ptr);

	//V = U1 * HH

	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	//X3 = r^2 - HHH - 2 * V

	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_A3;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_A3;
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_B5;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B5;
	Core_CMD_ptr->dest = MWMAC_RAM_X2;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_TS4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_TC4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS4;
	MWMAC_ModSub(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_TS4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B5;
	Core_CMD_ptr->dest = MWMAC_RAM_TC4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS4;
	MWMAC_ModSub(Core_CMD_ptr);

	// Y3 = r * (V-X3) - S1*HHH

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_A4;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_A4;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_TS1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_TC1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS1;
	MWMAC_ModSub(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_A3;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_TS1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_TC1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS1;
	MWMAC_ModSub(Core_CMD_ptr);

	//Z3 = Z1*Z2*H

	Core_CMD_ptr->src = MWMAC_RAM_E3;
	Core_CMD_ptr->dest = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E6;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	//Copy back to E
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_E1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_E2;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_E3;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);


	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)] = ioread32(MWMAC_RAM_ptr+0x280+i);
		Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+16] = ioread32(MWMAC_RAM_ptr+0x290+i);
		Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+32] = ioread32(MWMAC_RAM_ptr+0x2A0+i);
	}
}

static void MWMAC_ECC_Point_Double(Core_CMD_t *Core_CMD_ptr)
{
	
	u32 i;
	
	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x42+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x82+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0xC2+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x102+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x142+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x182+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x1C2+i*0x4));
	}
	// Write X to Register
	for(i=0; i < Core_CMD_ptr->prec/32; i++){
		iowrite32(Core_CMD_ptr->X[(Core_CMD_ptr->prec/32-1-i)+80], (MWMAC_RAM_ptr+0x350+i));
		iowrite32(Core_CMD_ptr->X[(Core_CMD_ptr->prec/32-1-i)+96], (MWMAC_RAM_ptr+0x360+i));
	}
	iowrite32(Core_CMD_ptr->X[0], (MWMAC_RAM_ptr+0x310));
	iowrite32(Core_CMD_ptr->X[1], (MWMAC_RAM_ptr+0x320));
	iowrite32(Core_CMD_ptr->X[2], (MWMAC_RAM_ptr+0x330));
	iowrite32(Core_CMD_ptr->X[3], (MWMAC_RAM_ptr+0x340));

	// Write Parameters to E Register Memory
	for(i=0; i < Core_CMD_ptr->prec/32; i++){
		iowrite32(Core_CMD_ptr->E[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+16], (MWMAC_RAM_ptr+0x290+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+32], (MWMAC_RAM_ptr+0x2A0+i));
	}
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->src = MWMAC_RAM_P1;
	Core_CMD_ptr->cmd = MONTR2;
	MWMAC_MontR(Core_CMD_ptr);

	//rw_prec = 192;
	//Copy R2 -> X1
	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);
	
	//Montgomery 2,3,4 and 8
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->src = MWMAC_RAM_X2;
	Core_CMD_ptr->cmd = COPYV2H;
	//Core_CMD_ptr->prec = 2048;
	MWMAC_Copy(Core_CMD_ptr);
	Core_CMD_ptr->dest = MWMAC_RAM_B2;
	Core_CMD_ptr->src = MWMAC_RAM_X3;
	Core_CMD_ptr->cmd = COPYV2H;
	//Core_CMD_ptr->prec = 2048;
	MWMAC_Copy(Core_CMD_ptr);
	Core_CMD_ptr->dest = MWMAC_RAM_B3;
	Core_CMD_ptr->src = MWMAC_RAM_X4;
	Core_CMD_ptr->cmd = COPYV2H;
	//Core_CMD_ptr->prec = 2048;
	MWMAC_Copy(Core_CMD_ptr);
	Core_CMD_ptr->dest = MWMAC_RAM_B4;
	Core_CMD_ptr->src = MWMAC_RAM_X5;
	Core_CMD_ptr->cmd = COPYV2H;
	//Core_CMD_ptr->prec = 2048;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->dest = MWMAC_RAM_X2;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = COPYH2V;
	//Core_CMD_ptr->prec = 2048;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->dest = MWMAC_RAM_X3;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = COPYH2V;
	//Core_CMD_ptr->prec = 2048;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->dest = MWMAC_RAM_X4;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = COPYH2V;
	//Core_CMD_ptr->prec = 2048;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->dest = MWMAC_RAM_X5;
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->cmd = COPYH2V;
	//Core_CMD_ptr->prec = 2048;
	MWMAC_Copy(Core_CMD_ptr);

	
	// S = 4 * X * Y^2
	Core_CMD_ptr->src = MWMAC_RAM_E2;
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = COPYV2H;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->dest = MWMAC_RAM_E2;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->dest = MWMAC_RAM_E1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->dest = MWMAC_RAM_X4;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);
	
	// M = 3 * X^2 + a * Z^4
	Core_CMD_ptr->src = MWMAC_RAM_E1;
	Core_CMD_ptr->dest = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = COPYV2H;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_E1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X3;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_E3;
	Core_CMD_ptr->dest = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = COPYV2H;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_Copy(Core_CMD_ptr);

	for (i = 0; i < 3; i++) {
		Core_CMD_ptr->dest = MWMAC_RAM_E3;
		Core_CMD_ptr->src = MWMAC_RAM_B3;
		Core_CMD_ptr->cmd = MONTMULT;
		//Core_CMD_ptr->prec = rw_prec;
		MWMAC_MontMult(Core_CMD_ptr);
	}

	Core_CMD_ptr->dest = MWMAC_RAM_X6;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_TS2;
	Core_CMD_ptr->cmd = COPYH2H;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_TC2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS2;
	MWMAC_ModAdd(Core_CMD_ptr);

	//X' M^2 - 2S
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X2;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_TS1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_TC1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS1;
	MWMAC_ModSub(Core_CMD_ptr);

	//Y' = M * (S - X') - 8 * Y^4
	Core_CMD_ptr->src = MWMAC_RAM_A1;
	Core_CMD_ptr->dest = MWMAC_RAM_TS2;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_TC2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS2;
	MWMAC_ModSub(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_E2;
	Core_CMD_ptr->dest = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	for (i = 0; i < 3; i++) {
		Core_CMD_ptr->dest = MWMAC_RAM_E2;
		Core_CMD_ptr->src = MWMAC_RAM_B3;
		Core_CMD_ptr->cmd = MONTMULT;
		//Core_CMD_ptr->prec = rw_prec;
		MWMAC_MontMult(Core_CMD_ptr);
	}

	Core_CMD_ptr->dest = MWMAC_RAM_X5;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_TS2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_TC2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS2;
	MWMAC_ModSub(Core_CMD_ptr);

	// Z' 2 * Y * Z
	Core_CMD_ptr->src = MWMAC_RAM_E2;
	Core_CMD_ptr->dest = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X2;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_E3;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);
	
	//Write Point to E
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		Core_CMD_ptr->E[Core_CMD_ptr->prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
		Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+16] = ioread32(MWMAC_RAM_ptr+0x43+i*0x4);
		Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+32] = ioread32(MWMAC_RAM_ptr+0x83+i*0x4);
	}
}

static void MWMAC_ECC_Point_Multiplication(Core_CMD_t *Core_CMD_ptr)
{
	u32 i, one = 0, helper;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	Clear_MWMAC_RAM();
	
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x42+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x82+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0xC2+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x102+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x142+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x182+i*0x4));
		iowrite32(Core_CMD_ptr->P[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x1C2+i*0x4));
	}
	// Write X to Register
	for(i=0; i < Core_CMD_ptr->prec/32; i++){
		iowrite32(Core_CMD_ptr->X[(Core_CMD_ptr->prec/32-1-i)+80], (MWMAC_RAM_ptr+0x350+i));
		iowrite32(Core_CMD_ptr->X[(Core_CMD_ptr->prec/32-1-i)+96], (MWMAC_RAM_ptr+0x360+i));
	}
	iowrite32(Core_CMD_ptr->X[0], (MWMAC_RAM_ptr+0x310));
	iowrite32(Core_CMD_ptr->X[1], (MWMAC_RAM_ptr+0x320));
	iowrite32(Core_CMD_ptr->X[2], (MWMAC_RAM_ptr+0x330));
	iowrite32(Core_CMD_ptr->X[3], (MWMAC_RAM_ptr+0x340));
	
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->src = MWMAC_RAM_P1;
	Core_CMD_ptr->cmd = MONTR2;
	MWMAC_MontR(Core_CMD_ptr);

	//Copy R2 -> X1
	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);
	
	//Montgomery 2,3,4 and 8
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->src = MWMAC_RAM_X2;
	Core_CMD_ptr->cmd = COPYV2H;
	Core_CMD_ptr->prec = 2048;
	MWMAC_Copy(Core_CMD_ptr);
		
	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->dest = MWMAC_RAM_X2;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = COPYH2V;
	Core_CMD_ptr->prec = 2048;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->prec = rw_prec;
	
	one = 1 << 31;
	for(i = 0; rw_prec > i; i++)
	{
		memcpy(&helper, &Core_CMD_ptr->A[(i/32)], 4);
		if (one & helper)
		{
			memcpy(&Core_CMD_ptr->E[0], &Core_CMD_ptr->E[48], 64);
			memcpy(&Core_CMD_ptr->E[16], &Core_CMD_ptr->E[64], 64);
			memcpy(&Core_CMD_ptr->E[32], &Core_CMD_ptr->E[80], 64);
			rw_prec = i;
			Core_CMD_ptr->A[(rw_prec/32)] = Core_CMD_ptr->A[(rw_prec/32)] << 1;
			rw_prec++;
			break;
		}
		Core_CMD_ptr->A[(i/32)] = Core_CMD_ptr->A[(i/32)] << 1;
	}
	for(i=0; i < Core_CMD_ptr->prec/32; i++){
		iowrite32(Core_CMD_ptr->E[Core_CMD_ptr->prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+16], (MWMAC_RAM_ptr+0x290+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+32], (MWMAC_RAM_ptr+0x2A0+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+48], (MWMAC_RAM_ptr+0x2B0+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+64], (MWMAC_RAM_ptr+0x2C0+i));
		iowrite32(Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+80], (MWMAC_RAM_ptr+0x2D0+i));
	}

	for (; rw_prec < Core_CMD_ptr->prec; rw_prec++)
	{
		MWMAC_ECC_Point_Double2(Core_CMD_ptr);
		memcpy(&helper, &Core_CMD_ptr->A[(rw_prec/32)], 4);
		if (one & helper)
		{
			MWMAC_ECC_Point_Add2(Core_CMD_ptr);
		}
		Core_CMD_ptr->A[(rw_prec/32)] = Core_CMD_ptr->A[(rw_prec/32)] << 1;
	}
	
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)] = ioread32(MWMAC_RAM_ptr+0x280+i);
		Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+16] = ioread32(MWMAC_RAM_ptr+0x290+i);
		Core_CMD_ptr->E[(Core_CMD_ptr->prec/32-1-i)+32] = ioread32(MWMAC_RAM_ptr+0x2A0+i);
	}
}

static void MWMAC_ECC_Point_Add2(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 help = 0;

	// Write Parameters to E Register Memory
	

	//U1 = X1 * Z2^2
	Core_CMD_ptr->src = MWMAC_RAM_E6;
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E6;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	MWMAC_MontMult(Core_CMD_ptr);

	//U2 = X2 * Z1^2
	Core_CMD_ptr->src = MWMAC_RAM_E3;
	Core_CMD_ptr->dest = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E3;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E4;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	//S1 = Y1 * Z2^3
	Core_CMD_ptr->src = MWMAC_RAM_E6;
	Core_CMD_ptr->dest = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E6;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	MWMAC_MontMult(Core_CMD_ptr);
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E2;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	MWMAC_MontMult(Core_CMD_ptr);

	//S2 = Y2 * Z1^3
	Core_CMD_ptr->src = MWMAC_RAM_E3;
	Core_CMD_ptr->dest = MWMAC_RAM_B4;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E3;
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	MWMAC_MontMult(Core_CMD_ptr);
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E5;
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	MWMAC_MontMult(Core_CMD_ptr);

	//H = U2 - U1
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_TS2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_TC2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS2;
	MWMAC_ModSub(Core_CMD_ptr);

	//CHECK IF U1 == U2
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		if (0 == ioread32(MWMAC_RAM_ptr+0x43+i*0x4))
		{
			help = 1;
		}
		else
		{
			help = 0;
			break;
		}
	}
	if (help == 1) {
		//Point Double
				MWMAC_ECC_Point_Double2(Core_CMD_ptr);
				return;
	}
	
	//HH = H^2
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	//HHH = H * HH

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	//r = S2 - S1

	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_TC4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_TS4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS4;
	MWMAC_ModSub(Core_CMD_ptr);

	//V = U1 * HH

	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	//X3 = r^2 - HHH - 2 * V

	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_A3;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_A3;
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_B5;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B5;
	Core_CMD_ptr->dest = MWMAC_RAM_X2;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_TS4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_TC4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS4;
	MWMAC_ModSub(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_TS4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B5;
	Core_CMD_ptr->dest = MWMAC_RAM_TC4;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS4;
	MWMAC_ModSub(Core_CMD_ptr);

	// Y3 = r * (V-X3) - S1*HHH

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_A4;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_A4;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_TS1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_TC1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS1;
	MWMAC_ModSub(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_A3;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_TS1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_TC1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS1;
	MWMAC_ModSub(Core_CMD_ptr);

	//Z3 = Z1*Z2*H

	Core_CMD_ptr->src = MWMAC_RAM_E3;
	Core_CMD_ptr->dest = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->dest = MWMAC_RAM_E6;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	//Copy back to E
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->dest = MWMAC_RAM_E1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_E2;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_E3;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

}

static void MWMAC_ECC_Point_Double2(Core_CMD_t *Core_CMD_ptr)
{	
	u32 i;
	
	// S = 4 * X * Y^2
	Core_CMD_ptr->src = MWMAC_RAM_E2;
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = COPYV2H;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->dest = MWMAC_RAM_E2;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->dest = MWMAC_RAM_E1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->dest = MWMAC_RAM_X4;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);
	
	// M = 3 * X^2 + a * Z^4
	Core_CMD_ptr->src = MWMAC_RAM_E1;
	Core_CMD_ptr->dest = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = COPYV2H;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_E1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X3;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_E3;
	Core_CMD_ptr->dest = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = COPYV2H;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_Copy(Core_CMD_ptr);

	for (i = 0; i < 3; i++) {
		Core_CMD_ptr->dest = MWMAC_RAM_E3;
		Core_CMD_ptr->src = MWMAC_RAM_B3;
		Core_CMD_ptr->cmd = MONTMULT;
		//Core_CMD_ptr->prec = rw_prec;
		MWMAC_MontMult(Core_CMD_ptr);
	}

	Core_CMD_ptr->dest = MWMAC_RAM_X6;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = MONTMULT;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_TS2;
	Core_CMD_ptr->cmd = COPYH2H;
	//Core_CMD_ptr->prec = rw_prec;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_TC2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS2;
	MWMAC_ModAdd(Core_CMD_ptr);

	//X' M^2 - 2S
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X2;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_TS1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_TC1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS1;
	MWMAC_ModSub(Core_CMD_ptr);

	//Y' = M * (S - X') - 8 * Y^4
	Core_CMD_ptr->src = MWMAC_RAM_A1;
	Core_CMD_ptr->dest = MWMAC_RAM_TS2;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_TC2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS2;
	MWMAC_ModSub(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_E2;
	Core_CMD_ptr->dest = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	for (i = 0; i < 3; i++) {
		Core_CMD_ptr->dest = MWMAC_RAM_E2;
		Core_CMD_ptr->src = MWMAC_RAM_B3;
		Core_CMD_ptr->cmd = MONTMULT;
		//Core_CMD_ptr->prec = rw_prec;
		MWMAC_MontMult(Core_CMD_ptr);
	}

	Core_CMD_ptr->dest = MWMAC_RAM_X5;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_TS2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_TC2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_TS2;
	MWMAC_ModSub(Core_CMD_ptr);

	// Z' 2 * Y * Z
	Core_CMD_ptr->src = MWMAC_RAM_E2;
	Core_CMD_ptr->dest = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_X2;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_E3;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = MONTMULT;
	MWMAC_MontMult(Core_CMD_ptr);
	
	//Copy back to E
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_E1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_E2;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_E3;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);
}

static void MWMAC_DeJacobian(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x42+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x82+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0xC2+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x102+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x142+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x182+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x1C2+i*0x4));
	}

	// Write Parameter e to E Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->E[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
	}

	// Write Parameter b to X Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->X[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x320+i));
		iowrite32(Core_CMD_ptr->X[(rw_prec/32-1-i)+16], (MWMAC_RAM_ptr+0x310+i));
		iowrite32(Core_CMD_ptr->X[(rw_prec/32-1-i)+32], (MWMAC_RAM_ptr+0x300+i));
	}

	MWMAC_MontExp(Core_CMD_ptr);
	Core_CMD_ptr->cmd = COPYH2V;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = COPYH2H;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_B2;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = COPYH2V;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	MWMAC_Copy(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_X3;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_X2;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->cmd = MONTMULT1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	MWMAC_MontMult1(Core_CMD_ptr);
	
	Core_CMD_ptr->cmd = MONTMULT1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult1(Core_CMD_ptr);

	//Write Point to E
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		Core_CMD_ptr->X[Core_CMD_ptr->prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	}
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		Core_CMD_ptr->X[(Core_CMD_ptr->prec/32-1-i)+16] = ioread32(MWMAC_RAM_ptr+0x43+i*0x4);
	}
	
}

static void MWMAC_Invert_Elem(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
		/*iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x42+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x82+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0xC2+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x102+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x142+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x182+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x1C2+i*0x4));*/
	}

	// Write Parameter e to E1 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->E[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x280+i));
	}

	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->src = MWMAC_RAM_P1;
	Core_CMD_ptr->cmd = MONTR2;
	MWMAC_MontR(Core_CMD_ptr);
	
	// Write element to A1 Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->X[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x200+i));		
	}
	
	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->cmd = COPYH2V;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	MWMAC_Copy(Core_CMD_ptr);
	
	MWMAC_MontExp(Core_CMD_ptr);
	
	Core_CMD_ptr->cmd = MONTMULT1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	MWMAC_MontMult(Core_CMD_ptr);

	//Write  to B
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		Core_CMD_ptr->B[Core_CMD_ptr->prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	}

}

static void MWMAC_Verification(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	Clear_MWMAC_RAM();

	// Write Parameter n to P Register Memory
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x42+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x82+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0xC2+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x102+i*0x4));
	}

	// Write Parameter to B Register Memory
	for(i=0; i<rw_prec/32; i++){
		//iowrite32(Core_CMD_ptr->B[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x43+i*0x4));
		iowrite32(Core_CMD_ptr->B[(rw_prec/32-1-i)+16], (MWMAC_RAM_ptr+0x300+i));
		iowrite32(Core_CMD_ptr->B[(rw_prec/32-1-i)+32], (MWMAC_RAM_ptr+0x43+i*0x4));
		iowrite32(Core_CMD_ptr->B[(rw_prec/32-1-i)+48], (MWMAC_RAM_ptr+0x83+i*0x4));
	}
	//Write Exponent to E
	for(i=0; i<rw_prec/32; i++){
		iowrite32(Core_CMD_ptr->E[(rw_prec/32-1-i)], (MWMAC_RAM_ptr+0x280+i));
	}



	Core_CMD_ptr->src = MWMAC_RAM_P1;
	Core_CMD_ptr->dest = MWMAC_RAM_B1;
	Core_CMD_ptr->cmd = MONTR2;
	MWMAC_MontR(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	MWMAC_MontExp(Core_CMD_ptr);

	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);

	//u2
	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	MWMAC_MontMult(Core_CMD_ptr);
	//u1
	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT1;
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	MWMAC_MontMult(Core_CMD_ptr);

	Core_CMD_ptr->cmd = MONTMULT1;
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	MWMAC_MontMult(Core_CMD_ptr);

	//Write  to B
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		Core_CMD_ptr->B[Core_CMD_ptr->prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x43+i*0x4);
		Core_CMD_ptr->B[(Core_CMD_ptr->prec/32-1-i)+16] = ioread32(MWMAC_RAM_ptr+0x83+i*0x4);
	}

}

static void MWMAC_Signcryption(Core_CMD_t *Core_CMD_ptr)
{
	u32 i;
	u32 mwmac_cmd_prec = 0;
	u32 mwmac_f_sel = 1;
	u32 rw_prec = 0;

	if(Core_CMD_ptr->f_sel == 0) {
		mwmac_f_sel = 0;
		for(i=0; i<16; i++){
			if(BINARY_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = BINARY_PRECISIONS[i][1];
				if(Core_CMD_ptr->prec % 32) {
					rw_prec = (Core_CMD_ptr->prec/32 + 1) * 32;
				} else {
					rw_prec = Core_CMD_ptr->prec;
				}
			}
		}
	}
	else {
		mwmac_f_sel = 1;
		for(i=0; i<13; i++){
			if(PRIME_PRECISIONS[i][0]==Core_CMD_ptr->prec){
				mwmac_cmd_prec = PRIME_PRECISIONS[i][1];
				rw_prec = Core_CMD_ptr->prec;
			}
		}
	}

	Clear_MWMAC_RAM();

	for(i=0; i<rw_prec/32; i++){
		//Primes
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x2+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x42+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x82+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0xC2+i*0x4));
		iowrite32(Core_CMD_ptr->P[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x102+i*0x4));
		//Values
		iowrite32(Core_CMD_ptr->B[rw_prec/32-1-i], (MWMAC_RAM_ptr+0x3+i*0x4));
		iowrite32(Core_CMD_ptr->B[(rw_prec/32-1-i)+16], (MWMAC_RAM_ptr+0x43+i*0x4));
		iowrite32(Core_CMD_ptr->B[(rw_prec/32-1-i)+32], (MWMAC_RAM_ptr+0x83+i*0x4));
		iowrite32(Core_CMD_ptr->B[(rw_prec/32-1-i)+48], (MWMAC_RAM_ptr+0x280+i));
	}
	Core_CMD_ptr->dest = MWMAC_RAM_B4;
	Core_CMD_ptr->src = MWMAC_RAM_P4;
	Core_CMD_ptr->cmd = MONTR2;
	MWMAC_MontR(Core_CMD_ptr);

	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	Core_CMD_ptr->src = MWMAC_RAM_B4;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_A1;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_TS1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->src = MWMAC_RAM_B2;
	Core_CMD_ptr->dest = MWMAC_RAM_TC1;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->src = MWMAC_RAM_TS1;
	Core_CMD_ptr->cmd = MODADD;
	MWMAC_ModAdd(Core_CMD_ptr);
	
	Core_CMD_ptr->src = MWMAC_RAM_B3;
	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_X1;
	Core_CMD_ptr->cmd = COPYH2V;
	MWMAC_Copy(Core_CMD_ptr);
	
	MWMAC_MontExp(Core_CMD_ptr);
	
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_B2;
	Core_CMD_ptr->cmd = COPYH2H;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->cmd = MONTMULT;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	Core_CMD_ptr->dest = MWMAC_RAM_A2;
	MWMAC_MontMult(Core_CMD_ptr);
	
	Core_CMD_ptr->src = MWMAC_RAM_A2;
	Core_CMD_ptr->dest = MWMAC_RAM_B3;
	Core_CMD_ptr->cmd = COPYV2H;
	MWMAC_Copy(Core_CMD_ptr);
	
	Core_CMD_ptr->cmd = MONTMULT1;
	Core_CMD_ptr->src = MWMAC_RAM_B1;
	MWMAC_MontMult(Core_CMD_ptr);
	
	for(i=0; i<Core_CMD_ptr->prec/32; i++){
		Core_CMD_ptr->B[Core_CMD_ptr->prec/32-1-i] = ioread32(MWMAC_RAM_ptr+0x3+i*0x4);
	}
}

module_init( cryptocore_init );
module_exit( cryptocore_exit );

MODULE_AUTHOR("MAI - Selected Topics of Embedded Software Development II");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("The driver for the FPGA-based CryptoCore");
MODULE_SUPPORTED_DEVICE("CryptoCore");
