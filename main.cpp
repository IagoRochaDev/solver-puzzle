#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <algorithm>
#include <limits>    
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
    "PDB",
    "PDB + Conflito Linear"
};

std::vector<std::string> valid_algorithms = {
    "A*",
    "IDA*",
    "IDA* Paralelo"
};

int main(int argc, char* argv[]) {
    int h_choice = 1;
    int a_choice = 3;

    if (argc >= 3) {
        try {
            h_choice = std::stoi(argv[1]);
            a_choice = std::stoi(argv[2]);
        } catch (...) {
            std::cerr << "[AVISO] Argumentos invalidos. Usando padroes: H:"<<valid_heuristics[h_choice - 1]<<" e A:"<<valid_algorithms[a_choice - 1]<<". \n";
        }
    }

    std::cout << "=== BEM-VINDO AO SOLVER DO PUZZLE ===\n";
    std::cout << "Configuracao: Heuristica [" << valid_heuristics[h_choice - 1] << "] | Algoritmo [" << valid_algorithms[a_choice - 1] << "]\n";
    std::cout << "Aguardando entrada de dados...\n\n";

    IHeuristic* selected_heuristic = nullptr;
    IAlgorithm* solver = nullptr;
    
    std::string line;
    int instance_id = 1;
    int total_instances = 0;
    int solvable_count = 0;
    int unsolvable_count = 0;
    double total_execution_time_ms = 0.0;
    uint64_t global_nodes_expanded = 0;
    std::vector<double> valid_execution_times;

    auto global_start_time = std::chrono::high_resolution_clock::now();

    while (std::getline(std::cin, line)) {
        if (line.empty() || line.find_first_not_of(" \t\r\n") == std::string::npos) continue;

        std::stringstream ss(line);
        std::vector<int> initial_board;
        int value;

        while (ss >> value) {
            initial_board.push_back(value);
        }

        int board_size = initial_board.size();
        total_instances++;
        
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
            std::cout << "[STATUS] Tabuleiro Impossivel! Abortando resolucao.\n";
            unsolvable_count++;
        } else {
            solvable_count++;
            
            auto inst_start = std::chrono::high_resolution_clock::now();
            
            solver->solve(initial_board);
            
            auto inst_end = std::chrono::high_resolution_clock::now();
            double duration_ms = std::chrono::duration<double, std::milli>(inst_end - inst_start).count();

            valid_execution_times.push_back(duration_ms);
            total_execution_time_ms += duration_ms;

            global_nodes_expanded += solver->get_nodes_expanded();

            solver->print_stats();
            std::cout << "Tempo nesta instancia: " << std::fixed << std::setprecision(2) << duration_ms << " ms\n";
        }
        std::cout << "==================================================\n\n";
    }

    auto global_end_time = std::chrono::high_resolution_clock::now();
    double total_wall_time_ms = std::chrono::duration<double, std::milli>(global_end_time - global_start_time).count();

    if (total_instances > 0 && solvable_count > 0) {
        
        std::sort(valid_execution_times.begin(), valid_execution_times.end());
        
        double min_time = valid_execution_times.front();
        double max_time = valid_execution_times.back();
        double avg_time_ms = total_execution_time_ms / solvable_count;
        double total_sec_active = total_execution_time_ms / 1000.0;
        double nodes_per_second = (total_sec_active > 0) ? (global_nodes_expanded / total_sec_active) : 0.0;

        std::cout << "==================================================\n";
        std::cout << "          RELATORIO DE EXECUCAO FINAL             \n";
        std::cout << "==================================================\n";
        std::cout << " Algoritmo:  " << valid_algorithms[a_choice - 1] << "\n";
        std::cout << " Heuristica: " << valid_heuristics[h_choice - 1] << "\n";
        std::cout << "--------------------------------------------------\n";
        std::cout << " [INSTANCIAS]\n";
        std::cout << " Total Lidas:         " << total_instances << "\n";
        std::cout << " Solucionadas:        " << solvable_count << "\n";
        std::cout << " Inviaveis (Pulo):    " << unsolvable_count << "\n";
        std::cout << "--------------------------------------------------\n";
        std::cout << " [TEMPOS DE RESOLUCAO]\n";
        std::cout << " Tempo Minimo:        " << std::fixed << std::setprecision(2) << min_time << " ms\n";
        std::cout << " Tempo Maximo:        " << std::fixed << std::setprecision(2) << max_time << " ms\n";
        std::cout << " Tempo Medio:         " << std::fixed << std::setprecision(2) << avg_time_ms << " ms\n";
        std::cout << " Total Busca Ativa:   " << std::fixed << std::setprecision(2) << total_sec_active << " s\n";
        std::cout << " Total Pipeline(Wall):" << std::fixed << std::setprecision(2) << total_wall_time_ms / 1000.0 << " s\n";
        std::cout << "--------------------------------------------------\n";
        std::cout << " [DESEMPENHO DO MOTOR]\n";
        std::cout << " Total de Nos Abertos:" << global_nodes_expanded << " (Fator de Poda)\n";
        std::cout << " Vazao de Busca:      " << std::fixed << std::setprecision(0) << nodes_per_second << " nos/segundo\n";
        std::cout << "==================================================\n";
        /*
        std::cout << "\n[DADOS BRUTOS PARA HISTOGRAMA - TEMPO EM MS]\n";
        for (double t : valid_execution_times) {
            std::cout << t << "\n";
        }
        */
    } else if (total_instances > 0) {
        std::cout << "Nenhuma instancia solucionavel foi encontrada no conjunto de testes.\n";
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