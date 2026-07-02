#include "bitmap.h"
#include "AVAILABLE_PIECES.h"
#include "precomputations.h"

#include <array>
#include <span>
#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <unordered_map>
#include <exception>
#include <ranges>
#include <iostream>
#include <vector>
#include <bitset>
#include <set>
#include <map>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<uint64_t> g_nodes{0}; // total solve() nodes visited
std::atomic<uint64_t> g_top_done{0}; // top-level branches completeds
std::atomic<uint64_t> g_top_total{0}; // top-level branches in total
std::atomic<bool>     g_running{true};
std::array<std::atomic<uint64_t>, AVAILABLE_PIECES.data.size() + 1> g_depth_nodes{};    // nodes visited per level
std::array<std::atomic<uint64_t>, AVAILABLE_PIECES.data.size() + 1> g_depth_children{}; // placements generated per level

void progress_reporter() {
    using namespace std::chrono;
    const auto start = steady_clock::now();
    uint64_t last_nodes = 0;
    auto last_time = start;

    while (g_running.load(std::memory_order_relaxed)) {
        std::this_thread::sleep_for(milliseconds(500));

        const auto now = steady_clock::now();
        const uint64_t nodes = g_nodes.load(std::memory_order_relaxed);

        const double   interval    = duration<double>(now - last_time).count();
        const double   total       = duration<double>(now - start).count();
        const uint64_t delta_nodes = nodes - last_nodes;

        const double inst_rate = delta_nodes / interval; // nodes/s
        const double avg_rate  = nodes / total;
        const double inst_us   = delta_nodes ? interval / delta_nodes * 1e6 // micro seconds /node
                                             : 0.0;
        const double avg_us    = nodes ? total / nodes * 1e6 : 0.0;

        last_nodes = nodes;
        last_time  = now;

        const auto secs = duration_cast<seconds>(now - start);
        std::cerr << "\r[" << g_top_done.load() << "/" << g_top_total.load()
                  << " top branches]  nodes=" << nodes
                  << "  " << std::format("{:.3}", inst_rate / 1e6) << "M nodes/s"
                  << " (avg " << std::format("{:.3}", avg_rate / 1e6) << "M)"
                  << "  " << std::format("{:.2f}", inst_us) << " us/node"
                  << " (avg " << std::format("{:.2f}", avg_us) << "u)"
                  << "  elapsed="
                  << [&]() {
                       std::chrono::hh_mm_ss hms{secs};
                       return std::format("{:02}:{:02}:{:02}",
                           hms.hours().count(), hms.minutes().count(), hms.seconds().count());
                     }()
                  << "   " << std::flush;
    }
}

size_t enclosed_area(const bitmap& m) {

    bitmap border { (ROW_MASKS[0] | COL_MASKS[0] 
                | ROW_MASKS[GRID_SIZE - 1] | COL_MASKS[GRID_SIZE - 1]) 
                & ~m};

    bitmap previous{}; // default false
    while (previous != border) {
        previous = border;
        border.expand(NBH4);
        border &= ~m;
    }
    return GRID_SIZE * GRID_SIZE - border.count() - m.count();
}

// precondition: all pieces are normalized
// postcondition: all placements are unique
std::vector<bitmap> generate_all_placements(std::span<const bitmap> pieces, const bitmap& frontier, const bitmap& occupied) {

    std::vector<bitmap> placements{};

    const auto valid_bits {frontier.get_valid_bits()};

    for (auto [frontier_c, frontier_r] : valid_bits) {

        for (const auto& raw_piece : pieces) {

            const auto orientations { ORIENTATIONS_PRECOMPUTED.at(raw_piece) };

            for (const auto& piece : orientations) {

                auto [max_col, max_row] { MAX_CORNER_PRECOMPUTED.at(piece) };

                const auto piece_indices { VALID_BITS_PRECOMPUTED.at(piece) };

                for (auto [piece_c, piece_r] : piece_indices) {

                    // require min_r + r > 0 --> r > 0
                    // require min_c + c > 0 --> c > 0
                    if (frontier_r < piece_r || frontier_c < piece_c) { continue; }
                    
                    auto r {frontier_r - piece_r};
                    auto c {frontier_c - piece_c};

                    // require max_r + r < GRID_SIZE 
                    // require min_c + c < GRID_SIZE
                    if (r + max_row >= GRID_SIZE || c + max_col >= GRID_SIZE) { continue; }

                    auto placed = piece;
                    placed.move(c, r);
                    if ((occupied & placed) == bitmap{}) { 
                        const auto placement { occupied | placed };
                        if (std::ranges::find(placements, placement) == placements.end()) {
                            placements.push_back(placement);
                        }
                    }
                }
            }
        }
    }
    return placements;
}

