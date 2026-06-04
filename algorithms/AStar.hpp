#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <chrono>
#include "../domain/State.hpp"
#include "IAlgorithm.hpp"
#include "../heuristics/IHeuristic.hpp"

class AStar : public IAlgorithm {
private:
    IHeuristic* heuristic;
    
    long long total_nodes_expanded;
    int final_cost;
    bool goal_found;
    long long total_duration_ms;
    std::vector<std::vector<int>> solution_path_boards;

public:
    AStar(IHeuristic* h) 
        : heuristic(h), total_nodes_expanded(0), final_cost(0), 
          goal_found(false), total_duration_ms(0) {}

    void solve(const std::vector<int>& initial_board) override {
        total_nodes_expanded = 0;
        final_cost = 0;
        goal_found = false;
        total_duration_ms = 0;
        solution_path_boards.clear();

        std::priority_queue<State*, std::vector<State*>, CompareF> open_set;
        std::unordered_map<uint64_t, State*> closed_set;

        State* start_state = new State(initial_board);
        start_state->set_h(heuristic->calculate(start_state->get_board()));
        
        open_set.push(start_state);
        closed_set[start_state->get_hash()] = start_state;

        std::cout << "\nIniciando A* (A-Estrela)...\n";
        std::cout << ">> [A*] Buscando solucao otima na fila de prioridade...\n";
        
        auto start_time = std::chrono::high_resolution_clock::now();

        while (!open_set.empty()) {
            State* current = open_set.top();
            open_set.pop();

            if (current->is_goal()) {
                auto end_time = std::chrono::high_resolution_clock::now();
                total_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
                
                std::cout << "   ↳ Concluido em: " << total_duration_ms << " ms | Nos expandidos: " << total_nodes_expanded << "\n";

                goal_found = true;
                final_cost = current->get_g();
                
                std::vector<State*> path_pointers;
                State* step = current;
                while (step != nullptr) {
                    path_pointers.push_back(step);
                    step = step->get_parent();
                }
                std::reverse(path_pointers.begin(), path_pointers.end());
                
                for (const State* s : path_pointers) {
                    solution_path_boards.push_back(s->get_board());
                }

                for (auto& pair : closed_set) {
                    delete pair.second;
                }
                
                return;
            }

            total_nodes_expanded++;
            
            if (total_nodes_expanded % 200000 == 0) {
                std::cout << ">> [A*] Progresso: " << total_nodes_expanded << " nos expandidos ate agora...\n";
            }

            std::vector<State*> children = current->generate_successors(current);

            for (State* child : children) {
                uint64_t child_hash = child->get_hash();

                if (closed_set.find(child_hash) != closed_set.end()) {
                    delete child;
                } else {
                    child->set_h(heuristic->calculate(child->get_board()));
                    closed_set[child_hash] = child;
                    open_set.push(child);
                }
            }
        }

        auto end_time = std::chrono::high_resolution_clock::now();
        total_duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
        
        std::cout << "   ↳ [FALHA] O espaco de busca foi esgotado em " << total_duration_ms << " ms e nenhuma solucao foi encontrada.\n";
        
        goal_found = false;
        for (auto& pair : closed_set) {
            delete pair.second;
        }
        print_stats();
        print_solution();
    }

    void print_stats() const override {
        std::cout << "\n==================================================\n";
        if (goal_found) {
            std::cout << " Objetivo encontrado com sucesso!\n";
            std::cout << " Custo total (movimentos): " << final_cost << "\n";
        } else {
            std::cout << " [FALHA] O espaco de busca foi esgotado e nenhuma solucao foi encontrada.\n";
        }
        std::cout << " Total de nos expandidos: " << total_nodes_expanded << "\n";
        std::cout << " Tempo total de execucao: " << total_duration_ms << " ms\n";
        std::cout << "==================================================\n";
    }

    void print_solution() const override {
        if (!goal_found) {
            std::cout << "Nenhuma solucao disponivel para impressao.\n";
            return;
        }

        std::cout << "\n=== CAMINHO DA SOLUÇÃO ===\n\n";
        int step_count = 0;
        for (const auto& board : solution_path_boards) {
            std::cout << "Passo #" << step_count++ << ":\n";
            State temp_state(board);
            temp_state.print();
            std::cout << "-----\n";
        }
    }
};