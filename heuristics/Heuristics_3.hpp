#pragma once

#include <vector>
#include <cmath>
#include <queue>
#include <unordered_map>
#include <cstdint>
#include <algorithm>
#include "IHeuristic.hpp"

// --- BANCO DE DADOS DA DISTÂNCIA DE CAMINHADA ---
class WalkingDistanceDB {
private:
    std::unordered_map<uint64_t, int> distance_table;

    // Compacta a matriz 4x4 e a posição do espaço em branco em um único inteiro de 64 bits
    uint64_t encode(const std::vector<int>& matrix, int blank_line) const {
        uint64_t key = 0;
        for (int val : matrix) {
            key = (key << 3) | (val & 0x7); // Cada contagem vai de 0 a 4 (cabe em 3 bits)
        }
        key = (key << 2) | (blank_line & 0x3); // Posição da linha/coluna do zero (cabe em 2 bits)
        return key;
    }

    // Pré-calcula todas as configurações possíveis de 1D usando BFS
    void precompute() {
        // Matriz objetivo em 1D: Na linha i, todas as 4 peças pertencem à linha i (Identidade)
        std::vector<int> goal_matrix(16, 0);
        for (int i = 0; i < 4; ++i) {
            goal_matrix[i * 4 + i] = 4;
        }
        int goal_blank_line = 3; // O zero termina na última linha/coluna

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

            // O espaço vazio pode se mover para cima/esquerda (-1) ou baixo/direita (+1)
            int shifts[] = {-1, 1};
            for (int shift : shifts) {
                int next_blank = curr.blank_line + shift;
                
                if (next_blank >= 0 && next_blank < 4) {
                    // O espaço vazio (cujo destino final é a linha/coluna 3) troca com 
                    // uma peça da linha vizinha que tem como destino a linha/coluna 'g'
                    for (int g = 0; g < 4; ++g) {
                        if (curr.matrix[next_blank * 4 + g] > 0) {
                            std::vector<int> next_matrix = curr.matrix;

                            // Atualiza as contagens devido à troca física das peças
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
        return 99; // Valor de segurança (indica estado inválido, teoricamente inacessível)
    }

    uint64_t get_encoded_key(const std::vector<int>& matrix, int blank_line) const {
        return encode(matrix, blank_line);
    }

    // Padrão Singleton para garantir a thread-safety e inicialização única
    static const WalkingDistanceDB& get_instance() {
        static WalkingDistanceDB instance;
        return instance;
    }
};

// --- CLASSE DE HEURÍSTICA ADAPTADA ---
class Heuristics_3 : public IHeuristic {
private:
    int board_size;
    int side_length;

public:
    Heuristics_3(int size = 16) {
        board_size = size;
        side_length = static_cast<int>(std::sqrt(board_size));
        // Dispara a inicialização da tabela se for a primeira vez
        WalkingDistanceDB::get_instance();
    }

    int calculate(const std::vector<int>& board) const override{
        // Obter a instância única do banco de dados pré-calculado
        const auto& db = WalkingDistanceDB::get_instance();

        // Matrizes de contagem 4x4 para Linhas e Colunas
        std::vector<int> row_matrix(16, 0);
        std::vector<int> col_matrix(16, 0);
        
        int blank_row = 0;
        int blank_col = 0;

        // Mapeia o estado atual do tabuleiro para as estruturas de 1D
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

        // Consulta as distâncias exatas no banco de dados
        uint64_t row_key = db.get_encoded_key(row_matrix, blank_row);
        uint64_t col_key = db.get_encoded_key(col_matrix, blank_col);

        int wd_row = db.get_distance(row_key);
        int wd_col = db.get_distance(col_key);

        return wd_row + wd_col;
    }
};