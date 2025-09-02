// button_handler.c

#include "button_handler.h"
#include "button.h"
#include "gamestate.h" // Để gọi hàm xử lý của GameState

/**
 * @brief Hàm callback duy nhất được gọi bởi thư viện Lumi khi có ngắt.
 *        Hàm này hoạt động như một "người đưa tin" siêu nhanh.
 */
static void Button_ISR_Callback(uint8_t id, uint16_t time) {
    (void)time; // Báo cho trình biên dịch biết là chúng ta không dùng tham số này

    // Chuyển ngay ID của nút bấm vật lý cho module GameState xử lý
    GameState_ProcessInput(id);
}

void ButtonHandler_Init(void) {
    // Đăng ký MỘT hàm callback duy nhất cho TẤT CẢ các sự kiện nhấn nút
    Button_RegisterEventCallback(BUTTON_EVENT_PRESS, Button_ISR_Callback);
}
