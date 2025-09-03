/*  graphics.h  —  Module hiển thị Tetris cho STM32 sử dụng Ucglib */

#ifndef GRAPHICS_H
#define GRAPHICS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "logic.h"   // Dùng các kiểu GameCell, BlockType, GameStats...

/* Cấu hình đồ họa */
#define CELL_SIZE_PX        6       // Kích thước mỗi ô (đã giảm từ 6 xuống 5 để vừa vặn hơn)
#define BOARD_OFFSET_X      2       // Lề trái của sân chơi
#define BOARD_OFFSET_Y      2       // Lề trên của sân chơi

/*  API chính  */
void Gfx_Init(void);
void Gfx_Clear(void);
void Gfx_DrawBoard(void);
void Gfx_DrawNext(BlockType next);
void Gfx_DrawStats(const GameStats* stats);
void Gfx_Refresh(void); // Dù Ucglib vẽ trực tiếp, giữ hàm này cho API nhất quán


// *** CÁC HÀM MỚI ĐỂ RENDER HIỆU QUẢ ***
/**
 * @brief Chỉ vẽ khối gạch đang rơi lên màn hình.
 */
void Gfx_DrawFallingPiece(void);

/**
 * @brief Chỉ xóa khối gạch đang rơi khỏi màn hình (vẽ đè màu nền).
 */
void Gfx_EraseFallingPiece(void);


#ifdef __cplusplus
}
#endif

#endif /* GRAPHICS_H */
