#ifndef INC_INPUT_READING_H_
#define INC_INPUT_READING_H_

#include <stdint.h>

#define BUTTON_COUNT         3U

#define BUTTON_EVENT_PRESS   0x01U
#define BUTTON_EVENT_LONG    0x02U
#define BUTTON_EVENT_RELEASE 0x04U

void input_reading_init(void);
void button_reading(void);

uint8_t is_button_pressed(uint8_t index);
uint8_t is_button_pressed_1s(uint8_t index);
uint8_t button_take_events(uint8_t index);

#endif /* INC_INPUT_READING_H_ */
