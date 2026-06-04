#pragma once

#include <vector>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <cstdint>
#include <algorithm>
#include "IHeuristic.hpp"


class WalkingDistanceDB {
private:
    std::unordered_map<uint64_t, int> distance_table;

    
    uint64_t encode(const std::vector<int>& matrix, int blank_line) const {
        uint64_t key = 0;
        for (int val : matrix) {
            key = (key << 3) | (val & 0x7); 
        }
        key = (key << 2) | (blank_line & 0x3); 
        return key;
    }

    
    void precompute() {
        
        std::vector<int> goal_matrix(16, 0);
        for (int i = 0; i < 4; ++i) {
            goal_matrix[i * 4 + i] = 4;
        }
        int goal_blank_line = 3; 

        uint64_t goal_key = encode(goal_matrix, goal_blank_line);
        distance_table[goal_key] = 0;

        struct QueueNode {
            std::vector<int> matrix;
            int blank_line;
            int dist;
        };

        std::queue<QueueNode> q;
        q.push({goal_matrix, goal_blank_line, 0});

        while (!q.empty()) {
            QueueNode curr = q.front();
            q.pop();

            
            int shifts[] = {-1, 1};
            for (int shift : shifts) {
                int next_blank = curr.blank_line + shift;
                
                if (next_blank >= 0 && next_blank < 4) {
                    
                    
                    for (int g = 0; g < 4; ++g) {
                        if (curr.matrix[next_blank * 4 + g] > 0) {
                            std::vector<int> next_matrix = curr.matrix;

                            
                            next_matrix[next_blank * 4 + g]--;
                            next_matrix[curr.blank_line * 4 + g]++;
                            
                            next_matrix[curr.blank_line * 4 + 3]--;
                            next_matrix[next_blank * 4 + 3]++;

                            uint64_t next_key = encode(next_matrix, next_blank);
                            
                            if (distance_table.find(next_key) == distance_table.end()) {
                                distance_table[next_key] = curr.dist + 1;
                                q.push({next_matrix, next_blank, curr.dist + 1});
                            }
                        }
                    }
                }
            }
        }
    }

public:
    WalkingDistanceDB() {
        precompute();
    }

    int get_distance(uint64_t key) const {
        auto it = distance_table.find(key);
        if (it != distance_table.end()) {
            return it->second;
        }
        return 99; 
    }

    uint64_t get_encoded_key(const std::vector<int>& matrix, int blank_line) const {
        return encode(matrix, blank_line);
    }

    
    static const WalkingDistanceDB& get_instance() {
        static WalkingDistanceDB instance;
        return instance;
    }
};


class Heuristics_3 : public IHeuristic {
private:
    int board_size;
    int side_length;

public:
    Heuristics_3(int size = 16) {
        board_size = size;
        side_length = static_cast<int>(std::sqrt(board_size));
        
        WalkingDistanceDB::get_instance();
    }

    int calculate(const std::vector<int>& board) const override{
        
        const auto& db = WalkingDistanceDB::get_instance();

        
        std::vector<int> row_matrix(16, 0);
        std::vector<int> col_matrix(16, 0);
        
        int blank_row = 0;
        int blank_col = 0;

        
        for (int i = 0; i < board_size; ++i) {
            int val = board[i];
            int curr_row = i / side_length;
            int curr_col = i % side_length;

            int goal_row, goal_col;
            if (val == 0) {
                blank_row = curr_row;
                blank_col = curr_col;
                goal_row = 3;
                goal_col = 3;
            } else {
                goal_row = (val - 1) / side_length;
                goal_col = (val - 1) % side_length;
            }

            row_matrix[curr_row * side_length + goal_row]++;
            col_matrix[curr_col * side_length + goal_col]++;
        }

        
        uint64_t row_key = db.get_encoded_key(row_matrix, blank_row);
        uint64_t col_key = db.get_encoded_key(col_matrix, blank_col);

        int wd_row = db.get_distance(row_key);
        int wd_col = db.get_distance(col_key);

        return wd_row + wd_col;
    }
};