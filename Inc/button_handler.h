// button_handler.h

#ifndef BUTTON_HANDLER_H_
#define BUTTON_HANDLER_H_

#include <stdint.h>
#include "button.h" // Cần cho typedef của Lumi

/**
 * @brief Khởi tạo hệ thống nút bấm và đăng ký callback.
 *        Hàm này sẽ kết nối ngắt phần cứng với module GameState.
 */
void ButtonHandler_Init(void);

#endif /* BUTTON_HANDLER_H_ */
