#include "ui.h"

static lv_obj_t * canvas;
static lv_obj_t * label_score;

/* Khởi tạo UI */
void ui_init(void)
{
    /* Canvas để vẽ sân chơi */
    canvas = lv_canvas_create(lv_scr_act());
    lv_obj_set_size(canvas, 128, 160);  // chỉnh theo màn hình ST7735
    lv_obj_align(canvas, LV_ALIGN_CENTER, 0, 0);

    /* Label điểm số */
    label_score = lv_label_create(lv_scr_act());
    lv_label_set_text(label_score, "Score: 0");
    lv_obj_align(label_score, LV_ALIGN_TOP_MID, 0, 5);
}

/* Hàm update giao diện (gọi mỗi frame hoặc khi logic báo thay đổi) */
void ui_update(void)
{
    // TODO: lấy dữ liệu từ logic (score, board state)
    // ví dụ test tạm:
    lv_label_set_text_fmt(label_score, "Score: %d", 123);
}
