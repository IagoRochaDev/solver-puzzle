#pragma once

#include <vector>
#include <cmath>
#include <cstdlib>
#include "IHeuristic.hpp"

class Heuristics_1 : public IHeuristic {
private:
    std::vector<std::vector<int>> manhattan_table;
    int side_length;

    void precompute_table(int board_size) {
        side_length = static_cast<int>(std::sqrt(board_size));
        manhattan_table.assign(board_size, std::vector<int>(board_size, 0));

        for (int value = 1; value < board_size; ++value) {
            int target_pos = value - 1;
            int target_row = target_pos / side_length;
            int target_col = target_pos % side_length;

            for (int current_pos = 0; current_pos < board_size; ++current_pos) {
                int current_row = current_pos / side_length;
                int current_col = current_pos % side_length;

                manhattan_table[value][current_pos] = 
                    std::abs(target_row - current_row) + std::abs(target_col - current_col);
            }
        }
    }

public:
    Heuristics_1(int board_size = 9) {
        precompute_table(board_size);
    }

    int calculate(const std::vector<int>& board) const override{
        int total_distance = 0;
        for (int i = 0; i < board.size(); ++i) {
            int value = board[i];
            if (value != 0) {
                total_distance += manhattan_table[value][i];
            }
        }
        return total_distance;
    }
};