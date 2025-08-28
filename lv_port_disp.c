#include "lv_port_disp.h"
#include "st7735.h"   // Driver ST7735  (có sẵn)
#include "main.h"

#define MY_DISP_HOR_RES    128
#define MY_DISP_VER_RES    160

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[MY_DISP_HOR_RES * 10];   /* Buffer 1 hàng */
static lv_color_t buf2[MY_DISP_HOR_RES * 10];   /* Buffer 2 hàng */

/* Callback LVGL yêu cầu flush vùng ảnh */
static void my_disp_flush(lv_disp_drv_t * disp_drv,
                          const lv_area_t * area, lv_color_t * color_p)
{
    ST7735_DrawBitmap(area->x1, area->y1,
                      (uint16_t*)color_p,
                      area->x2 - area->x1 + 1,
                      area->y2 - area->y1 + 1);

    lv_disp_flush_ready(disp_drv);  /* Báo LVGL là xong */
}

void lv_port_disp_init(void)
{
    /* Init LCD driver */
    ST7735_Init();

    /* Init buffer */
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, MY_DISP_HOR_RES * 10);

    /* Init driver */
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);

    disp_drv.hor_res = MY_DISP_HOR_RES;
    disp_drv.ver_res = MY_DISP_VER_RES;
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;

    lv_disp_drv_register(&disp_drv);
}
