#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include "heuristics/IHeuristic.hpp"
#include "algorithms/IAlgorithm.hpp"
#include "heuristics/Heuristics_1.hpp"
#include "heuristics/Heuristics_2.hpp"
#include "heuristics/Heuristics_3.hpp"
#include "heuristics/Heuristics_4.hpp"
#include "algorithms/AStar.hpp"
#include "algorithms/IDAStar.hpp"
#include "algorithms/ParallelIDAStar.hpp"

bool is_solvable(const std::vector<int>& board);
IHeuristic* create_heuristic(int choice, int board_size);
IAlgorithm* create_solver(int choice, IHeuristic* heuristic);


std::vector<std::string> valid_heuristics = {
    "Manhattan",
    "Manhattan + Conflito Linear",
    "WalkingDistanceDB",
    "WalkingDistanceDB + Conflito Linear"
};

std::vector<std::string> valid_algorithms = {
    "A*",
    "IDA*",
    "IDA* Paralelo"
};


int main(int argc, char* argv[]) {
    int h_choice = 4;
    int a_choice = 3;

    if (argc >= 3) {
        try {
            h_choice = std::stoi(argv[1]);
            a_choice = std::stoi(argv[2]);
        } catch (...) {
            std::cerr << "[AVISO] Argumentos invalidos. Usando padroes: H:"<<valid_heuristics[3]<<"e A:"<<valid_algorithms[2]<<". \n";
        }
    }

    std::cout << "=== BEM-VINDO AO SOLVER DO PUZZLE ===\n";
    std::cout << "Configuracao: Heuristica [" << valid_heuristics[h_choice - 1] << "] | Algoritmo [" << valid_algorithms[a_choice - 1] << "]\n";
    std::cout << "Aguardando entrada de dados...\n\n";

    IHeuristic* selected_heuristic = nullptr;
    IAlgorithm* solver = nullptr;
    
    std::string line;
    int instance_id = 1;

    while (std::getline(std::cin, line)) {
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

        std::stringstream ss(line);
        std::vector<int> initial_board;
        int value;

        while (ss >> value) {
            initial_board.push_back(value);
        }

        int board_size = initial_board.size();
        
        if (solver == nullptr) {
            selected_heuristic = create_heuristic(h_choice, board_size);
            solver = create_solver(a_choice, selected_heuristic);
        }

        std::cout << "==================================================\n";
        std::cout << " PROCESSANDO INSTANCIA #" << instance_id++ << "\n";
        std::cout << "==================================================\n";
        std::cout << "Tabuleiro Inicial: ";
        for (int val : initial_board) std::cout << val << " ";
        std::cout << "\n--------------------------------------------------\n";

        if (!is_solvable(initial_board)) {
            std::cout << "[STATUS] Tabuleiro Impossivel! Abortando resolucao matematicamente.\n";
        } else {
            solver->solve(initial_board);
        }
        std::cout << "==================================================\n\n";
    }

    delete solver;
    delete selected_heuristic;

    return 0;
}

bool is_solvable(const std::vector<int>& board) {
    int size = board.size();
    int width = (size == 9) ? 3 : (size == 16) ? 4 : 0;
    
    if (width == 0) return true;

    int inversions = 0;
    int blank_row_from_bottom = 0;

    for (int i = 0; i < size; i++) {
        if (board[i] == 0) {
            blank_row_from_bottom = width - (i / width);
            continue;
        }
        for (int j = i + 1; j < size; j++) {
            if (board[j] == 0) continue;
            if (board[i] > board[j]) inversions++;
        }
    }

    if (width % 2 != 0) {
        return inversions % 2 == 0;
    } 
    else {
        bool is_blank_row_even = (blank_row_from_bottom % 2 == 0);
        bool is_inversions_even = (inversions % 2 == 0);
        return is_blank_row_even != is_inversions_even;
    }
}

IHeuristic* create_heuristic(int choice, int board_size) {
    switch (choice) {
        case 1: return new Heuristics_1(board_size);
        case 2: return new Heuristics_2(board_size);
        case 3: return new Heuristics_3(board_size);
        case 4: return new Heuristics_4(board_size);
        default: return new Heuristics_4(board_size);
    }
}

IAlgorithm* create_solver(int choice, IHeuristic* heuristic) {
    switch (choice) {
        case 1: return new AStar(heuristic);
        case 2: return new IDAStar(heuristic);
        case 3: return new ParallelIDAStar(heuristic);
        default: return new ParallelIDAStar(heuristic);
    }
}