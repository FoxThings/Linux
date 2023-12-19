#include <linux/init.h>
#include <linux/module.h>
#include <linux/keyboard.h>
#include <linux/semaphore.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/timer.h>

MODULE_LICENSE("MIT");
MODULE_AUTHOR("Zvezdin Nikita");
MODULE_DESCRIPTION("This module collects data about keyboard");
MODULE_VERSION("1.0");


// -------------------------------> Globals <-------------------------------
static int irq = 1;
#define LOG_PERIOD 60000

static struct semaphore sem;
static struct timer_list my_timer;
static unsigned int char_count = 0;
// -------------------------------------------------------------------------



// ------------------------------> Handlers <------------------------------
static irqreturn_t irq_handler(int irq, void *dev)
{
    down(&sem);
    char_count++;
    up(&sem);
    return IRQ_HANDLED;
}

static void timer_callback(struct timer_list *t)
{
    pr_info("%u characters were typed last minute\n", char_count);
    char_count = 0;
    mod_timer(t, jiffies + msecs_to_jiffies(LOG_PERIOD));
}
// ------------------------------------------------------------------------


// --------------------------> Module life events <--------------------------
static int __init inputstat_init(void)
{
    free_irq(irq, &irq);
    if (request_irq(irq, irq_handler, IRQF_SHARED, "inputstat", &irq))
    {
        pr_err("Failed to register IRQ handler\n");
        return -EIO;
    }

    sema_init(&sem, 1);
    timer_setup(&my_timer, timer_callback, 0);
    mod_timer(&my_timer, jiffies + msecs_to_jiffies(LOG_PERIOD));

    pr_info("Inputstat module loaded\n");
    return 0;
}

static void __exit inputstat_exit(void)
{
    synchronize_irq(irq);
    free_irq(irq, &irq);

    del_timer(&my_timer);

    pr_info("Inputstat module unloaded\n");
}

module_init(inputstat_init);
module_exit(inputstat_exit);
// --------------------------------------------------------------------------
