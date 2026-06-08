#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <string>

#define NUM_INSTANCES 5
#define NUM_MOVES_INF 5
#define NUM_MOVES_SUP 20

#define PUZZLE_N 5 
#define BOARD_AREA (PUZZLE_N * PUZZLE_N)

enum Move { UP = -PUZZLE_N, DOWN = PUZZLE_N, LEFT = -1, RIGHT = 1, NONE = 0 };

std::vector<Move> get_valid_moves(int zero_index) {
    std::vector<Move> moves;
    int row = zero_index / PUZZLE_N;
    int col = zero_index % PUZZLE_N;

    if (row > 0) moves.push_back(UP);
    if (row < PUZZLE_N - 1) moves.push_back(DOWN);
    if (col > 0) moves.push_back(LEFT);
    if (col < PUZZLE_N - 1) moves.push_back(RIGHT);

    return moves;
}

Move get_opposite(Move m) {
    if (m == UP) return DOWN;
    if (m == DOWN) return UP;
    if (m == LEFT) return RIGHT;
    if (m == RIGHT) return LEFT;
    return NONE;
}

int main() {
    const int num_instances = NUM_INSTANCES; 
    std::vector<int> goal_state(BOARD_AREA);
    for(int i = 0; i < BOARD_AREA - 1; i++) {
        goal_state[i] = i + 1;
    }
    goal_state[BOARD_AREA - 1] = 0;

    std::string filename = std::to_string(BOARD_AREA - 1) + "puzzle_instances_valled.txt";
    std::ofstream f_entradas(filename);

    if (!f_entradas.is_open()) {
        std::cerr << "Erro ao criar o arquivo de saida!\n";
        return 1;
    }

    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist_moves(NUM_MOVES_INF, NUM_MOVES_SUP);

    std::cout << "Gerando " << num_instances << " instancias validas para o " 
              << (BOARD_AREA - 1) << "-puzzle...\n";

    for (int i = 0; i < num_instances; i++) {
        std::vector<int> current_board = goal_state;
        int zero_index = BOARD_AREA - 1;
        int steps = dist_moves(rng);
        Move last_move = NONE;

        for (int step = 0; step < steps; step++) {
            std::vector<Move> valid_moves = get_valid_moves(zero_index);
            std::vector<Move> safe_moves;

            for (Move m : valid_moves) {
                if (m != get_opposite(last_move)) {
                    safe_moves.push_back(m);
                }
            }

            if (safe_moves.empty()) safe_moves = valid_moves;
            std::uniform_int_distribution<int> dist_choice(0, safe_moves.size() - 1);
            Move chosen_move = safe_moves[dist_choice(rng)];
            int new_zero_index = zero_index + chosen_move;
            std::swap(current_board[zero_index], current_board[new_zero_index]);
            zero_index = new_zero_index;
            last_move = chosen_move;
        }
        for (int j = 0; j < BOARD_AREA; j++) {
            f_entradas << current_board[j] << (j == BOARD_AREA - 1 ? "" : " ");
        }
        f_entradas << "\n";
    }

    f_entradas.close();

    std::cout << "Concluido! Arquivo '" << filename << "' criado com sucesso.\n";
    return 0;
}