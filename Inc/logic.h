/* ============================================================================
 *  logic.h  —  Game logic 6 khối tùy chỉnh cho STM32F401RE
 *  Tác giả : Embedded Developer  
 *  Ghi chú : 6 khối I, L, O, T, U, PLUS - tối ưu cho STM32
 * ========================================================================== */

#ifndef LOGIC_H
#define LOGIC_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>

/* ====== Cấu hình game =================================================== */
#define BOARD_W                 10      // Số cột
#define BOARD_H                 20      // Số hàng  
#define TICK_HZ                 25      // Tần số logic (Hz)
#define LOCK_DELAY_TICKS        15      // Độ trễ khóa khối (~0.5s)
#define GRAVITY_BASE_TICKS      33      // Tốc độ rơi cơ bản (1s/ô)

// Macro ngắt an toàn cho STM32
#define CRITICAL_ENTER()        __disable_irq()
#define CRITICAL_EXIT()         __enable_irq()

/* ====== Kiểu dữ liệu ==================================================== */

typedef uint8_t GameCell;       // Ô trên bàn: 0=trống, 1-6=loại khối

// Mã lỗi trả về
typedef enum {
    GAME_OK = 0,                // Thành công
    GAME_COLLISION,             // Va chạm
    GAME_OUT_OF_BOUNDS,         // Ra ngoài biên  
    GAME_GAME_OVER,             // Kết thúc game
    GAME_INVALID_BLOCK          // Khối không hợp lệ
} GameResult;

// 6 loại khối tùy chỉnh
typedef enum {
    BLOCK_NONE = 0,
    BLOCK_I = 1,                // Khối thẳng |
    BLOCK_L = 2,                // Khối chữ L  
    BLOCK_O = 3,                // Khối vuông []
    BLOCK_T = 4,                // Khối chữ T
    BLOCK_U = 5,                // Khối chữ U
    BLOCK_PLUS = 6              // Khối dấu +
} BlockType;

// Hướng xoay
typedef enum {
    ROT_0   = 0,                // 0°
    ROT_90  = 1,                // 90° thuận chiều kim đồng hồ
    ROT_180 = 2,                // 180°
    ROT_270 = 3                 // 270°
} BlockRotation;

// Khối đang rơi
typedef struct {
    BlockType       type;       // Loại khối
    BlockRotation   rotation;   // Hướng xoay hiện tại
    int8_t          x, y;       // Vị trí (góc trái-trên của ô 4x4)
    uint16_t        shape_mask; // Hình dạng dạng bitmask 4x4
    uint8_t         lock_timer; // Bộ đếm delay khóa khối
} FallingBlock;

// Thống kê game
typedef struct {
    uint32_t score;             // Điểm số
    uint32_t lines_total;       // Tổng số hàng đã xóa
    uint16_t level;             // Cấp độ
    uint16_t blocks_placed;     // Số khối đã đặt
} GameStats;

// Cờ trạng thái game
typedef union {
    uint8_t raw;
    struct {
        uint8_t game_over       : 1;    // Game kết thúc
        uint8_t paused          : 1;    // Tạm dừng
        uint8_t soft_dropping   : 1;    // Đang rơi nhanh
        uint8_t lock_active     : 1;    // Đang trong delay khóa
        uint8_t reserved        : 4;    // Dự phòng
    };
} GameFlags;

// Sự kiện từ mỗi tick
typedef struct {
    uint8_t lines_cleared;      // Số hàng vừa xóa (0-4)
    bool    block_spawned;      // Khối mới xuất hiện
    bool    block_locked;       // Khối vừa được khóa
    bool    game_over;          // Game kết thúc
    bool    level_up;           // Lên cấp
} GameTickEvent;

// Tóm tắt trạng thái game cho UI
typedef struct {
    GameStats stats;            // Thống kê
    GameFlags flags;            // Cờ trạng thái  
    BlockType next_block;       // Khối tiếp theo
} GameSummary;

/*  API chính  */

// Khởi tạo game engine
GameResult Game_Init(uint32_t seed);

// Đặt lại game về trạng thái ban đầu
GameResult Game_Reset(bool keep_stats);

// Tick chính - gọi ở tần số TICK_HZ
GameResult Game_Tick(GameTickEvent* event_out);

/*  Điều khiển người chơi  */

GameResult Game_MoveLeft(void);        // Di chuyển trái
GameResult Game_MoveRight(void);       // Di chuyển phải  
GameResult Game_SoftDrop(void);        // Rơi nhanh 1 bước
GameResult Game_HardDrop(uint8_t* drop_distance);  // Rơi xuống đáy ngay
GameResult Game_RotateCW(void);        // Xoay thuận chiều
GameResult Game_RotateCCW(void);       // Xoay ngược chiều
void Game_SetSoftDropActive(bool active);  // Bật/tắt chế độ rơi nhanh

/*  Truy vấn trạng thái (cho đồ họa)  */

// Lấy giá trị ô trên bàn
GameCell Game_GetBoardCell(int8_t x, int8_t y);

// Lấy khối đang rơi (chỉ đọc)
GameResult Game_GetCurrentBlock(FallingBlock* block_out);

// Lấy vị trí Y của bóng khối (preview cho hard drop)
int8_t Game_GetGhostY(void);

// Lấy tóm tắt game cho UI
GameResult Game_GetSummary(GameSummary* summary_out);

// Kiểm tra khối hiện tại có ở vị trí (x,y) không
bool Game_IsCurrentBlockAt(int8_t x, int8_t y);

/*  Điều khiển game  */

void Game_SetPaused(bool paused);      // Tạm dừng/tiếp tục
void Game_SetLevel(uint16_t level);    // Đặt cấp độ

/*  Hàm tiện ích (inline để tối ưu)  */

// Lấy bit từ shape mask 4x4
static inline bool Game_ShapeBit(uint16_t mask, uint8_t x, uint8_t y) {
    if (x >= 4 || y >= 4) return false;
    return (mask & (0x8000 >> (y * 4 + x))) != 0;
}

// Lấy màu khối cho đồ họa
static inline uint8_t Game_GetBlockColor(BlockType type) {
    return (type >= BLOCK_I && type <= BLOCK_PLUS) ? (uint8_t)type : 0;
}

// Kiểm tra tọa độ có trong biên không
static inline bool Game_InBounds(int8_t x, int8_t y) {
    return (x >= 0 && x < BOARD_W && y >= 0 && y < BOARD_H);
}

/* Dữ liệu hình dạng khối  */

// Lấy shape mask cho khối và hướng xoay
uint16_t Game_GetShapeMask(BlockType type, BlockRotation rotation);

// Kiểm tra khối có thể xoay không  
bool Game_CanRotate(BlockType type);

/*  Debug (chỉ khi GAME_DEBUG_ENABLED) */
#ifdef GAME_DEBUG_ENABLED
void Game_DebugSetCell(int8_t x, int8_t y, GameCell value);    // Đặt ô trực tiếp
void Game_DebugClear(void);                                     // Xóa toàn bộ bàn
GameResult Game_DebugSpawn(BlockType type);                     // Spawn khối cụ thể
#endif

/* Thông tin sử dụng bộ nhớ  */
#define GAME_RAM_USAGE (BOARD_W * BOARD_H + sizeof(FallingBlock) + 64)

// Cảnh báo nếu RAM > 512 bytes cho STM32F401RE
/*#if GAME_RAM_USAGE > 512
#warning "Game RAM usage > 512 bytes"
#endif
*/
#ifdef __cplusplus
}
#endif

#endif /* LOGIC_H */
