#include "input_reading.h"
#include "main.h"

#define BUTTON_SAMPLE_MS 10U
#define BUTTON_LONG_MS   1000U

static GPIO_TypeDef * const button_ports[BUTTON_COUNT] = {
    Button1_GPIO_Port,
    Button2_GPIO_Port,
    Button3_GPIO_Port
};

static const uint16_t button_pins[BUTTON_COUNT] = {
    Button1_Pin,
    Button2_Pin,
    Button3_Pin
};

static GPIO_PinState previous_raw[BUTTON_COUNT];
static GPIO_PinState current_raw[BUTTON_COUNT];

static volatile GPIO_PinState stable_state[BUTTON_COUNT];
static volatile uint8_t pending_events[BUTTON_COUNT];
static volatile uint8_t held_1s[BUTTON_COUNT];

static uint16_t held_ms[BUTTON_COUNT];

void input_reading_init(void)
{
    for (uint8_t i = 0U; i < BUTTON_COUNT; i++)
    {
        previous_raw[i] = GPIO_PIN_SET;
        current_raw[i] = GPIO_PIN_SET;
        stable_state[i] = GPIO_PIN_SET;

        pending_events[i] = 0U;
        held_1s[i] = 0U;
        held_ms[i] = 0U;
    }
}

void button_reading(void)
{
    for (uint8_t i = 0U; i < BUTTON_COUNT; i++)
    {
        GPIO_PinState old_state = stable_state[i];

        previous_raw[i] = current_raw[i];

        current_raw[i] = HAL_GPIO_ReadPin(
            button_ports[i],
            button_pins[i]
        );

        if (previous_raw[i] == current_raw[i])
        {
            stable_state[i] = current_raw[i];
        }

        if (old_state != stable_state[i])
        {
            held_ms[i] = 0U;
            held_1s[i] = 0U;

            if (stable_state[i] == GPIO_PIN_RESET)
            {
                pending_events[i] |= BUTTON_EVENT_PRESS;
            }
            else
            {
                pending_events[i] |= BUTTON_EVENT_RELEASE;
            }
        }
        else if (stable_state[i] == GPIO_PIN_RESET)
        {
            if (held_ms[i] < BUTTON_LONG_MS)
            {
                held_ms[i] += BUTTON_SAMPLE_MS;

                if (held_ms[i] >= BUTTON_LONG_MS)
                {
                    held_ms[i] = BUTTON_LONG_MS;
                    held_1s[i] = 1U;
                    pending_events[i] |= BUTTON_EVENT_LONG;
                }
            }
        }
    }
}

uint8_t is_button_pressed(uint8_t index)
{
    if (index >= BUTTON_COUNT)
    {
        return 0U;
    }

    return (stable_state[index] == GPIO_PIN_RESET) ? 1U : 0U;
}

uint8_t is_button_pressed_1s(uint8_t index)
{
    if (index >= BUTTON_COUNT)
    {
        return 0U;
    }

    return held_1s[index];
}

uint8_t button_take_events(uint8_t index)
{
    if (index >= BUTTON_COUNT)
    {
        return 0U;
    }

    uint32_t saved_primask = __get_PRIMASK();
    __disable_irq();

    uint8_t events = pending_events[index];
    pending_events[index] = 0U;

    __set_PRIMASK(saved_primask);

    return events;
}
