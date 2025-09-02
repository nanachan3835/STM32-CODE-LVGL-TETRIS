#include "logic.h"
#include<stddef.h>
/*  Biến trạng thái game  */
static GameCell s_board[BOARD_H][BOARD_W];     // Bàn chơi
static FallingBlock s_current;                 // Khối đang rơi
static BlockType s_next_block;                 // Khối tiếp theo
static GameStats s_stats;                      // Thống kê
static GameFlags s_flags;                      // Cờ trạng thái
static uint16_t s_tick_counter;                // Bộ đếm tick
static uint16_t s_gravity_ticks = GRAVITY_BASE_TICKS;  // Tốc độ rơi
static uint32_t s_rng_state;                   // Trạng thái RNG

/*  RNG đơn giản (xorshift32)  */
static uint32_t rng32(void) {
    uint32_t x = s_rng_state ? s_rng_state : 0xA5A5A5A5u;
    x ^= x << 13; x ^= x >> 17; x ^= x << 5;
    return s_rng_state = x;
}

static uint32_t rand_range(uint32_t max) {
    return (max == 0) ? 0 : (rng32() % max);
}

/* ====== Dữ liệu hình dạng khối (4x4 grid, MSB = góc trái-trên) ========== */

static const uint16_t SHAPE_I[4] = {
    0b1111000000000000,
    0b1000100010001000,
    0b1111000000000000,
    0b1000100010001000,
};

static const uint16_t SHAPE_L[4] = {
    0b1000100011000000,
    0b1110100000000000,
    0b1100010001000000,
    0b0010111000000000,
};

static const uint16_t SHAPE_O[4] = {
    0b1100110000000000,
    0b1100110000000000,
    0b1100110000000000,
    0b1100110000000000,
};

static const uint16_t SHAPE_T[4] = {
    0b1110010000000000,
    0b0100011001000000,
    0b0100111000000000,
    0b0100110001000000,
};

static const uint16_t SHAPE_U[4] = {
    0b1010111000000000,
    0b1100100011000000,
    0b1110101000000000,
    0b1100001011000000,
};

static const uint16_t SHAPE_PLUS[4] = {
    0b0100111001000000,
    0b0100111001000000,
    0b0100111001000000,
    0b0100111001000000,
};

// Bảng tra cứu hình dạng
static const uint16_t* SHAPE_TABLE[7] = {
    NULL, SHAPE_I, SHAPE_L, SHAPE_O, SHAPE_T, SHAPE_U, SHAPE_PLUS
};

/*  API hình dạng khối  */
bool Game_CanRotate(BlockType type) {
    return (type != BLOCK_O && type != BLOCK_PLUS);
}

uint16_t Game_GetShapeMask(BlockType type, BlockRotation rotation) {
    if (type < BLOCK_I || type > BLOCK_PLUS) return 0;
    return SHAPE_TABLE[type][rotation & 3];
}

/*  Phát hiện va chạm  */
static bool check_collision(int8_t offset_x, int8_t offset_y, uint16_t shape_mask) {
    for (uint8_t y = 0; y < 4; y++) {
        for (uint8_t x = 0; x < 4; x++) {
            if (!Game_ShapeBit(shape_mask, x, y)) continue;
            
            int8_t board_x = offset_x + x;
            int8_t board_y = offset_y + y;
            
            // Kiểm tra biên
            if (board_x < 0 || board_x >= BOARD_W || board_y >= BOARD_H) {
                return true;
            }
            
            // Kiểm tra va chạm với khối đã có
            if (board_y >= 0 && s_board[board_y][board_x] != 0) {
                return true;
            }
        }
    }
    return false;
}

/* Tính toán bóng khối  */
int8_t Game_GetGhostY(void) {
    if (s_flags.game_over || s_current.type == BLOCK_NONE) {
        return -1;
    }
    
    int8_t ghost_y = s_current.y;
    while (!check_collision(s_current.x, ghost_y + 1, s_current.shape_mask)) {
        ghost_y++;
    }
    return ghost_y;
}

/*  Đặt khối lên bàn  */
static void place_current_block(void) {
    for (uint8_t y = 0; y < 4; y++) {
        for (uint8_t x = 0; x < 4; x++) {
            if (!Game_ShapeBit(s_current.shape_mask, x, y)) continue;
            
            int8_t board_x = s_current.x + x;
            int8_t board_y = s_current.y + y;
            
            if (board_y >= 0 && board_y < BOARD_H && 
                board_x >= 0 && board_x < BOARD_W) {
                s_board[board_y][board_x] = (GameCell)s_current.type;
            }
        }
    }
    s_stats.blocks_placed++;
}

