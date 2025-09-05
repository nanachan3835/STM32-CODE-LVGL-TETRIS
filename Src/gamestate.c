// gamestate.c

#include "gamestate.h"
#include "logic.h"      // Engine game
#include "graphics.h"   // Engine đồ họa (đã được cập nhật để dùng Ucglib)
#include <stddef.h>
#include "melody.h"
#include "buzzer.h"
// Biến trạng thái nội bộ của middleware
static GameState_e s_current_state;

// --- Các hàm nội bộ để quản lý luồng game ---

/**
 * @brief Hàm này giờ đây sẽ gọi các hàm Gfx_... để vẽ lại toàn bộ
 *        cảnh game dựa trên trạng thái hiện tại.
 */
static void render_full_scene(void) {
    GameSummary summary;
    Game_GetSummary(&summary);

    Gfx_Clear();

    switch (s_current_state) {
        case GAME_STATE_START_MENU:
            Gfx_DrawStartMenu();

            break;

        case GAME_STATE_PLAYING:
            Gfx_DrawBoard();
            Gfx_DrawNext(summary.next_block);
            break;

        case GAME_STATE_PAUSED:
        	Gfx_DrawPausedScreen();
            break;

        case GAME_STATE_GAME_OVER:
        	Gfx_DrawGameOverScreen(&summary.stats);
            break;
    }

    Gfx_Refresh(); // Cập nhật màn hình (dù có thể không cần với Ucglib)
}

static void start_new_game(void) {
    Game_Reset(false); // Reset logic game, không giữ lại stats
    s_current_state = GAME_STATE_PLAYING;
    render_full_scene();
}

// --- API công khai của middleware ---

void GameState_Init(uint32_t seed) {
    Gfx_Init();
    Game_Init(seed);

    s_current_state = GAME_STATE_START_MENU;
    render_full_scene();
}

//ham xử lí tick game dat trong timer
void GameState_ProcessTick(void) {
    if (s_current_state != GAME_STATE_PLAYING) {
        return;
    }

    GameTickEvent event;
    Game_Tick(&event);
    GameSummary summary;

    // Engine tự xử lý việc rơi xuống 1 ô
    if (event.game_over) {
        s_current_state = GAME_STATE_GAME_OVER;
        render_full_scene();
    }
    else if (event.lines_cleared > 0) {
        render_full_scene();
    }
    else if (event.block_locked) {
        Gfx_DrawBoard();
        //GameSummary summary;
        Game_GetSummary(&summary);
        Gfx_DrawNext(summary.next_block);
    }
    else {
        //Gfx_EraseFallingPiece();
    	Gfx_DrawFallingPiece();
        Gfx_EraseFallingPiece();
    }
}

//xử lí nút bấm trong state machine
void GameState_ProcessInput(uint8_t physical_button_id) {
    bool needs_full_render = false;
    switch (s_current_state) {
        case GAME_STATE_START_MENU:
            if (physical_button_id == BUTTON_ID_CENTER) {
                start_new_game();
                BuzzerControl_SetMelody(p2beep);
            }
            break;

        case GAME_STATE_PLAYING:
            switch (physical_button_id) {
                case BUTTON_ID_LEFT:   Game_MoveLeft(); BuzzerControl_SetMelody(pbeep);   break;
                case BUTTON_ID_RIGHT:  Game_MoveRight();  BuzzerControl_SetMelody(pbeep);break;
                case BUTTON_ID_UP:     Game_RotateCW();   BuzzerControl_SetMelody(pbeep); break;
                case BUTTON_ID_DOWN:   Game_HardDrop(NULL);  BuzzerControl_SetMelody(pbeep); break;
                case BUTTON_ID_CENTER: 
                    s_current_state = GAME_STATE_PAUSED;
                    Game_SetPaused(true);
                    BuzzerControl_SetMelody(p2beep);
                    needs_full_render = true; 
                    break;
                default:
                    //Game_SoftDrop();
                    break;
            }
            break;

        case GAME_STATE_PAUSED:
        	if (physical_button_id == BUTTON_ID_CENTER) {
        	                s_current_state = GAME_STATE_PLAYING;
        	                Game_SetPaused(false);
        	                BuzzerControl_SetMelody(p2beep);
        	                needs_full_render = true;
        	            }
            break;

        case GAME_STATE_GAME_OVER:
            if (physical_button_id == BUTTON_ID_CENTER) {
                s_current_state = GAME_STATE_START_MENU;
                Game_Reset(false);
                BuzzerControl_SetMelody(p2beep);
                needs_full_render = true;
            }
            break;
    }

    if (needs_full_render) {
        render_full_scene();
    }
}
