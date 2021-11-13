#include <cassert>
#include <iostream>
#include <random>
#include <tuple>

constexpr uint32_t map_side = (1 << 5);
constexpr uint32_t map_w = map_side;
constexpr uint32_t map_h = map_side;
constexpr char horiz_naught = ' ';
constexpr char horiz_line = '_';
constexpr char vert_naught = ',';
constexpr char vert_line = '|';

bool map_horiz[map_h][map_w - 1] = {};
bool map_vert[map_h - 1][map_w] = {};

void draw_map() {
    for (uint32_t h = 0; h < map_h; h++) {
        for (uint32_t w = 0; w < map_w; w++) {
            std::cout << (((h != 0) && map_vert[h - 1][w]) ? vert_line : vert_naught);
            std::cout << (((w != (map_w - 1)) && map_horiz[h][w]) ? horiz_line : horiz_naught);
        }
        std::cout << '\n';
    }
}

void deinterleave(uint32_t idx, uint16_t& a, uint16_t& b) {
    uint16_t x = 0;
    for (int i = 0; i < 16; i++) {
        int j = 2 * i;
        x = x | static_cast<uint16_t>((idx & (1 << j)) >> (j - i));
    }
    uint16_t y = 0;
    for (int i = 0; i < 16; i++) {
        int k = (2 * i) + 1;
        y = y | static_cast<uint16_t>((idx & (1 << k)) >> (k - i));
    }
    a = x;
    b = y;
}

std::tuple<uint16_t, uint16_t> spacefill_idx_to_coords(uint32_t idx) {
    uint16_t x;
    uint16_t y;
    deinterleave(idx, x, y);

    const uint16_t z = x ^ y;
    uint16_t z_out = 0;
    uint16_t y_out = 0;
    bool tsp = true;
    bool inv = false;
    for (uint16_t bit = static_cast<uint16_t>(1) << 15; bit != 0; bit >>= 1) {
        const uint16_t zb = z & bit;
        const uint16_t yb = y & bit;
        uint16_t zc = zb;
        uint16_t yc = yb;
        if (tsp) { std::swap(zc, yc); }
        if (inv) {
            zc ^= bit;
            yc ^= bit;
        }
        z_out |= zc;
        y_out |= yc;
        if (zb == 0) {
            tsp = !tsp;
            if (yb != 0) { inv = !inv; }
        }
    }

    return {z_out, y_out};
}

std::tuple<uint16_t, uint16_t> spacefill_idx_to_coords_alt(uint32_t idx) {
    uint16_t x;
    uint16_t y;
    deinterleave(idx, x, y);

    auto propagate = [](uint16_t val) -> uint16_t {
        uint16_t result = 0;
        for (uint16_t bit = static_cast<uint16_t>(1) << 15; bit != 0; bit >>= 1) {
            const uint16_t x = val & bit;
            result ^= (x == 0) ? 0 : (x - 1);
        }
        return result;
    };

    const uint16_t z = x ^ y;
    const uint16_t tsp = ~propagate(~z);
    const uint16_t inv = propagate(x & y);
    const uint16_t diff = (x & tsp) ^ inv;
    return {z ^ diff, y ^ diff};
}

int main() {
    std::ios::sync_with_stdio(false);
    auto last_coords = std::tuple<uint16_t, uint16_t>(0, 0);
    constexpr uint32_t point_cnt = map_w * map_h;
    for (uint32_t idx = 1; idx < point_cnt; idx++) {
        auto coords = spacefill_idx_to_coords(idx);
        auto x_min = std::min(std::get<0>(coords), std::get<0>(last_coords));
        auto x_max = std::max(std::get<0>(coords), std::get<0>(last_coords));
        auto y_min = std::min(std::get<1>(coords), std::get<1>(last_coords));
        auto y_max = std::max(std::get<1>(coords), std::get<1>(last_coords));
        auto dx = x_max - x_min;
        auto dy = y_max - y_min;
        assert((dx + dy) == 1);
        if (dx == 1) {
            if ((y_min < map_h) && ((x_min + 1) < map_w)) { map_horiz[y_min][x_min] = true; }
        } else {
            if (((y_min + 1) < map_h) && (x_min < map_w)) { map_vert[y_min][x_min] = true; }
        }
        auto coords_alt = spacefill_idx_to_coords_alt(idx);
        assert(coords == coords_alt);
        last_coords = coords;
    }

    draw_map();
}