/*  Xóa hàng đầy và tính điểm  */
static uint8_t clear_completed_lines(void) {
    uint8_t lines_cleared = 0;
    
    // Quét từ dưới lên
    for (int y = BOARD_H - 1; y >= 0; y--) {
        bool is_full = true;
        
        // Kiểm tra hàng đầy
        for (int x = 0; x < BOARD_W; x++) {
            if (s_board[y][x] == 0) {
                is_full = false;
                break;
            }
        }
        
        if (is_full) {
            lines_cleared++;
            
            // Đẩy các hàng trên xuống
            for (int move_y = y; move_y > 0; move_y--) {
                for (int x = 0; x < BOARD_W; x++) {
                    s_board[move_y][x] = s_board[move_y - 1][x];
                }
            }
            
            // Xóa hàng trên cùng
            for (int x = 0; x < BOARD_W; x++) {
                s_board[0][x] = 0;
            }
            
            y++; // Kiểm tra lại cùng hàng sau khi đẩy
        }
    }
    
    // Cập nhật thống kê và điểm
    if (lines_cleared > 0) {
        s_stats.lines_total += lines_cleared;
        
        // Tính điểm: 1 hàng=100, 2=300, 3=500, 4=800
        uint32_t base_score;
        switch (lines_cleared) {
            case 1:  base_score = 100; break;
            case 2:  base_score = 300; break;
            case 3:  base_score = 500; break;
            case 4:  base_score = 800; break;
            default: base_score = 1000; break;
        }
        s_stats.score += base_score * (s_stats.level + 1);
    }
    
    return lines_cleared;
}

/*  Sinh khối ngẫu nhiên  */
static BlockType generate_random_block(void) {
    static const BlockType block_pool[6] = {
        BLOCK_I, BLOCK_L, BLOCK_O, BLOCK_T, BLOCK_U, BLOCK_PLUS
    };
    return block_pool[rand_range(6)];
}

static GameResult spawn_new_block(BlockType forced_type) {
    // Đặt loại khối
    s_current.type = (forced_type != BLOCK_NONE) ? forced_type : s_next_block;
    s_next_block = generate_random_block();
    
    // Khởi tạo vị trí và hướng
    s_current.rotation = ROT_0;
    s_current.shape_mask = Game_GetShapeMask(s_current.type, s_current.rotation);
    s_current.x = (BOARD_W - 4) / 2;  // Căn giữa
    s_current.y = -1;                 // Bắt đầu trên màn hình
    s_current.lock_timer = 0;
    
    // Kiểm tra game over
    if (check_collision(s_current.x, s_current.y, s_current.shape_mask)) {
        s_flags.game_over = 1;
        return GAME_GAME_OVER;
    }
    
    return GAME_OK;
}

/* Quản lý bộ đếm khóa khối  */
static void update_lock_timer(void) {
    bool at_bottom = check_collision(s_current.x, s_current.y + 1, s_current.shape_mask);
    
    if (at_bottom) {
        s_flags.lock_active = 1;
        if (s_current.lock_timer < LOCK_DELAY_TICKS) {
            s_current.lock_timer++;
        }
    } else {
        s_flags.lock_active = 0;
        s_current.lock_timer = 0;
    }
}

/*  Xử lý trọng lực  */
static bool handle_gravity(void) {
    s_tick_counter++;
    uint16_t gravity_speed = s_flags.soft_dropping ? 1 : s_gravity_ticks;
    
    if (s_tick_counter >= gravity_speed) {
        s_tick_counter = 0;
        
        if (!check_collision(s_current.x, s_current.y + 1, s_current.shape_mask)) {
            s_current.y++;
            s_current.lock_timer = 0;  // Reset timer khi còn rơi được
            return false;  // Chưa chạm đáy
        }
    }
    
    return check_collision(s_current.x, s_current.y + 1, s_current.shape_mask);
}

