/*  graphics.h  —  Module hiển thị Tetris cho STM32*/

#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "logic.h"   // dùng GameCell, BlockType, FallingBlock, GameSummary

/* Cấu hình đồ họa */
#define CELL_SIZE_PX        6       // số pixel cho 1 ô
#define BOARD_OFFSET_X      2       // offset vẽ bàn trên màn hình
#define BOARD_OFFSET_Y      2

/*  API chính  */

// Khởi tạo module graphics
void Gfx_Init(void);

// Xóa toàn bộ màn hình
void Gfx_Clear(void);

// Vẽ toàn bộ bàn chơi + khối rơi + ghost
void Gfx_DrawBoard(void);

// Vẽ khối tiếp theo (next piece)
void Gfx_DrawNext(BlockType next);

// Vẽ thông tin thống kê: score, level, lines
void Gfx_DrawStats(const GameStats* stats);

// Cập nhật (flush buffer -> màn hình thật)
void Gfx_Refresh(void);

#ifdef __cplusplus
}
#endif

#endif /* GRAPHICS_H */
