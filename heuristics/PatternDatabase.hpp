#pragma once
#include <vector>
#include <array>
#include <deque>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdint>

class PDB {
private:
    std::vector<int> pattern_tiles;
    std::vector<uint8_t> table;
    uint64_t table_size;
    std::string filename;
    int k_tokens; 

    
    uint64_t P(int n, int r) const {
        uint64_t res = 1;
        for(int i = 0; i < r; ++i) res *= (n - i);
        return res;
    }

    
    uint64_t rank_state(const std::array<int8_t, 16>& board) const {
        int8_t pos_map[16];
        for (int i = 0; i < 16; ++i) {
            pos_map[board[i]] = i;
        }

        uint64_t index = 0;
        uint16_t mask = 0;
        
        for(int i = 0; i < k_tokens; ++i) {
            
            int pos = (i < k_tokens - 1) ? pos_map[pattern_tiles[i]] : pos_map[0];
            int used_less = __builtin_popcount(mask & ((1 << pos) - 1));
            int count = pos - used_less;
            index += count * P(16 - 1 - i, k_tokens - 1 - i);
            mask |= (1 << pos);
        }
        return index;
    }

    
    void generate() {
        std::cout << "Gerando PDB (" << filename << ")... Aguarde alguns segundos.\n";
        std::array<int8_t, 16> goal_board;
        for(int i = 0; i < 15; ++i) goal_board[i] = i + 1;
        goal_board[15] = 0;

        std::deque<std::array<int8_t, 16>> q;
        table.assign(table_size, 255);

        uint64_t goal_rank = rank_state(goal_board);
        table[goal_rank] = 0;
        q.push_back(goal_board);

        int moves[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};

        while(!q.empty()) {
            auto curr_board = q.front();
            q.pop_front();

            uint64_t curr_rank = rank_state(curr_board);
            int curr_cost = table[curr_rank];

            int blank_idx = 0;
            for(int i=0; i<16; ++i) {
                if(curr_board[i] == 0) { blank_idx = i; break; }
            }
            int r = blank_idx / 4;
            int c = blank_idx % 4;

            for(int i=0; i<4; ++i) {
                int nr = r + moves[i][0];
                int nc = c + moves[i][1];
                if(nr >= 0 && nr < 4 && nc >= 0 && nc < 4) {
                    int next_idx = nr * 4 + nc;
                    
                    auto next_board = curr_board;
                    std::swap(next_board[blank_idx], next_board[next_idx]);

                    
                    int target_tile = curr_board[next_idx]; 
                    bool is_pattern = false;
                    for(int t : pattern_tiles) {
                        if(t == target_tile) { is_pattern = true; break; }
                    }
                    int move_cost = is_pattern ? 1 : 0;
                    int next_cost = curr_cost + move_cost;

                    uint64_t next_rank = rank_state(next_board);
                    if(next_cost < table[next_rank]) {
                        table[next_rank] = next_cost;
                        if(move_cost == 0) {
                            q.push_front(next_board); 
                        } else {
                            q.push_back(next_board);
                        }
                    }
                }
            }
        }

        
        std::ofstream out(filename, std::ios::binary);
        out.write(reinterpret_cast<char*>(table.data()), table_size);
        std::cout << "Arquivo " << filename << " gravado com sucesso.\n";
    }

public:
    PDB(const std::vector<int>& tiles, const std::string& file) 
        : pattern_tiles(tiles), filename(file) {
        k_tokens = pattern_tiles.size() + 1;
        table_size = P(16, k_tokens);

        std::ifstream in(filename, std::ios::binary);
        if(in) {
            table.resize(table_size);
            in.read(reinterpret_cast<char*>(table.data()), table_size);
            std::cout << "PDB '" << filename << "' carregado do disco (" << table_size / 1024 << " KB).\n";
        } else {
            generate();
        }
    }

    int get_cost(const std::vector<int>& board) const {
        std::array<int8_t, 16> arr;
        for(int i=0; i<16; ++i) arr[i] = board[i];
        return table[rank_state(arr)];
    }
};