/*  Nâng cấp độ  */
static bool update_level(void) {
    uint16_t new_level = s_stats.lines_total / 10;  // Lên cấp mỗi 10 hàng
    bool level_up = (new_level != s_stats.level);
    
    if (level_up) {
        s_stats.level = new_level;
        // Tăng tốc độ rơi (tối thiểu 2 ticks)
        s_gravity_ticks = (GRAVITY_BASE_TICKS > s_stats.level * 2) ? 
                         (GRAVITY_BASE_TICKS - s_stats.level * 2) : 2;
    }
    
    return level_up;
}

/*  API công khai  */

GameResult Game_Init(uint32_t seed) {
    // Khởi tạo RNG
    s_rng_state = seed ? seed : 0xC0FFEEu;
    
    // Xóa bàn chơi
    for (int y = 0; y < BOARD_H; y++) {
        for (int x = 0; x < BOARD_W; x++) {
            s_board[y][x] = 0;
        }
    }
    
    // Khởi tạo trạng thái game
    s_stats = (GameStats){0};
    s_flags = (GameFlags){0};
    s_tick_counter = 0;
    s_gravity_ticks = GRAVITY_BASE_TICKS;
    
    // Tạo khối đầu tiên
    s_next_block = generate_random_block();
    s_current.type = BLOCK_NONE;
    
    return spawn_new_block(BLOCK_NONE);
}

GameResult Game_Reset(bool keep_stats) {
    GameStats saved_stats = s_stats;
    uint32_t saved_rng = s_rng_state;
    
    Game_Init(saved_rng);
    
    if (keep_stats) {
        s_stats = saved_stats;
    }
    
    return GAME_OK;
}

GameResult Game_Tick(GameTickEvent* event_out) {
    // Xóa sự kiện
    if (event_out) {
        *event_out = (GameTickEvent){0};
    }
    
    // Bỏ qua nếu tạm dừng hoặc game over
    if (s_flags.paused || s_flags.game_over) {
        return s_flags.game_over ? GAME_GAME_OVER : GAME_OK;
    }
    
    // Xử lý trọng lực và kiểm tra chạm đáy
    bool at_bottom = handle_gravity();
    
    // Cập nhật bộ đếm khóa
    update_lock_timer();
    
    // Kiểm tra có nên khóa khối không
    if (at_bottom && s_current.lock_timer >= LOCK_DELAY_TICKS) {
        // Đặt khối lên bàn
        place_current_block();
        if (event_out) event_out->block_locked = true;
        
        // Xóa hàng đầy
        uint8_t lines_cleared = clear_completed_lines();
        if (event_out) event_out->lines_cleared = lines_cleared;
        
        // Kiểm tra lên cấp
        bool level_up = update_level();
        if (event_out) event_out->level_up = level_up;
        
        // Sinh khối mới
        if (spawn_new_block(BLOCK_NONE) == GAME_GAME_OVER) {
            if (event_out) event_out->game_over = true;
            return GAME_GAME_OVER;
        }
        
        if (event_out) event_out->block_spawned = true;
    }
    
    return GAME_OK;
}

/*  Điều khiển người chơi  */

GameResult Game_MoveLeft(void) {
    if (s_flags.game_over) return GAME_GAME_OVER;
    
    if (!check_collision(s_current.x - 1, s_current.y, s_current.shape_mask)) {
        s_current.x--;
        s_current.lock_timer = 0;  // Reset delay khi di chuyển
        return GAME_OK;
    }
    
    return GAME_COLLISION;
}

GameResult Game_MoveRight(void) {
    if (s_flags.game_over) return GAME_GAME_OVER;
    
    if (!check_collision(s_current.x + 1, s_current.y, s_current.shape_mask)) {
        s_current.x++;
        s_current.lock_timer = 0;  // Reset delay khi di chuyển
        return GAME_OK;
    }
    
    return GAME_COLLISION;
}

GameResult Game_SoftDrop(void) {
    if (s_flags.game_over) return GAME_GAME_OVER;
    
    if (!check_collision(s_current.x, s_current.y + 1, s_current.shape_mask)) {
        s_current.y++;
        s_stats.score += 1;  // Điểm thưởng rơi nhanh
        return GAME_OK;
    }
    
    return GAME_COLLISION;
}

