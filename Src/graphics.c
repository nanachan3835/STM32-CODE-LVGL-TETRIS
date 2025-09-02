#include "graphics.h"
#include "Ucglib.h"     // Thư viện đồ họa chính
#include <stdio.h>      // Dùng cho sprintf

// Biến ucglib toàn cục, chỉ dùng trong file này
static ucg_t g_ucg;

// --- CÁC HÀM HELPER NỘI BỘ (STATIC) ---

// Chuyển đổi tọa độ game (cột, hàng) sang tọa độ pixel
static void get_pixel_coords(int8_t x, int8_t y, int* out_px, int* out_py) {
    *out_px = BOARD_OFFSET_X + x * CELL_SIZE_PX;
    *out_py = BOARD_OFFSET_Y + y * CELL_SIZE_PX;
}

// Hàm vẽ một ô duy nhất lên màn hình
static void draw_cell(int8_t x, int8_t y, BlockType type) {
    if (!Game_InBounds(x, y)) return;

    int px, py;
    get_pixel_coords(x, y, &px, &py);

    if (type != BLOCK_NONE) {
        // Đặt màu vẽ là trắng
        ucg_SetColor(&g_ucg, 0, 255, 255, 255);
        ucg_DrawBox(&g_ucg, px, py, CELL_SIZE_PX, CELL_SIZE_PX);
    } else {
        // Đặt màu vẽ là đen (màu nền) để xóa
        ucg_SetColor(&g_ucg, 0, 0, 0, 0);
        ucg_DrawBox(&g_ucg, px, py, CELL_SIZE_PX, CELL_SIZE_PX);
    }
}

// --- HIỆN THỰC API CÔNG KHAI ---

void Gfx_Init(void) {
    Ucglib4WireSWSPI_begin(&g_ucg, UCG_FONT_MODE_SOLID);
    ucg_SetRotate180(&g_ucg);
    Gfx_Clear();
}

void Gfx_Clear(void) {
    ucg_ClearScreen(&g_ucg);
}

void Gfx_DrawBoard(void) {
    // 1. Vẽ khung viền
    ucg_SetColor(&g_ucg, 0, 255, 255, 255);
    ucg_DrawFrame(&g_ucg, 0, 0, BOARD_W * CELL_SIZE_PX + 2, BOARD_H * CELL_SIZE_PX + 2);

    // 2. Vẽ các khối đã được cố định
    for (int y = 0; y < BOARD_H; y++) {
        for (int x = 0; x < BOARD_W; x++) {
            GameCell cell_type = Game_GetBoardCell(x, y);
            draw_cell(x, y, (BlockType)cell_type);
        }
    }

    // 3. Vẽ khối đang rơi
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

    // 4. Vẽ bóng khối (ghost piece)
    int8_t ghost_y = Game_GetGhostY();
    if (ghost_y >= 0 && cur.type != BLOCK_NONE) {
        ucg_SetColor(&g_ucg, 0, 255, 255, 255); // Màu trắng
        for (int by = 0; by < 4; by++) {
            for (int bx = 0; bx < 4; bx++) {
                if (Game_ShapeBit(cur.shape_mask, bx, by)) {
                    int px, py;
                    get_pixel_coords(cur.x + bx, ghost_y + by, &px, &py);
                    // Dùng ucg_DrawFrame để vẽ hình chữ nhật rỗng
                    ucg_DrawFrame(&g_ucg, px, py, CELL_SIZE_PX, CELL_SIZE_PX);
                }
            }
        }
    }
}

void Gfx_DrawNext(BlockType next) {
    int x0 = BOARD_OFFSET_X + (BOARD_W * CELL_SIZE_PX) + 10;
    int y0 = 4;

    // Xóa vùng "Next" cũ
    ucg_SetColor(&g_ucg, 0, 0, 0, 0); // Màu đen
    ucg_DrawBox(&g_ucg, x0, y0, 4 * CELL_SIZE_PX, 4 * CELL_SIZE_PX);

    // Vẽ khối mới
    ucg_SetColor(&g_ucg, 0, 255, 255, 255); // Màu trắng
    ucg_SetFont(&g_ucg, ucg_font_6x10_tf);
    ucg_DrawString(&g_ucg, x0, y0 - 2, 0, "Next:");

    uint16_t mask = Game_GetShapeMask(next, ROT_0);
    for (int by = 0; by < 4; by++) {
        for (int bx = 0; bx < 4; bx++) {
            if (Game_ShapeBit(mask, bx, by)) {
                ucg_DrawBox(&g_ucg, x0 + bx * CELL_SIZE_PX, y0 + by * CELL_SIZE_PX, CELL_SIZE_PX, CELL_SIZE_PX);
            }
        }
    }
}

void Gfx_DrawStats(const GameStats* stats) {
	char buf[20];
	    int x0 = BOARD_OFFSET_X + (BOARD_W * CELL_SIZE_PX) + 10;
	    int y0 = 40;

	    // Xóa vùng stats cũ
	    ucg_SetColor(&g_ucg, 0, 0, 0, 0);
	    ucg_DrawBox(&g_ucg, x0, y0, 60, 30);

	    // Vẽ stats mới
	    ucg_SetColor(&g_ucg, 0, 255, 255, 255);
	    // *** SỬA LỖI: DÙNG FONT NHỎ HƠN ***
	    ucg_SetFont(&g_ucg, ucg_font_5x8_tf); // Sử dụng font 5x8 nhỏ gọn

	    // Điều chỉnh lại tọa độ Y cho phù hợp với font mới
	    sprintf(buf, "Score:%lu", (unsigned long)stats->score);
	    ucg_DrawString(&g_ucg, x0, y0 + 8, 0, buf);
	    sprintf(buf, "Lines:%lu", (unsigned long)stats->lines_total);
	    ucg_DrawString(&g_ucg, x0, y0 + 18, 0, buf);
	    sprintf(buf, "Level:%u", stats->level);
	    ucg_DrawString(&g_ucg, x0, y0 + 28, 0, buf);
}

void Gfx_Refresh(void) {
    // Với Ucglib, việc vẽ thường diễn ra ngay lập tức.
    // Hàm này được giữ lại để đảm bảo tính nhất quán của API.
    // Nếu driver màn hình của bạn có buffer, lệnh flush sẽ được đặt ở đây.
}
