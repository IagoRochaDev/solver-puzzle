#include <iostream>
#include <vector>
#include <fstream>
#include <random>

#define NUM_INSTANCES 100
#define NUM_MOVES_INF 1000000
#define NUM_MOVES_SUP 10000000


// Representacao dos movimentos (diferenca no indice do vetor 1D)
enum Move { UP = -4, DOWN = 4, LEFT = -1, RIGHT = 1, NONE = 0 };

// Retorna os movimentos possiveis baseados na posicao do 0
std::vector<Move> get_valid_moves(int zero_index) {
    std::vector<Move> moves;
    int row = zero_index / 4;
    int col = zero_index % 4;

    if (row > 0) moves.push_back(UP);
    if (row < 3) moves.push_back(DOWN);
    if (col > 0) moves.push_back(LEFT);
    if (col < 3) moves.push_back(RIGHT);

    return moves;
}

// Retorna o movimento oposto para evitar ir e voltar no mesmo lugar
Move get_opposite(Move m) {
    if (m == UP) return DOWN;
    if (m == DOWN) return UP;
    if (m == LEFT) return RIGHT;
    if (m == RIGHT) return LEFT;
    return NONE;
}

int main() {
    // Configuracoes
    const int num_instances = NUM_INSTANCES; // Quantas instancias voce quer gerar?
    const std::vector<int> goal_state = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0};

    // Preparacao dos arquivos de saida
    std::ofstream f_entradas("15puzzle_instances_valled.txt");

    if (!f_entradas.is_open()) {
        std::cerr << "Erro ao criar os arquivos de saida!\n";
        return 1;
    }

    // Preparacao do gerador de numeros aleatorios
    std::random_device rd;
    std::mt19937 rng(rd());
    std::uniform_int_distribution<int> dist_moves(NUM_MOVES_INF, NUM_MOVES_SUP);

    std::cout << "Gerando " << num_instances << " instancias validas...\n";

    for (int i = 0; i < num_instances; i++) {
        std::vector<int> current_board = goal_state;
        int zero_index = 15; // O 0 comeca na ultima posicao
        
        // Sorteia quantos movimentos serao feitos (0 a 100)
        int steps = dist_moves(rng);
        Move last_move = NONE;

        // Embaralha o tabuleiro
        for (int step = 0; step < steps; step++) {
            std::vector<Move> valid_moves = get_valid_moves(zero_index);
            std::vector<Move> safe_moves;

            // Filtra os movimentos para nao desfazer o passo anterior
            for (Move m : valid_moves) {
                if (m != get_opposite(last_move)) {
                    safe_moves.push_back(m);
                }
            }

            // Se por acaso sobrar nenhum movimento (teoricamente impossivel aqui), reseta
            if (safe_moves.empty()) safe_moves = valid_moves;

            // Escolhe um movimento aleatorio entre os seguros
            std::uniform_int_distribution<int> dist_choice(0, safe_moves.size() - 1);
            Move chosen_move = safe_moves[dist_choice(rng)];

            // Aplica o movimento
            int new_zero_index = zero_index + chosen_move;
            std::swap(current_board[zero_index], current_board[new_zero_index]);
            
            // Atualiza variaveis para o proximo passo
            zero_index = new_zero_index;
            last_move = chosen_move;
        }

        // Salva o tabuleiro no arquivo de entradas (16 numeros separados por espaco)
        for (int j = 0; j < 16; j++) {
            f_entradas << current_board[j] << (j == 15 ? "" : " ");
        }
        f_entradas << "\n";
    }

    f_entradas.close();


    std::cout << "Concluido! Arquivos '15puzzle_instances_valled.txt' criado.\n";
    return 0;
}