GameResult Game_HardDrop(uint8_t* drop_distance) {
    if (s_flags.game_over) return GAME_GAME_OVER;
    
    uint8_t distance = 0;
    while (!check_collision(s_current.x, s_current.y + 1, s_current.shape_mask)) {
        s_current.y++;
        distance++;
    }
    
    if (drop_distance) *drop_distance = distance;
    
    // Điểm thưởng rơi cứng
    s_stats.score += 2 * distance;
    
    // Buộc khóa ngay lập tức
    s_current.lock_timer = LOCK_DELAY_TICKS;
    
    return GAME_OK;
}

static GameResult try_rotation(int rotation_delta) {
    if (s_flags.game_over) return GAME_GAME_OVER;
    if (!Game_CanRotate(s_current.type)) return GAME_OK;
    
    BlockRotation new_rotation = (BlockRotation)((s_current.rotation + rotation_delta) & 3);
    uint16_t new_shape = Game_GetShapeMask(s_current.type, new_rotation);
    
    // Thử xoay cơ bản trước
    if (!check_collision(s_current.x, s_current.y, new_shape)) {
        s_current.rotation = new_rotation;
        s_current.shape_mask = new_shape;
        s_current.lock_timer = 0;  // Reset delay khi xoay
        return GAME_OK;
    }
    
    // Thử wall kicks (đơn giản)
    const int8_t wall_kicks[][2] = {{-1, 0}, {1, 0}, {0, -1}, {-1, -1}, {1, -1}};
    
    for (unsigned i = 0; i < sizeof(wall_kicks) / sizeof(wall_kicks[0]); i++) {
        int8_t kick_x = s_current.x + wall_kicks[i][0];
        int8_t kick_y = s_current.y + wall_kicks[i][1];
        
        if (!check_collision(kick_x, kick_y, new_shape)) {
            s_current.x = kick_x;
            s_current.y = kick_y;
            s_current.rotation = new_rotation;
            s_current.shape_mask = new_shape;
            s_current.lock_timer = 0;
            return GAME_OK;
        }
    }
    
    return GAME_COLLISION;
}

GameResult Game_RotateCW(void) {
    return try_rotation(1);
}

GameResult Game_RotateCCW(void) {
    return try_rotation(3);  // +3 tương đương -1 trong mod 4
}

void Game_SetSoftDropActive(bool active) {
    s_flags.soft_dropping = active ? 1 : 0;
}

void Game_SetPaused(bool paused) {
    s_flags.paused = paused ? 1 : 0;
}

void Game_SetLevel(uint16_t level) {
    s_stats.level = level;
    s_gravity_ticks = (GRAVITY_BASE_TICKS > level * 2) ? 
                     (GRAVITY_BASE_TICKS - level * 2) : 2;
}

/*  Truy vấn trạng thái  */

GameCell Game_GetBoardCell(int8_t x, int8_t y) {
    return Game_InBounds(x, y) ? s_board[y][x] : 0;
}

GameResult Game_GetCurrentBlock(FallingBlock* block_out) {
    if (!block_out || s_current.type == BLOCK_NONE) {
        return GAME_INVALID_BLOCK;
    }
    
    *block_out = s_current;
    return GAME_OK;
}

bool Game_IsCurrentBlockAt(int8_t x, int8_t y) {
    if (s_current.type == BLOCK_NONE) return false;
    
    for (uint8_t block_y = 0; block_y < 4; block_y++) {
        for (uint8_t block_x = 0; block_x < 4; block_x++) {
            if (Game_ShapeBit(s_current.shape_mask, block_x, block_y)) {
                if (s_current.x + block_x == x && s_current.y + block_y == y) {
                    return true;
                }
            }
        }
    }
    
    return false;
}

GameResult Game_GetSummary(GameSummary* summary_out) {
    if (!summary_out) return GAME_INVALID_BLOCK;
    
    summary_out->stats = s_stats;
    summary_out->flags = s_flags;
    summary_out->next_block = s_next_block;
    
    return GAME_OK;
}

/*  Hỗ trợ debug  */

#ifdef GAME_DEBUG_ENABLED
void Game_DebugSetCell(int8_t x, int8_t y, GameCell value) {
    if (Game_InBounds(x, y)) {
        s_board[y][x] = value;
    }
}

void Game_DebugClear(void) {
    for (int y = 0; y < BOARD_H; y++) {
        for (int x = 0; x < BOARD_W; x++) {
            s_board[y][x] = 0;
        }
    }
}

GameResult Game_DebugSpawn(BlockType type) {
    return spawn_new_block(type);
}
#endif
