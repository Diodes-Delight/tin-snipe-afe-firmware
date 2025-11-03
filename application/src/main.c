#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>

#if !DT_NODE_EXISTS(DT_PATH(zephyr_user)) || \
	!DT_NODE_HAS_PROP(DT_PATH(zephyr_user), io_channels)
#error "No suitable devicetree overlay specified"
#endif

#define ZEPHYR_USER_NODE DT_PATH(zephyr_user)

#define DT_SPEC_AND_COMMA(node_id, prop, idx) \
	ADC_DT_SPEC_GET_BY_IDX(node_id, idx),

/* Data of ADC io-channels specified in devicetree. */
static const struct adc_dt_spec adc_channels[] = {
	DT_FOREACH_PROP_ELEM(DT_PATH(zephyr_user), io_channels,
			     DT_SPEC_AND_COMMA)
};

const struct gpio_dt_spec relay1_reset = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, relay1_reset_gpios);
const struct gpio_dt_spec relay1_set = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, relay1_set_gpios);
const struct gpio_dt_spec relay2_reset = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, relay2_reset_gpios);
const struct gpio_dt_spec relay2_set = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, relay2_set_gpios);
const struct gpio_dt_spec relay3_reset = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, relay3_reset_gpios);
const struct gpio_dt_spec relay3_set = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, relay3_set_gpios);

const struct gpio_dt_spec en_pos = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, en_pos_gpios);
const struct gpio_dt_spec en_neg = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, en_neg_gpios);
const struct gpio_dt_spec en_ldo = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, en_ldo_gpios);

const struct gpio_dt_spec a10x_inh = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, a10x_inh_gpios);
const struct gpio_dt_spec att_s1 = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, att_s1_gpios);
const struct gpio_dt_spec att_s2 = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, att_s2_gpios);

const struct gpio_dt_spec gs_a = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, gs_a_gpios);
const struct gpio_dt_spec gs_b = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, gs_b_gpios);
const struct gpio_dt_spec gs_inh = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, gs_inh_gpios);

const struct gpio_dt_spec is_a = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, is_a_gpios);
const struct gpio_dt_spec is_b = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, is_b_gpios);
const struct gpio_dt_spec is_inh = GPIO_DT_SPEC_GET(ZEPHYR_USER_NODE, is_inh_gpios);


int main(void)
{
	int err;
	uint32_t count = 0;
	uint16_t buf;
	struct adc_sequence sequence = {
		.buffer = &buf,
		/* buffer size in bytes, not number of samples */
		.buffer_size = sizeof(buf),
	};
	printk("ADC sample application\n");
	/* Configure channels individually prior to sampling. */
	for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
		if (!adc_is_ready_dt(&adc_channels[i])) {
			printk("ADC controller device %s not ready\n", adc_channels[i].dev->name);
			return 0;
		}

		err = adc_channel_setup_dt(&adc_channels[i]);
		if (err < 0) {
			printk("Could not setup channel #%d (%d)\n", i, err);
			return 0;
		}
	}

	// ensure all relay drivers are off
	gpio_pin_configure_dt(&relay1_reset, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&relay1_set, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&relay2_reset, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&relay2_set, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&relay3_reset, GPIO_OUTPUT_INACTIVE);
	gpio_pin_configure_dt(&relay3_set, GPIO_OUTPUT_INACTIVE);

	while (1) {

		printk("ADC reading[%u]:\n", count++);
		for (size_t i = 0U; i < ARRAY_SIZE(adc_channels); i++) {
			int32_t val_mv;

			printk("- %s, channel %d: ",
			       adc_channels[i].dev->name,
			       adc_channels[i].channel_id);

			(void)adc_sequence_init_dt(&adc_channels[i], &sequence);

			err = adc_read_dt(&adc_channels[i], &sequence);
			if (err < 0) {
				printk("Could not read (%d)\n", err);
				continue;
			}

			/*
			 * If using differential mode, the 16 bit value
			 * in the ADC sample buffer should be a signed 2's
			 * complement value.
			 */
			if (adc_channels[i].channel_cfg.differential) {
				val_mv = (int32_t)((int16_t)buf);
			} else {
				val_mv = (int32_t)buf;
			}
			printk("%"PRId32, val_mv);
			err = adc_raw_to_millivolts_dt(&adc_channels[i],
						       &val_mv);
			/* conversion to mV may not be supported, skip if not */
			if (err < 0) {
				printk(" (value in mV not available)\n");
			} else {
				printk(" = %"PRId32" mV\n", val_mv);
			}
		}

		k_sleep(K_MSEC(1000));
	}
	return 0;
}
