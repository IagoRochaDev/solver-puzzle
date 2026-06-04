#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <limits>
#include <unistd.h>
#include "heuristics/IHeuristic.hpp"
#include "heuristics/Heuristics_1.hpp"
#include "heuristics/Heuristics_2.hpp"
#include "heuristics/Heuristics_3.hpp"
#include "heuristics/Heuristics_4.hpp"
#include "algorithms/IAlgorithm.hpp"
#include "algorithms/IDAStar.hpp"
#include "algorithms/AStar.hpp"
#include "algorithms/ParallelIDAStar.hpp"

int main(int argc, char* argv[]) {
    int h_choice = 1, a_choice = 2;

    std::cout << "=== BEM-VINDO AO SOLVER DO PUZZLE ===\n\n";
    bool stdin_is_tty = isatty(fileno(stdin));
    bool args_provided = argc >= 3;
    std::vector<std::string> input_lines;

    if (args_provided) {
        try {
            h_choice = std::stoi(argv[1]);
        } catch (...) {
            h_choice = 1;
        }
        try {
            a_choice = std::stoi(argv[2]);
        } catch (...) {
            a_choice = 2;
        }
        std::cout << "Heuristica selecionada: " << h_choice << "\n";
        std::cout << "Algoritmo selecionado: " << a_choice << "\n";
    } else if (!stdin_is_tty) {
        std::cerr << "Erro: stdin redirecionado. Use argumentos para heuristica e algoritmo.\n";
        std::cerr << "Uso: make run ARGS=\"<heuristica> <algoritmo>\" < ins/15puzzle_instances_valled.txt\n";
        return 1;
    } else {
        std::cout << "Escolha a Heuristica:\n";
        std::cout << "1. Heuristica 1 (Manhattan)\n";
        std::cout << "2. Heuristica 2 (Linear Conflict)\n";
        std::cout << "3. Heuristica 3 (PDB 1)\n";
        std::cout << "4. Heuristica 4 (PDB 2)\n";
        std::cout << "> " << std::flush;
        if (!(std::cin >> h_choice)) {
            std::cerr << "\nErro: entrada invalida ou EOF antes da escolha da heuristica.\n";
            return 1;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // 2. Escolha do Algoritmo
    if (argc < 3) {
        std::cout << "\nEscolha o Algoritmo:\n";
        std::cout << "1. A* (A-Estrela)\n";
        std::cout << "2. IDA* Sequencial\n";
        std::cout << "3. IDA* Paralelo\n";
        std::cout << "> " << std::flush;
        if (!(std::cin >> a_choice)) {
            std::cerr << "\nErro: entrada invalida ou EOF antes da escolha do algoritmo.\n";
            return 1;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    auto infer_board_size = [&](const std::string& line) {
        std::stringstream ss(line);
        int value;
        int count = 0;
        while (ss >> value) {
            count++;
        }
        return count;
    };

    auto create_heuristic = [&](int board_size) -> IHeuristic* {
        switch (h_choice) {
            case 1: return new Heuristics_1(board_size);
            case 2: return new Heuristics_2(board_size);
            case 3:
                if (board_size != 16) {
                    std::cerr << "Heuristica 3 suporta apenas tabuleiros 4x4 (16 pecas).\n";
                    return nullptr;
                }
                return new Heuristics_3(16);
            case 4:
                if (board_size != 16 && board_size != 9) {
                    std::cerr << "Heuristica 4 suporta apenas tabuleiros 3x3 ou 4x4.\n";
                    return nullptr;
                }
                return new Heuristics_4(board_size);
            default:
                return new Heuristics_1(board_size);
        }
    };

    auto create_solver = [&](IHeuristic* heuristic) -> IAlgorithm* {
        switch (a_choice) {
            case 1: return new AStar(heuristic);
            case 2: return new IDAStar(heuristic);
            case 3: return new ParallelIDAStar(heuristic);
            default:
                std::cout << "Opcao invalida. Usando IDA* Sequencial por padrao.\n";
                return new IDAStar(heuristic);
        }
    };

    if (!stdin_is_tty) {
        std::string input_line;
        while (std::getline(std::cin, input_line)) {
            if (input_line.empty() || input_line.find_first_not_of(" \t\r\n") == std::string::npos) {
                continue;
            }
            input_lines.push_back(input_line);
        }

        if (input_lines.empty()) {
            std::cerr << "Erro: nenhuma instancia valida encontrada na entrada.\n";
            return 1;
        }
    }

    IHeuristic* selected_heuristic = nullptr;
    IAlgorithm* solver = nullptr;
    bool solver_created = false;

    if (!stdin_is_tty && !input_lines.empty()) {
        int board_size = infer_board_size(input_lines[0]);
        if (board_size <= 0) {
            std::cerr << "Erro: nao foi possivel inferir o tamanho do tabuleiro da primeira linha.\n";
            return 1;
        }
        selected_heuristic = create_heuristic(board_size);
        if (!selected_heuristic) return 1;
        solver = create_solver(selected_heuristic);
        solver_created = true;

        int instance_id = 1;
        for (const std::string& line : input_lines) {
            std::stringstream ss(line);
            std::vector<int> initial_board;
            int value;

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

            solver->solve(initial_board);
            std::cout << "==================================================\n\n";
        }
    } else {
        std::string line;
        int instance_id = 1;
        while (std::getline(std::cin, line)) {
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

            if (!solver_created) {
                int board_size = static_cast<int>(initial_board.size());
                if (board_size <= 0) {
                    std::cerr << "Erro: instancia invalida.\n";
                    return 1;
                }
                selected_heuristic = create_heuristic(board_size);
                if (!selected_heuristic) return 1;
                solver = create_solver(selected_heuristic);
                solver_created = true;
            }

            std::cout << "==================================================\n";
            std::cout << " PROCESSANDO INSTANCIA #" << instance_id++ << "\n";
            std::cout << "==================================================\n";
            std::cout << "Tabuleiro Inicial: ";
            for (int val : initial_board) {
                std::cout << val << " ";
            }
            std::cout << "\n--------------------------------------------------\n";

            solver->solve(initial_board);
            
            std::cout << "==================================================\n\n";
        }
    }

    delete solver;
    delete selected_heuristic;
    return 0;
}