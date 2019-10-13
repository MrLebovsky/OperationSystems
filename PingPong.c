#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/reboot.h>
#include <linux/unistd.h>
#include <linux/time.h>
#include <linux/random.h>
#include <linux/syscalls.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/rbtree.h>
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/jiffies.h>
#include <linux/vmalloc.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/sched.h>

struct Channel
{
	spinlock_t read_lock;
	spinlock_t write_lock;
	char buffer[256];
};

struct Channel* toPing;
struct Channel* toPong;

struct task_struct* ping_task;
struct task_struct* pong_task;
struct task_struct* starter_task;

void put(struct Channel* channel, char* src, size_t len)
{

	do {
 		schedule();
	} while (spin_trylock(&(channel->write_lock))); 
	
	memcpy(channel->buffer, src, len);
	spin_unlock(&(channel->read_lock));
}

void get(struct Channel* channel, char* dst)
{
	int len;
	do {
 		schedule();
	} while (spin_trylock(&(channel->read_lock))); 
	len = channel->buffer[0];
	memcpy(dst, channel->buffer, len);
	spin_unlock(&(channel->write_lock));
}

static int ping(void* data)
{
	char buffer[256];
	printk("Ping Started!\n");
	
		while (true)
		{
			//printk("Ping: send ping to Pong!\n");
			put(toPong, "ping", 4);
		
			get(toPing, buffer);
			if (strcmp(buffer, "pong") == 0) {
				printk("Ping: got the pong!\n");
				printk("Ping Stoped!\n");
				return 0;
			}
			schedule();
		}
}

static int pong(void* data)
{
	char buffer[256];
	printk("Pong Started!\n");
	while (true)
	{
		//printk("Pong: waiting for ping!\n");
		get(toPong, buffer);
		//printk("Pong: got something!\n");
		if (strcmp(buffer, "ping") == 0) {
			printk("Pong: I'm got the ping! Send the pong!\n");

			put(toPing, "pong", 4);

			printk("Pong Stoped!\n");
			return 0;
		}
		schedule();
	}
}

static int __init lab3_init(void) {
	
	toPing =  kmalloc(sizeof(struct Channel), GFP_KERNEL);
	toPong =  kmalloc(sizeof(struct Channel), GFP_KERNEL);
	
	spin_lock_init(&(toPong->read_lock));
	spin_lock_init(&(toPong->write_lock));
	spin_lock(&(toPong->read_lock));

	spin_lock_init(&(toPing->read_lock));
	spin_lock_init(&(toPing->write_lock));
	spin_lock(&(toPing->read_lock));
	
	ping_task = kthread_run(&ping, (void *)NULL, "ping Thread"); 
	pong_task = kthread_run(&pong, (void *)NULL, "pong Thread");
	
	
	return 0;
}

static void __exit lab3_exit(void) {
	printk(KERN_INFO "Module lab3 exit!\n");
}

module_init(lab3_init);
module_exit(lab3_exit);
