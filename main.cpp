#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "AStar.hpp"

int main() {
    std::string line;
    int instance_id = 1;

    // std::cin vai ler linha por linha do arquivo redirecionado automaticamente
    while (std::getline(std::cin, line)) {
        // Ignora linhas vazias ou que contenham apenas espaços em branco
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) {
            continue; 
        }

        std::stringstream ss(line);
        std::vector<int> initial_board;
        int value;

        // Extrai os inteiros separados por espaço da linha atual
        while (ss >> value) {
            initial_board.push_back(value);
        }

        std::cout << "==================================================\n";
        std::cout << " PROCESSANDO INSTANCIA #" << instance_id++ << "\n";
        std::cout << "==================================================\n";
        std::cout << "Tabuleiro Inicial: ";
        for (int val : initial_board) {
            std::cout << val << " ";
        }
        std::cout << "\n--------------------------------------------------\n";

        // Executa o algoritmo A* para a instância atual
        solve_astar(initial_board);
        
        std::cout << "==================================================\n\n";
    }

    return 0;
}