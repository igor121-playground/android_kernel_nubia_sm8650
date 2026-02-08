// game_sw_sysfs.c
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/gpio/consumer.h>
#include <linux/interrupt.h>
#include <linux/of.h>
#include <linux/sysfs.h>
#include <linux/input.h>

struct gpio_switch_data {
    struct gpio_desc *gpio_on;
    struct gpio_desc *gpio_off;
    int irq_on;
    int irq_off;
    int state; 
    struct device *dev;
    struct input_dev *input;
    unsigned int code_on;  
    unsigned int code_off;  
    unsigned int debounce_interval;
    bool wakeup;
};

static irqreturn_t gpio_switch_on_isr(int irq, void *dev_id)
{
    struct gpio_switch_data *data = dev_id;
    
    data->state = 1;
    
    input_report_key(data->input, data->code_on, 1);
    input_sync(data->input);
    input_report_key(data->input, data->code_on, 0);
    input_sync(data->input);
    
    sysfs_notify(&data->dev->kobj, NULL, "state");
    
    dev_dbg(data->dev, "Switch up (code=%d)\n", data->code_on);
    
    return IRQ_HANDLED;
}

static irqreturn_t gpio_switch_off_isr(int irq, void *dev_id)
{
    struct gpio_switch_data *data = dev_id;
    
    /* Update state */
    data->state = 0;
    
    /* Report key event */
    input_report_key(data->input, data->code_off, 1);
    input_sync(data->input);
    input_report_key(data->input, data->code_off, 0);
    input_sync(data->input);
    
    /* Notify sysfs */
    sysfs_notify(&data->dev->kobj, NULL, "state");
    
    dev_dbg(data->dev, "Switch down (code=%d)\n", data->code_off);
    
    return IRQ_HANDLED;
}

static ssize_t state_show(struct device *dev,
                          struct device_attribute *attr, char *buf)
{
    struct gpio_switch_data *data = dev_get_drvdata(dev);
    return sprintf(buf, "%d\n", data->state);
}

static DEVICE_ATTR_RO(state);

static struct attribute *gpio_switch_attrs[] = {
    &dev_attr_state.attr,
    NULL,
};
ATTRIBUTE_GROUPS(gpio_switch);

