#ifndef UI_H
#define UI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"

/* Khởi tạo toàn bộ giao diện */
void ui_init(void);

/* Cập nhật giao diện (vẽ lại canvas, điểm, khối, ...) */
void ui_update(void);

#ifdef __cplusplus
}
#endif

#endif /* UI_H */
