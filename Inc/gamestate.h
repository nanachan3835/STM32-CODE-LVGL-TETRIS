// gamestate.h

#ifndef GAMESTATE_H_
#define GAMESTATE_H_

#include <stdint.h>
#include <stdbool.h>
#include "logic.h" // Quan trọng: Dùng các kiểu dữ liệu từ engine game của bạn

// Định nghĩa ID cho các nút bấm vật lý để khớp với thư viện Lumi
#define BUTTON_ID_UP      1 // SW1
#define BUTTON_ID_DOWN    5 // SW5
#define BUTTON_ID_LEFT    2 // SW2
#define BUTTON_ID_RIGHT   4 // SW4
#define BUTTON_ID_CENTER  3 // SW3

typedef enum {
    GAME_STATE_START_MENU,
    GAME_STATE_PLAYING,
    GAME_STATE_PAUSED,
    GAME_STATE_GAME_OVER
} GameState_e;

// API chính của middleware
void GameState_Init(uint32_t seed);
void GameState_ProcessInput(uint8_t physical_button_id);
void GameState_ProcessTick(void);

#endif /* GAMESTATE_H_ */
