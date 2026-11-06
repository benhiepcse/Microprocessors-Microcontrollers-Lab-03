#ifndef INC_LED_DISPLAY_H_
#define INC_LED_DISPLAY_H_

#include <stdint.h>

void led_display_init(void);
void traffic_leds_off(void);
void display_blank(void);
void led_display_show_mode(uint8_t mode);
void led_display_scan(void);
void led_display_run(void);
void led_display_set_edit_value(uint8_t value);

#endif /* INC_LED_DISPLAY_H_ */