std::vector<bitmap> solve(const bitmap& grid, util::fast_map<bitmap, size_t, NUM_PIECES>& pieces, int depth = AVAILABLE_PIECES.data.size()) { 

    g_nodes.fetch_add(1, std::memory_order_relaxed);
    const size_t level = AVAILABLE_PIECES.data.size() - depth;
    g_depth_nodes[level].fetch_add(1, std::memory_order_relaxed);

    if (depth == 0) { return { grid }; }

    auto piece_masks { 
        AVAILABLE_PIECES
        | std::views::filter([](const auto& pair) { return pair.second >= 1; })
        | std::views::keys
        | std::ranges::to<std::vector<bitmap>>() };

    std::vector<bitmap> placements{};
    if (grid == bitmap{}) {
        placements = generate_all_placements(piece_masks, TOP_LEFT, bitmap{});
    }
    else {
        placements = generate_all_placements(piece_masks, grid.get_frontier(NBH8), grid);
    }

    // Record the size of the top-level fan-out so the reporter can show n/total.
    const bool is_top = (depth == AVAILABLE_PIECES.data.size());
    if (is_top) { g_top_total.store(placements.size(), std::memory_order_relaxed); }

    // Also record each level
    g_depth_children[level].fetch_add(placements.size(), std::memory_order_relaxed);

    bitmap max_mask {grid};
    size_t max_area{0};
    std::vector<bitmap> max_solutions{};

    for (auto placement : placements) {

        auto played_piece {placement & ~grid};
        played_piece.normalize();
        played_piece = TO_CANONICAL_PRECOMPUTED.at(played_piece);

        pieces.at(played_piece)--;

        std::vector<bitmap> solutions { solve(placement, pieces, depth - 1)} ;

        for (const auto& solution : solutions) {
            size_t area { enclosed_area(solution) };
            if (area > max_area) {
                max_area = area;
                max_solutions.clear();
                max_solutions.push_back(solution);
            }
            else if (area == max_area) {
                max_solutions.push_back(solution);
            }
        }
        
        pieces.at(played_piece)++;

        if (is_top) { g_top_done.fetch_add(1, std::memory_order_relaxed); }
    }
    return max_solutions;
}

int main() {
    
    bitmap grid{};

    std::cout << "ALL PIECES:\n";
    for (const auto& [piece, ct] : AVAILABLE_PIECES) {
        std::cout << "Count: " << ct << '\n';
        piece.print();
        std::cout << '\n';
    }

    std::cout << "\n\n";

    std::thread reporter(progress_reporter);
    auto pieces {AVAILABLE_PIECES};
    std::vector<bitmap> solutions { solve(grid, pieces) };
    g_running.store(false, std::memory_order_relaxed);
    reporter.join();

    std::cout << '\n';

    std::vector<bitmap> distinct{};
    for (auto& sol : solutions) {
        sol.canonicalize();
        if (std::ranges::find(distinct, sol) == distinct.end()) { distinct.push_back(sol); }
    }

    std::cout << "Total distinct solutions found: " << distinct.size() << '\n';
    for (size_t idx{1uz}; const auto& sol : distinct) {
        std::cout << "Solution #" << idx << '\n';
        sol.print();
        std::cout << '\n';
        idx++;
    }

    std::cout << "\n=== Branching by depth ===\n"
            << std::format("{:>5}  {:>14}  {:>16}  {:>10}\n",
                            "level", "nodes", "children", "avg branch");
    for (size_t lvl{}; lvl <= AVAILABLE_PIECES.data.size(); lvl++) {
        const uint64_t n { g_depth_nodes[lvl].load() };
        const uint64_t c { g_depth_children[lvl].load() };
        if (n == 0) { continue; }
        std::cout << std::format("{:>5}  {:>14}  {:>16}  {:>10.2f}\n",
                                lvl, n, c, n ? double(c) / n : 0.0);
    }
}