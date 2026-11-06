#ifndef INC_INPUT_PROCESSING_H_
#define INC_INPUT_PROCESSING_H_

#include <stdint.h>

void input_processing_init(void);
void fsm_for_input_processing(void);

uint8_t app_get_mode(void);
uint8_t app_get_edit_value(void);
uint8_t app_get_red_duration(void);
uint8_t app_get_amber_duration(void);
uint8_t app_get_green_duration(void);

#endif
