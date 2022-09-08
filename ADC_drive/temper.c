#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/interrupt.h>
#include <mach/irqs.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/delay.h>

#define ADCCON   0x58000000
#define ADCDAT0  0x5800000c
#define INITSUBMSK
#define INTMSK

static dev_t dev_num = -1;	//设备号
static int major = 253;		//主设备号
static int minor = 0;		//次设备号
static struct cdev temper_cdev;//cdev结构体

static unsigned long *adccon  = NULL;
static unsigned long *adcdat0 = NULL;

static char key = '0';
static wait_queue_head_t adc_wait;

static irqreturn_t do_adc(int irq, void *dev)
{
	printk("irq = %d\n",irq);
	key = 1;
	wake_up_interruptible(&adc_wait);
	return IRQ_HANDLED;
}

static int ADC_init()
{
	*adccon = (1<<14) | (5<<6);         //AINT0
	//*adccon = (1<<14) | (10<<6) | (1<<3);//AINT1
}
static ssize_t temper_read(struct file *file, void __user *buf, loff_t *lof)
{

	int adc = 0;
	*adccon |= 1; //start	
	key = 0;
	wait_event_interruptible_timeout(adc_wait, key, 3*HZ);
	adc = (*adcdat0) & (0x3ff);
	printk("adc = %d\n", adc);
	copy_to_user(buf, &adc, sizeof(adc));
}

static int temper_open(struct inode *inode, struct file *file)
{
	printk("temper open run\n");
	return 0;
}

static int temper_close(struct inode *inode, struct file *file)
{
	printk("temper close run\n");
	return 0;
}

static struct file_operations temper_fops = {
	.read = temper_read,
	.open = temper_open,
	.release = temper_close,
};

static __init int temper_init_2440(void)
{
	int ret = -1;
	dev_num = MKDEV(major, minor);	//制作32位设备号
	ret = register_chrdev_region(dev_num, 4, "button_test");//注册设备号
	if (ret){
		goto ERR0;
	}

	cdev_init(&temper_cdev, &temper_fops);		//初始化cdev结构体（把要执行的操作添加进cdev结构体中）
	ret = cdev_add(&temper_cdev, dev_num, 4);	//添加cdev结构体到内核链表中
	if (ret){
		goto ERR1;
	}

	adccon  = ioremap(ADCCON, 4);//物理地址到虚拟地址映射
	adcdat0 = ioremap(ADCDAT0, 4);

	ADC_init();
	init_waitqueue_head(&adc_wait);
	ret = request_irq(IRQ_ADC, do_adc, IRQF_SHARED, "temper", "temper");
	//ret = request_irq(IRQ_ADCPARENT, do_adc, IRQF_SHARED, "temper", "temper");
	if (ret){
		goto ERR0;
	}

	printk("temper_init_2440\n");
	return 0;

ERR0:
	return -EBUSY;
ERR1:
	unregister_chrdev_region(dev_num,4);	//添加cdev失败 注销设备号
}

static __exit void temper_exit_2440(void)
{
	cdev_del(&temper_cdev);	//从系统链表删除cdev
	unregister_chrdev_region(dev_num,4);	//注销设备号
//	disable_irq(IRQ_ADCPARENT);
//	free_irq(IRQ_ADCPARENT, "temper");
	disable_irq(IRQ_ADC);
	free_irq(IRQ_ADC, "temper");
	printk("temper_exit_2440\n");
}

module_init(temper_init_2440);
module_exit(temper_exit_2440);

MODULE_LICENSE("GPL");

