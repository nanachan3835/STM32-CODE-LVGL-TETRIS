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

    Gfx_Clear(); // Xóa màn hình bằng Ucglib

    switch (s_current_state) {
        case GAME_STATE_START_MENU:
            // TODO: Tạo một hàm Gfx_DrawStartMenu() trong graphics.c để vẽ menu đẹp hơn
            // Tạm thời, chúng ta sẽ vẽ một thông báo đơn giản bằng các hàm Gfx
            Gfx_DrawStats(&summary.stats); // Vẽ stats để debug
            break;

        case GAME_STATE_PLAYING:
            Gfx_DrawBoard();
            Gfx_DrawNext(summary.next_block);
            Gfx_DrawStats(&summary.stats);
            break;

        case GAME_STATE_PAUSED:
            // TODO: Tạo hàm Gfx_DrawPausedScreen() trong graphics.c
            Gfx_DrawStats(&summary.stats);
            break;

        case GAME_STATE_GAME_OVER:
            Gfx_DrawBoard(); // Vẽ lại bàn chơi lần cuối
            Gfx_DrawStats(&summary.stats);
            // TODO: Tạo hàm Gfx_DrawGameOverText() trong graphics.c
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
    Gfx_Init();      // Khởi tạo Ucglib
    Game_Init(seed); // Khởi tạo logic game

    s_current_state = GAME_STATE_START_MENU;
    render_full_scene();
}

void GameState_ProcessTick(void) {
    if (s_current_state != GAME_STATE_PLAYING) {
        return;
    }

    // *** BƯỚC 1: XÓA VỊ TRÍ CŨ ***

    // *** BƯỚC 2: CẬP NHẬT LOGIC GAME ***
    GameTickEvent event;
    Game_Tick(&event); // Engine tự xử lý việc rơi xuống 1 ô

    // *** BƯỚC 3: XỬ LÝ KẾT QUẢ VÀ VẼ LẠI ***
    if (event.game_over) {
        s_current_state = GAME_STATE_GAME_OVER;
        render_full_scene(); // Vẽ lại toàn bộ màn hình game over
    }
    else if (event.lines_cleared > 0) {
        // Nếu có xóa hàng, toàn bộ bàn chơi thay đổi -> vẽ lại toàn bộ
        render_full_scene();
    }
    else if (event.block_locked) {
        // Khối cũ đã được khóa và trở thành 1 phần của bàn chơi
        // Khối mới đã được tạo ra
        // Vẽ lại bàn chơi (bao gồm cả khối vừa khóa) và khối mới
        Gfx_DrawBoard();
        
        // Cập nhật các thông tin phụ
        GameSummary summary;
        Game_GetSummary(&summary);
        Gfx_DrawNext(summary.next_block);
        Gfx_DrawStats(&summary.stats);
        
        //Gfx_Refresh();
    }
    else {
        // **TRƯỜNG HỢP PHỔ BIẾN NHẤT: KHỐI CHỈ RƠI XUỐNG**
        // Chúng ta đã xóa nó ở vị trí cũ, giờ chỉ cần vẽ nó ở vị trí mới
        Gfx_DrawFallingPiece();
        Gfx_EraseFallingPiece();
        //Gfx_Refresh();
    }
}

void GameState_ProcessInput(uint8_t physical_button_id) {
    bool needs_full_render = false;
    switch (s_current_state) {
        case GAME_STATE_START_MENU:
            if (physical_button_id == BUTTON_ID_CENTER) {
                start_new_game();
            }
            break;

        case GAME_STATE_PLAYING:
            switch (physical_button_id) {
                case BUTTON_ID_LEFT:   Game_MoveLeft();   break;
                case BUTTON_ID_RIGHT:  Game_MoveRight();  break;
                case BUTTON_ID_UP:     Game_RotateCW();   break;
                case BUTTON_ID_DOWN:   Game_SoftDrop();   break;
                case BUTTON_ID_CENTER:
                    // Chức năng Hard Drop khi nhấn nút giữa lúc đang chơi
                    Game_HardDrop(NULL);
                    // Sau khi hard drop, cần cập nhật toàn bộ vì có thể có xóa hàng
                    //needs_full_render = true;
                    break;
            }
            break;

        case GAME_STATE_PAUSED:
            // Tạm thời chưa có, sẽ thêm sau
            break;

        case GAME_STATE_GAME_OVER:
            if (physical_button_id == BUTTON_ID_CENTER) {
                // Quay lại menu chính, cần vẽ lại toàn bộ
                s_current_state = GAME_STATE_START_MENU;
                Game_Reset(false);
                needs_full_render = true;
            }
            break;
    }

    if (needs_full_render) {
        render_full_scene();
    } else if (s_current_state == GAME_STATE_PLAYING) {
        // Sau khi di chuyển/xoay, chỉ cần vẽ lại bàn chơi là đủ, không cần xóa toàn bộ màn hình
       Gfx_DrawFallingPiece();
       Gfx_EraseFallingPiece();
    }
}
