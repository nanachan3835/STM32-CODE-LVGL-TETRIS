#include "graphics.h"
#include "Ucglib.h"     // Thư viện đồ họa chính
#include <stdio.h>      // Dùng cho sprintf
#include <string.h>
// Biến ucglib toàn cục, chỉ dùng trong file này
static ucg_t g_ucg;

// --- CÁC HÀM HELPER NỘI BỘ (STATIC) ---

// Hàm itoa đơn giản (integer to ascii) để chuyển số sang kí tự  chuỗi
static void itoa(uint32_t value, char* str) {
    char* ptr = str;
    char* ptr1 = str;
    char tmp_char;
    uint32_t tmp_value;

    if (value == 0) {
        *ptr++ = '0';
        *ptr = '\0';
        return;
    }

    while (value) {
        tmp_value = value;
        value /= 10;
        *ptr++ = "0123456789"[tmp_value % 10];
    }

    *ptr-- = '\0';
    while (ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr-- = *ptr1;
        *ptr1++ = tmp_char;
    }
}


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

//hàm xử lí vẽ rơi
static void draw_current_piece_internal(bool do_erase) {
    FallingBlock cur;
    if (Game_GetCurrentBlock(&cur) != GAME_OK) {
        return;
    }

    if (do_erase) {
        ucg_SetColor(&g_ucg, 0, 0, 0, 0);
    } else {
        ucg_SetColor(&g_ucg, 0, 255, 255, 255);
    }

    for (int by = 0; by < 4; by++) {
        for (int bx = 0; bx < 4; bx++) {
            if (Game_ShapeBit(cur.shape_mask, bx, by)) {
                int px, py;
                get_pixel_coords(cur.x + bx, cur.y + by, &px, &py);
                ucg_DrawBox(&g_ucg, px, py, CELL_SIZE_PX, CELL_SIZE_PX);
            }
        }
    }
}




// api cơ bản

void Gfx_Init(void) {
    Ucglib4WireSWSPI_begin(&g_ucg, UCG_FONT_MODE_SOLID);
    ucg_SetRotate180(&g_ucg);
    Gfx_Clear();
}

void Gfx_Clear(void) {
    ucg_ClearScreen(&g_ucg);
}


void Gfx_DrawFallingPiece(void) {
    draw_current_piece_internal(false);
}

void Gfx_EraseFallingPiece(void) {
    draw_current_piece_internal(true);
}


void Gfx_DrawBoard(void) {
    // 1. Vẽ khung viền
    ucg_SetColor(&g_ucg, 0, 255, 255, 255);
    ucg_DrawFrame(&g_ucg, 0, 0, BOARD_W * CELL_SIZE_PX + 2, BOARD_H * CELL_SIZE_PX + 2);

    // 2. Vẽ các khối đã được cố định
    for (int y = 0; y < BOARD_H; y++) {
        for (int x = 0; x < BOARD_W; x++) {
            GameCell cell_type = Game_GetBoardCell(x, y);
            // Chỉ vẽ các ô không trống để tối ưu
            if (cell_type != BLOCK_NONE) {
                 draw_cell(x, y, (BlockType)cell_type);
            }
        }
    }

    // 3. Vẽ khối đang rơi
//Gfx_DrawFallingPiece();
}

void Gfx_DrawNext(BlockType next) {
    int x0 = BOARD_OFFSET_X + (BOARD_W * CELL_SIZE_PX) + 10;
    int y0 = 4;
    ucg_SetFont(&g_ucg, ucg_font_helvR08_tf);
    // Xóa vùng "Next" cũ
    ucg_SetColor(&g_ucg, 0, 0, 0, 0); // Màu đen
    ucg_DrawBox(&g_ucg, x0, y0, 4 * CELL_SIZE_PX, 4 * CELL_SIZE_PX);

    // Vẽ khối mới
    ucg_SetColor(&g_ucg, 0, 255, 0, 0);
     // Font nhỏ gọn

    uint16_t mask = Game_GetShapeMask(next, ROT_0);
    for (int by = 0; by < 4; by++) {
        for (int bx = 0; bx < 4; bx++) {
            if (Game_ShapeBit(mask, bx, by)) {
                ucg_DrawBox(&g_ucg, x0 + bx * CELL_SIZE_PX, y0 + by * CELL_SIZE_PX, CELL_SIZE_PX, CELL_SIZE_PX);
            }
        }
    }
    ucg_DrawString(&g_ucg, x0, y0 + 40, 0, "Next");
}

