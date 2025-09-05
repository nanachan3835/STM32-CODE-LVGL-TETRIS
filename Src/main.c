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
#include "logic.h"
//int tick_count=0;

// Callback được gọi bởi Timer của Lumi, với tần số bằng TICK_HZ
void GameTick_Callback(void) {
    GameState_ProcessTick();
}

int main(void) {
    SystemCoreClockUpdate();
    TimerInit();
    Button_Init();
    EventButton_Init();
    ButtonHandler_Init();
    GameState_Init(12345);
    BuzzerControl_Init();
    const uint32_t game_tick_period_ms = 1000 / TICK_HZ;
    TimerStart("GameTick", game_tick_period_ms, TIMER_REPEAT_FOREVER, (void*)GameTick_Callback, NULL);
    while (1) {
        // Vòng lặp chính chỉ cần chạy bộ lập lịch timer của Lumi
        processTimerScheduler();
    }
    return 0;
}
