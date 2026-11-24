#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(blinky, LOG_LEVEL_INF);

#define BLINK_PERIOD_MS 1000

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(blinky_led), gpios);

/* Wait (up to timeout_ms) for a terminal to open the USB serial port,
 * so the first log lines are not lost. */
static void wait_for_terminal(uint32_t timeout_ms)
{
	const struct device *console = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	uint32_t dtr = 0;

	for (uint32_t t = 0; t < timeout_ms && !dtr; t += 100) {
		uart_line_ctrl_get(console, UART_LINE_CTRL_DTR, &dtr);
		k_msleep(100);
	}
}

int main(void)
{
	int ret = usb_enable(NULL);

	if (ret != 0 && ret != -EALREADY) {
		/* No USB console; keep blinking anyway */
	}

	wait_for_terminal(5000);

	if (!gpio_is_ready_dt(&led)) {
		LOG_ERR("GPIO device not ready");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure P0.02 (%d)", ret);
		return 0;
	}

	LOG_INF("Blinky started: LED on P0.02, period %d ms", BLINK_PERIOD_MS);

	bool led_on = false;

	while (1) {
		led_on = !led_on;
		gpio_pin_set_dt(&led, led_on);
		LOG_INF("LED %s", led_on ? "ON" : "OFF");
		k_msleep(BLINK_PERIOD_MS);
	}

	return 0;
}