// vẽ stats game (hiện tại chỉ có score)
void Gfx_DrawStats(const GameStats* stats) {
	    char buf[20];
	    char num_buf[15];
	    int x0 = BOARD_OFFSET_X + (BOARD_W * CELL_SIZE_PX);
	    int y0 = 40;

	    ucg_SetFont(&g_ucg, ucg_font_5x8_tf);
	    // Xóa vùng stats cũ
	    ucg_SetColor(&g_ucg, 0, 0, 0, 0);
	    ucg_DrawBox(&g_ucg, x0, y0, 60, 30);

	    // Vẽ stats mới
	    ucg_SetColor(&g_ucg, 0, 255, 255, 255);
	    //score
	    strcpy(buf, "Score:");
	    itoa((unsigned long)stats->score, num_buf);
	    strcat(buf, num_buf);
	    ucg_DrawString(&g_ucg, x0, y0, 0, buf);
}

void Gfx_Refresh(void) {
    // Hàm này sẽ vẽ lại toàn bộ bàn chơi, khối tiếp theo và stats
}



// CẢI TIẾN: Hiện thực các hàm vẽ cho các trạng thái game
void Gfx_DrawStartMenu(void) {
    ucg_SetFont(&g_ucg,ucg_font_helvR08_tf); // Font lớn cho tiêu đề
    ucg_SetColor(&g_ucg, 0, 255, 255, 255);

    // Căn giữa tiêu đề "TETRIS"
    ucg_int_t width = ucg_GetStrWidth(&g_ucg, "TETRIS");
    ucg_DrawString(&g_ucg, (ucg_GetWidth(&g_ucg) - width) / 2, 40, 0, "TETRIS");
}


void Gfx_DrawPausedScreen(void) {
    ucg_SetFont(&g_ucg, ucg_font_helvR08_tf);
    ucg_SetColor(&g_ucg, 0, 255, 255, 255);

    // Vẽ chữ "PAUSED" ở giữa màn hình
    ucg_int_t width = ucg_GetStrWidth(&g_ucg, "PAUSED");
    ucg_DrawString(&g_ucg, (ucg_GetWidth(&g_ucg) - width) / 2, 60, 0, "PAUSED");
}


void Gfx_DrawGameOverScreen(const GameStats* stats) {
    char buf[20];
    char num_buf[20];
    ucg_SetFont(&g_ucg, ucg_font_helvR08_hf); // Font lớn
    ucg_SetColor(&g_ucg, 0, 255, 255, 255);

    // Vẽ "GAME OVER"
    ucg_int_t width = ucg_GetStrWidth(&g_ucg, "GAME OVER");
    ucg_DrawString(&g_ucg, (ucg_GetWidth(&g_ucg) - width) / 2, 40, 0, "GAME OVER");

    // sử dụng string.h để xử lí nhập chuỗi
    strcpy(buf, "Score:");
    itoa((unsigned long)stats->score, num_buf);
    strcat(buf, num_buf);
    width = ucg_GetStrWidth(&g_ucg, buf);
    ucg_DrawString(&g_ucg, (ucg_GetWidth(&g_ucg) - width) / 2, 65, 0, buf);
    // Hướng dẫn
    width = ucg_GetStrWidth(&g_ucg, "Restart");
    ucg_DrawString(&g_ucg, (ucg_GetWidth(&g_ucg) - width) / 2, 90, 0, "Restart");
}
