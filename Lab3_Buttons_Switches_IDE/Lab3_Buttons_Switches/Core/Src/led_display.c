#include "led_display.h"
#include "main.h"

#define BLINK_INTERVAL_MS 250U

static const uint8_t segment_codes[10] = {
    0x40U, 0x79U, 0x24U, 0x30U, 0x19U,
    0x12U, 0x02U, 0x78U, 0x00U, 0x10U
};

static const uint16_t road1_segment_pins[7] = {
    a_Pin, b_Pin, c_Pin, d_Pin,
    e_Pin, f_Pin, g_Pin
};

static const uint16_t road2_segment_pins[7] = {
    h_Pin, i_Pin, j_Pin, k_Pin,
    l_Pin, m_Pin, n_Pin
};

static volatile uint8_t displayed_mode = 1U;
static volatile uint8_t displayed_edit = 0U;

/* Used by the display ISR after initialization. */
static uint8_t scan_position = 0U;
static uint8_t frame_mode = 1U;
static uint8_t frame_edit = 0U;

/* Used only by main. */
static uint8_t active_mode = 1U;
static uint8_t blink_on = 0U;
static uint32_t blink_previous_ms = 0U;

void traffic_leds_off(void)
{
    HAL_GPIO_WritePin(
        GPIOA,
        LED_RED_1_Pin |
        LED_YELLOW_1_Pin |
        LED_GREEN_1_Pin |
        LED_RED_2_Pin |
        LED_YELLOW_2_Pin |
        LED_GREEN_2_Pin,
        GPIO_PIN_SET
    );
}

void display_blank(void)
{
    HAL_GPIO_WritePin(
        GPIOA,
        EN0_Pin | EN1_Pin | EN2_Pin,
        GPIO_PIN_SET
    );

    HAL_GPIO_WritePin(
        EN3_GPIO_Port,
        EN3_Pin,
        GPIO_PIN_SET
    );
}

static void write_digit(const uint16_t pins[7], uint8_t digit)
{
    uint8_t pattern = 0x7FU;

    if (digit < 10U) {
        pattern = segment_codes[digit];
    }

    for (uint8_t segment = 0U; segment < 7U; segment++) {
        GPIO_PinState level =
            ((pattern & (1U << segment)) != 0U)
            ? GPIO_PIN_SET
            : GPIO_PIN_RESET;

        HAL_GPIO_WritePin(GPIOB, pins[segment], level);
    }
}

static void write_selected_color(uint8_t on)
{
    uint16_t pins = 0U;

    switch (active_mode) {
        case 2U:
            pins = LED_RED_1_Pin | LED_RED_2_Pin;
            break;

        case 3U:
            pins = LED_YELLOW_1_Pin | LED_YELLOW_2_Pin;
            break;

        case 4U:
            pins = LED_GREEN_1_Pin | LED_GREEN_2_Pin;
            break;

        default:
            return;
    }

    HAL_GPIO_WritePin(
        GPIOA,
        pins,
        (on != 0U) ? GPIO_PIN_RESET : GPIO_PIN_SET
    );
}

void led_display_init(void)
{
    display_blank();
    traffic_leds_off();

    HAL_GPIO_WritePin(
        GPIOB,
        a_Pin | b_Pin | c_Pin | d_Pin |
        e_Pin | f_Pin | g_Pin |
        h_Pin | i_Pin | j_Pin | k_Pin |
        l_Pin | m_Pin | n_Pin,
        GPIO_PIN_SET
    );

    displayed_mode = 1U;
    displayed_edit = 0U;

    scan_position = 0U;
    frame_mode = 1U;
    frame_edit = 0U;

    active_mode = 1U;
    blink_on = 0U;
    blink_previous_ms = HAL_GetTick();
}

void led_display_show_mode(uint8_t mode)
{
    if ((mode < 1U) || (mode > 4U)) {
        return;
    }

    active_mode = mode;

    uint32_t saved_primask = __get_PRIMASK();
    __disable_irq();

    displayed_mode = mode;
    displayed_edit = 0U;

    __set_PRIMASK(saved_primask);

    traffic_leds_off();
    blink_previous_ms = HAL_GetTick();

    if (mode == 1U) {
        blink_on = 0U;

        /* Temporary Mode 1 indicator; traffic FSM is not implemented yet. */
        HAL_GPIO_WritePin(
            LED_RED_1_GPIO_Port,
            LED_RED_1_Pin,
            GPIO_PIN_RESET
        );
    } else {
        blink_on = 1U;
        write_selected_color(blink_on);
    }
}

void led_display_set_edit_value(uint8_t value)
{
    uint32_t saved_primask = __get_PRIMASK();
    __disable_irq();

    displayed_edit =
        (value >= 1U && value <= 99U) ? value : 0U;

    __set_PRIMASK(saved_primask);
}

void led_display_scan(void)
{
    display_blank();

    if (scan_position == 0U) {
        frame_mode = displayed_mode;
        frame_edit = displayed_edit;

        if ((frame_mode < 1U) || (frame_mode > 4U)) {
            frame_mode = 1U;
        }

        /* Show the editing value in modes 2, 3 and 4. */
        if ((frame_mode < 2U) || (frame_mode > 4U) ||
            (frame_edit > 99U)) {
            frame_edit = 0U;
        }

        write_digit(road2_segment_pins, 0U);

        if (frame_edit != 0U) {
            write_digit(road1_segment_pins, frame_edit / 10U);

            HAL_GPIO_WritePin(
                EN0_GPIO_Port,
                EN0_Pin,
                GPIO_PIN_RESET
            );
        }

        HAL_GPIO_WritePin(
            EN2_GPIO_Port,
            EN2_Pin,
            GPIO_PIN_RESET
        );

        scan_position = 1U;
    } else {
        write_digit(road2_segment_pins, frame_mode);

        if (frame_edit != 0U) {
            write_digit(road1_segment_pins, frame_edit % 10U);

            HAL_GPIO_WritePin(
                EN1_GPIO_Port,
                EN1_Pin,
                GPIO_PIN_RESET
            );
        }

        HAL_GPIO_WritePin(
            EN3_GPIO_Port,
            EN3_Pin,
            GPIO_PIN_RESET
        );

        scan_position = 0U;
    }
}

void led_display_run(void)
{
    if ((active_mode < 2U) || (active_mode > 4U)) {
        return;
    }

    uint32_t now_ms = HAL_GetTick();

    uint32_t elapsed_ms =
        (uint32_t)(now_ms - blink_previous_ms);

    if (elapsed_ms >= BLINK_INTERVAL_MS) {
        uint32_t intervals =
            elapsed_ms / BLINK_INTERVAL_MS;

        blink_previous_ms +=
            intervals * BLINK_INTERVAL_MS;

        if ((intervals & 1U) != 0U) {
            blink_on ^= 1U;
            write_selected_color(blink_on);
        }
    }
}
