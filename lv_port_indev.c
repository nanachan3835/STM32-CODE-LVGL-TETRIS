#include "lv_port_indev.h"
#include "main.h"

/* Callback đọc nút bấm */
static void my_input_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    //  PA0 = LEFT, PA1 = RIGHT
    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0) == GPIO_PIN_RESET) {
        data->state = LV_INDEV_STATE_PR;
        data->key = LV_KEY_LEFT;
    }
    else if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_1) == GPIO_PIN_RESET) {
        data->state = LV_INDEV_STATE_PR;
        data->key = LV_KEY_RIGHT;
    }
    else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void lv_port_indev_init(void)
{
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);

    indev_drv.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv.read_cb = my_input_read;

    lv_indev_drv_register(&indev_drv);
}
