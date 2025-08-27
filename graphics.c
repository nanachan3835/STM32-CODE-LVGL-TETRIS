#include "graphics.h"
#include "ssd1306.h"   // driver phần cứng

/* ====== Nội bộ ========================================================= */

// map BlockType -> pixel intensity
static uint8_t block_color(BlockType type) {
    if (type == BLOCK_NONE) return 0;
    return 1; // ở SSD1306 chỉ có đen/trắng
}

// vẽ 1 ô (cell) lên màn hình
static void draw_cell(int8_t x, int8_t y, BlockType type) {
    if (!Game_InBounds(x, y)) return;
    int px = BOARD_OFFSET_X + x * CELL_SIZE_PX;
    int py = BOARD_OFFSET_Y + y * CELL_SIZE_PX;
    if (block_color(type)) {
        ssd1306_DrawFilledRectangle(px, py, px + CELL_SIZE_PX - 1, py + CELL_SIZE_PX - 1, White);
    } else {
        ssd1306_DrawFilledRectangle(px, py, px + CELL_SIZE_PX - 1, py + CELL_SIZE_PX - 1, Black);
    }
}

/* ====== API công khai ================================================== */

void Gfx_Init(void) {
    ssd1306_Init();
    Gfx_Clear();
    Gfx_Refresh();
}

void Gfx_Clear(void) {
    ssd1306_Fill(Black);
}

void Gfx_DrawBoard(void) {
    // vẽ nền
    for (int y = 0; y < BOARD_H; y++) {
        for (int x = 0; x < BOARD_W; x++) {
            GameCell cell = Game_GetBoardCell(x, y);
            draw_cell(x, y, (BlockType)cell);
        }
    }

    // vẽ khối rơi hiện tại
    FallingBlock cur;
    if (Game_GetCurrentBlock(&cur) == GAME_OK) {
        for (int by = 0; by < 4; by++) {
            for (int bx = 0; bx < 4; bx++) {
                if (Game_ShapeBit(cur.shape_mask, bx, by)) {
                    draw_cell(cur.x + bx, cur.y + by, cur.type);
                }
            }
        }
    }

    // vẽ ghost
    int8_t ghost_y = Game_GetGhostY();
    if (ghost_y >= 0 && cur.type != BLOCK_NONE) {
        for (int by = 0; by < 4; by++) {
            for (int bx = 0; bx < 4; bx++) {
                if (Game_ShapeBit(cur.shape_mask, bx, by)) {
                    int gx = cur.x + bx;
                    int gy = ghost_y + by;
                    ssd1306_DrawRectangle(
                        BOARD_OFFSET_X + gx * CELL_SIZE_PX,
                        BOARD_OFFSET_Y + gy * CELL_SIZE_PX,
                        BOARD_OFFSET_X + (gx + 1) * CELL_SIZE_PX - 1,
                        BOARD_OFFSET_Y + (gy + 1) * CELL_SIZE_PX - 1,
                        White
                    );
                }
            }
        }
    }
}

void Gfx_DrawNext(BlockType next) {
    int x0 = 80, y0 = 4; // vị trí ô preview
    ssd1306_DrawString(x0, y0 - 10, "Next:", Font_6x8, White);
    uint16_t mask = Game_GetShapeMask(next, ROT_0);
    for (int by = 0; by < 4; by++) {
        for (int bx = 0; bx < 4; bx++) {
            if (Game_ShapeBit(mask, bx, by)) {
                ssd1306_DrawFilledRectangle(
                    x0 + bx * CELL_SIZE_PX,
                    y0 + by * CELL_SIZE_PX,
                    x0 + (bx+1) * CELL_SIZE_PX - 1,
                    y0 + (by+1) * CELL_SIZE_PX - 1,
                    White
                );
            }
        }
    }
}

void Gfx_DrawStats(const GameStats* stats) {
    char buf[20];
    sprintf(buf, "Score:%lu", (unsigned long)stats->score);
    ssd1306_DrawString(80, 30, buf, Font_6x8, White);
    sprintf(buf, "Lines:%lu", (unsigned long)stats->lines_total);
    ssd1306_DrawString(80, 40, buf, Font_6x8, White);
    sprintf(buf, "Level:%u", stats->level);
    ssd1306_DrawString(80, 50, buf, Font_6x8, White);
}

void Gfx_Refresh(void) {
    ssd1306_UpdateScreen();
}
