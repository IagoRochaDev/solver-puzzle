#pragma once

#include <vector>
#include <cmath>
#include <cstdlib>
#include "IHeuristic.hpp"

class Heuristics_2 : public IHeuristic {
private:
    int board_size;
    int side_length;
    std::vector<std::vector<int>> manhattan_table;

    // Constrói a tabela de Manhattan no início do programa
    void build_manhattan_table() {
        manhattan_table.assign(board_size, std::vector<int>(board_size, 0));

        for (int tile = 1; tile < board_size; ++tile) {
            int goal_index = tile - 1; // Assumindo objetivo: 1 2 3 ... 0
            int goal_row = goal_index / side_length;
            int goal_col = goal_index % side_length;

            for (int current_index = 0; current_index < board_size; ++current_index) {
                int current_row = current_index / side_length;
                int current_col = current_index % side_length;

                int distance = std::abs(current_row - goal_row) + std::abs(current_col - goal_col);
                manhattan_table[tile][current_index] = distance;
            }
        }
    }

public:
    Heuristics_2(int size = 9) {
        board_size = size;
        side_length = static_cast<int>(std::sqrt(board_size));
        build_manhattan_table();
    }

    // Calcula Manhattan + Conflito Linear
    int calculate(const std::vector<int>& board) const override{
        int manhattan = 0;
        int linear_conflict = 0;

        // 1. Distância de Manhattan Básica
        for (int i = 0; i < board_size; ++i) {
            int value = board[i];
            if (value != 0) {
                manhattan += manhattan_table[value][i];
            }
        }

        // 2. Conflito Linear (Linhas)
        for (int row = 0; row < side_length; ++row) {
            for (int col1 = 0; col1 < side_length; ++col1) {
                int index1 = row * side_length + col1;
                int val1 = board[index1];
                
                if (val1 == 0) continue;

                int goal_row1 = (val1 - 1) / side_length;
                
                // Se a peça 1 está na linha correta
                if (goal_row1 == row) {
                    for (int col2 = col1 + 1; col2 < side_length; ++col2) {
                        int index2 = row * side_length + col2;
                        int val2 = board[index2];
                        
                        if (val2 == 0) continue;

                        int goal_row2 = (val2 - 1) / side_length;
                        
                        // Se a peça 2 também está na mesma linha correta
                        if (goal_row2 == row) {
                            // Se o objetivo da peça 1 é estar depois da peça 2 (Inversão)
                            if (val1 > val2) {
                                linear_conflict += 2;
                            }
                        }
                    }
                }
            }
        }

        // 3. Conflito Linear (Colunas)
        for (int col = 0; col < side_length; ++col) {
            for (int row1 = 0; row1 < side_length; ++row1) {
                int index1 = row1 * side_length + col;
                int val1 = board[index1];
                
                if (val1 == 0) continue;

                int goal_col1 = (val1 - 1) % side_length;
                
                // Se a peça 1 está na coluna correta
                if (goal_col1 == col) {
                    for (int row2 = row1 + 1; row2 < side_length; ++row2) {
                        int index2 = row2 * side_length + col;
                        int val2 = board[index2];
                        
                        if (val2 == 0) continue;

                        int goal_col2 = (val2 - 1) % side_length;
                        
                        // Se a peça 2 também está na mesma coluna correta
                        if (goal_col2 == col) {
                            // Se o objetivo da peça 1 é estar abaixo da peça 2 (Inversão)
                            // Comparar os valores diretos aqui funciona porque no objetivo 
                            // os números crescem da esquerda para a direita, de cima para baixo.
                            if (val1 > val2) {
                                linear_conflict += 2;
                            }
                        }
                    }
                }
            }
        }

        // Retorna a soma das duas heurísticas
        return manhattan + linear_conflict;
    }
};