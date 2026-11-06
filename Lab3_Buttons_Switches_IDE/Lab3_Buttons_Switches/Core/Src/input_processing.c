#include "input_processing.h"
#include "input_reading.h"
#include "led_display.h"
#include "main.h"

#define REPEAT_INTERVAL_MS 500U

typedef enum {
    BUTTON_RELEASED = 0,
    BUTTON_PRESSED,
    BUTTON_HELD
} ButtonState;

static ButtonState button_states[BUTTON_COUNT];

static uint8_t current_mode = 1U;

static uint8_t red_duration = 5U;
static uint8_t amber_duration = 2U;
static uint8_t green_duration = 3U;

static uint8_t edit_value = 0U;

static uint8_t inc_blocked = 0U;
static uint8_t inc_armed = 0U;
static uint8_t repeat_active = 0U;

static uint32_t repeat_previous_ms = 0U;

static void increase_edit_value(void)
{
    edit_value = (edit_value >= 99U)
               ? 1U : (uint8_t)(edit_value + 1U);

    led_display_set_edit_value(edit_value);
}

static void set_edit_value(void)
{
    if ((edit_value < 1U) || (edit_value > 99U)) {
        return;
    }

    switch (current_mode) {
        case 2U:
            if (edit_value <= amber_duration) {
                return;
            }

            red_duration = edit_value;
            green_duration =
                (uint8_t)(red_duration - amber_duration);
            break;

        case 3U:
            if (edit_value >= red_duration) {
                return;
            }

            amber_duration = edit_value;
            green_duration =
                (uint8_t)(red_duration - amber_duration);
            break;

        case 4U: {
            uint16_t new_red =
                (uint16_t)edit_value + (uint16_t)amber_duration;

            if (new_red > 99U) {
                return;
            }

            green_duration = edit_value;
            red_duration = (uint8_t)new_red;
            break;
        }

        default:
            break;
    }
}

static void enter_mode(uint8_t mode, uint8_t inc_is_pressed)
{
    current_mode = mode;

    repeat_active = 0U;
    inc_armed = 0U;
    inc_blocked = inc_is_pressed;

    switch (mode) {
        case 2U:
            edit_value = red_duration;
            break;

        case 3U:
            edit_value = amber_duration;
            break;

        case 4U:
            edit_value = green_duration;
            break;

        default:
            edit_value = 0U;
            break;
    }

    led_display_show_mode(mode);
    led_display_set_edit_value(edit_value);
}

void input_processing_init(void)
{
    for (uint8_t i = 0U; i < BUTTON_COUNT; i++) {
        button_states[i] = BUTTON_RELEASED;
    }

    red_duration = 5U;
    amber_duration = 2U;
    green_duration = 3U;

    repeat_previous_ms = 0U;

    enter_mode(1U, 0U);
}

void fsm_for_input_processing(void)
{
    uint8_t events[BUTTON_COUNT];
    uint8_t pressed[BUTTON_COUNT];
    uint8_t inc_held;

    uint32_t saved_primask = __get_PRIMASK();

    /* Capture one consistent snapshot of all buttons. */
    __disable_irq();

    for (uint8_t i = 0U; i < BUTTON_COUNT; i++) {
        events[i] = button_take_events(i);
        pressed[i] = is_button_pressed(i);
    }

    inc_held = is_button_pressed_1s(1U);

    __set_PRIMASK(saved_primask);

    for (uint8_t i = 0U; i < BUTTON_COUNT; i++) {
        if ((events[i] & BUTTON_EVENT_PRESS) != 0U) {
            button_states[i] = BUTTON_PRESSED;
        }

        if (((events[i] & BUTTON_EVENT_LONG) != 0U) &&
            (button_states[i] == BUTTON_PRESSED)) {
            button_states[i] = BUTTON_HELD;
        }

        if ((events[i] & BUTTON_EVENT_RELEASE) != 0U) {
            button_states[i] = BUTTON_RELEASED;
        }
    }

    /* MODE has the highest priority. */
    if ((events[0] & BUTTON_EVENT_PRESS) != 0U) {
        uint8_t next_mode = (current_mode >= 4U)
                          ? 1U : (uint8_t)(current_mode + 1U);

        enter_mode(next_mode, pressed[1]);
        return;
    }

    if ((events[1] & BUTTON_EVENT_RELEASE) != 0U) {
        repeat_active = 0U;
        inc_armed = 0U;
        inc_blocked = 0U;
    }

    /* Red, amber and green editing are enabled. */
    if ((current_mode < 2U) || (current_mode > 4U)) {
        return;
    }

    /* SET has priority over INC, even when SET is rejected. */
    if ((events[2] & BUTTON_EVENT_PRESS) != 0U) {
        set_edit_value();

        repeat_active = 0U;
        inc_armed = 0U;
        inc_blocked = pressed[1];

        return;
    }

    if (inc_blocked != 0U) {
        return;
    }

    if ((events[1] & BUTTON_EVENT_PRESS) != 0U) {
        increase_edit_value();

        inc_armed = pressed[1];
        repeat_active = 0U;
    }

    if (pressed[1] == 0U) {
        inc_armed = 0U;
        repeat_active = 0U;
        return;
    }

    if ((((events[1] & BUTTON_EVENT_LONG) != 0U) ||
         (inc_held != 0U)) &&
        (inc_armed != 0U) && (repeat_active == 0U)) {
        increase_edit_value();

        repeat_active = 1U;
        repeat_previous_ms = HAL_GetTick();

        return;
    }

    if (repeat_active != 0U) {
        uint32_t now_ms = HAL_GetTick();

        if ((uint32_t)(now_ms - repeat_previous_ms) >=
            REPEAT_INTERVAL_MS) {
            increase_edit_value();

            /* Avoid a burst of increments after a delayed main loop. */
            repeat_previous_ms = now_ms;
        }
    }
}

uint8_t app_get_mode(void)
{
    return current_mode;
}

uint8_t app_get_edit_value(void)
{
    return edit_value;
}

uint8_t app_get_red_duration(void)
{
    return red_duration;
}

uint8_t app_get_amber_duration(void)
{
    return amber_duration;
}

uint8_t app_get_green_duration(void)
{
    return green_duration;
}
