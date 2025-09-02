// main.c

#include <system_stm32f4xx.h>
#include "timer.h"
#include "button.h"
#include "eventbutton.h"
#include <stddef.h>
// Các module middleware của chúng ta
#include "gamestate.h"
#include "button_handler.h"
#include "buzzer.h"

// Callback được gọi bởi Timer của Lumi, với tần số bằng TICK_HZ
void GameTick_Callback(void) {
    GameState_ProcessTick();
}

int main(void) {
	//SystemClock_Config();
    SystemCoreClockUpdate();
    TimerInit();
    //MX_TIM2_Init();

    // Khởi tạo các module cấp thấp của Lumi
    Button_Init();
    EventButton_Init();

    // Khởi tạo middleware của chúng ta
    ButtonHandler_Init();   // Kết nối ngắt với GameState
    GameState_Init(12345);  // Khởi tạo toàn bộ game với một seed
    BuzzerControl_Init();         // Khởi tạo buzzer
    // Tạo một timer phần mềm để gọi GameTick_Callback
    // Ví dụ: TICK_HZ = 30 -> 1000ms / 30 ~= 33ms
    TimerStart("GameTick", 33, TIMER_REPEAT_FOREVER, (void*)GameTick_Callback, NULL);
    //HAL_TIM_Base_Start_IT(&htim2);
    while (1) {
        // Vòng lặp chính chỉ cần chạy bộ lập lịch timer của Lumi
        processTimerScheduler();
    }
    return 0;
}
