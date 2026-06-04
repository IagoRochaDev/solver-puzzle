#include <vector>
#include <iostream>
#include <cmath>
#include <memory>
#include <algorithm>

class State {
private:
    std::vector<int> board; 
    int blank_pos;          
    int g_cost;             
    int h_cost;             
    int side_length;        
    State *parent;

public:
    State(const std::vector<int>& initial_board, int g = 0, State *parent_state = nullptr) {
        board = initial_board;
        g_cost = g;
        h_cost = 0;
        parent = parent_state;
        side_length = (int)(std::sqrt(board.size()));

        for (int i = 0; i < board.size(); ++i) {
            if (board[i] == 0) {
                blank_pos = i;
                break;
            }
        }
    }

    const std::vector<int>& get_board() const { return board; }
    int get_g() const { return g_cost; }
    int get_h() const { return h_cost; }
    int get_f() const { return g_cost + h_cost; }
    int get_blank_pos() const { return blank_pos; }
    State *get_parent() const { return parent; }
    
    void set_h(int h) { h_cost = h; }

    bool is_goal() const {
        for (size_t i = 0; i < board.size() - 1; ++i) {
            if (board[i] != i + 1) return false;
        }
        return board.back() == 0;
    }

    std::vector<State*> generate_successors(State *current_ptr) const {
        std::vector<State*> successors;
        
        int row = blank_pos / side_length;
        int col = blank_pos % side_length;

        int moves[4] = {
            blank_pos - side_length,
            blank_pos + side_length, 
            blank_pos - 1,           
            blank_pos + 1            
        };

        bool valid_moves[4] = {
            row > 0,                   
            row < side_length - 1,
            col > 0,             
            col < side_length - 1 
        };

        for (int i = 0; i < 4; ++i) {
            if (valid_moves[i]) {
                std::vector<int> new_board = board;
                std::swap(new_board[blank_pos], new_board[moves[i]]); 
                State* child = new State(new_board, g_cost + 1, current_ptr);
                successors.push_back(child);
            }
        }

        return successors;
    }

    void print() const {
        for (int i = 0; i < board.size(); ++i) {
            std::cout << board[i] << "\t";
            if ((i + 1) % side_length == 0) {
                std::cout << "\n";
            }
        }
        std::cout << "\n";
    }

    bool operator<(const State& other) const { return this->board < other.board; }
    bool operator==(const State& other) const { return this->board == other.board; }

    uint64_t get_hash() const {
        uint64_t hash = 0;
        for (int val : board) {
            hash = (hash << 4) | val; 
        }
        return hash;
    }
};

struct CompareF {
    bool operator()(const State *a, const State *b) const {
        return a->get_f() > b->get_f(); 
    }
};

struct CompareBoardPtr {
    bool operator()(const State *a, const State *b) const {
        return a->get_board() < b->get_board();
    }
};