static int gpio_switch_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    struct gpio_switch_data *data;
    int ret;
    u32 code;

    data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
    if (!data)
        return -ENOMEM;

    data->dev = dev;
    platform_set_drvdata(pdev, data);

    data->gpio_on = devm_gpiod_get(dev, "on", GPIOD_IN);
    if (IS_ERR(data->gpio_on)) {
        dev_err(dev, "Failed to get ON GPIO: %ld\n", PTR_ERR(data->gpio_on));
        return PTR_ERR(data->gpio_on);
    }

    data->gpio_off = devm_gpiod_get(dev, "off", GPIOD_IN);
    if (IS_ERR(data->gpio_off)) {
        dev_err(dev, "Failed to get OFF GPIO: %ld\n", PTR_ERR(data->gpio_off));
        return PTR_ERR(data->gpio_off);
    }

    /* Get key codes from DT (optional) */
    if (of_property_read_u32(dev->of_node, "linux,code-on", &code))
        data->code_on = KEY_GREEN;  // default
    else
        data->code_on = code;

    if (of_property_read_u32(dev->of_node, "linux,code-off", &code))
        data->code_off = KEY_RED;  // default
    else
        data->code_off = code;

    if (of_property_read_u32(dev->of_node, "debounce-interval", &data->debounce_interval))
        data->debounce_interval = 30;  // default 30ms

    data->wakeup = of_property_read_bool(dev->of_node, "wakeup-source") ||
                   of_property_read_bool(dev->of_node, "gpio-key,wakeup");

    data->input = devm_input_allocate_device(dev);
    if (!data->input) {
        dev_err(dev, "Failed to allocate input device\n");
        return -ENOMEM;
    }

    data->input->name = "game_switch";
    data->input->phys = "game_switch/input0";
    data->input->id.bustype = BUS_HOST;
    data->input->id.vendor = 0x0001;
    data->input->id.product = 0x0001;
    data->input->id.version = 0x0100;

    input_set_capability(data->input, EV_KEY, data->code_on);
    input_set_capability(data->input, EV_KEY, data->code_off);

    ret = input_register_device(data->input);
    if (ret) {
        dev_err(dev, "Failed to register input device: %d\n", ret);
        return ret;
    }


    data->irq_on = gpiod_to_irq(data->gpio_on);
    if (data->irq_on < 0) {
        dev_err(dev, "Failed to get ON IRQ: %d\n", data->irq_on);
        return data->irq_on;
    }

    data->irq_off = gpiod_to_irq(data->gpio_off);
    if (data->irq_off < 0) {
        dev_err(dev, "Failed to get OFF IRQ: %d\n", data->irq_off);
        return data->irq_off;
    }

    /* Set debounce if supported */
    gpiod_set_debounce(data->gpio_on, data->debounce_interval * 1000);
    gpiod_set_debounce(data->gpio_off, data->debounce_interval * 1000);

    /* Initialize state based on current GPIO levels */
    if (gpiod_get_value(data->gpio_on))
        data->state = 1;
    else if (gpiod_get_value(data->gpio_off))
        data->state = 0;
    else
        data->state = 0;  // default to off

    /* Request IRQs */
    ret = devm_request_threaded_irq(dev, data->irq_on, NULL,
                                     gpio_switch_on_isr,
                                     IRQF_TRIGGER_RISING | IRQF_ONESHOT,
                                     "game_sw_on", data);
    if (ret) {
        dev_err(dev, "Failed to request ON IRQ %d: %d\n", data->irq_on, ret);
        return ret;
    }

    ret = devm_request_threaded_irq(dev, data->irq_off, NULL,
                                     gpio_switch_off_isr,
                                     IRQF_TRIGGER_RISING | IRQF_ONESHOT,
                                     "game_sw_off", data);
    if (ret) {
        dev_err(dev, "Failed to request OFF IRQ %d: %d\n", data->irq_off, ret);
        return ret;
    }

    if (data->wakeup) {
        device_init_wakeup(dev, true);
        enable_irq_wake(data->irq_on);
        enable_irq_wake(data->irq_off);
    }

    dev_info(dev, "Game switch registered (ON=KEY_%d, OFF=KEY_%d), initial state: %d\n",
             data->code_on, data->code_off, data->state);

    return 0;
}

static int gpio_switch_remove(struct platform_device *pdev)
{
    struct gpio_switch_data *data = platform_get_drvdata(pdev);

    if (data->wakeup) {
        disable_irq_wake(data->irq_on);
        disable_irq_wake(data->irq_off);
        device_init_wakeup(data->dev, false);
    }

    return 0;
}

#ifdef CONFIG_PM_SLEEP
static int gpio_switch_suspend(struct device *dev)
{
    struct gpio_switch_data *data = dev_get_drvdata(dev);

    if (device_may_wakeup(dev)) {
        enable_irq_wake(data->irq_on);
        enable_irq_wake(data->irq_off);
    }

    return 0;
}

static int gpio_switch_resume(struct device *dev)
{
    struct gpio_switch_data *data = dev_get_drvdata(dev);

    if (device_may_wakeup(dev)) {
        disable_irq_wake(data->irq_on);
        disable_irq_wake(data->irq_off);
    }

    return 0;
}
#endif

static SIMPLE_DEV_PM_OPS(gpio_switch_pm_ops, gpio_switch_suspend, gpio_switch_resume);

static const struct of_device_id gpio_switch_of_match[] = {
    { .compatible = "nubia_game_sw", },
    { },
};
MODULE_DEVICE_TABLE(of, gpio_switch_of_match);

static struct platform_driver nubia_game_sw = {
    .probe = gpio_switch_probe,
    .remove = gpio_switch_remove,
    .driver = {
        .name = "nubia_game_sw",
        .of_match_table = gpio_switch_of_match,
        .dev_groups = gpio_switch_groups,
        .pm = &gpio_switch_pm_ops,
    },
};

module_platform_driver(nubia_game_sw);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("game sw driver");
MODULE_AUTHOR("Superuser1958");
