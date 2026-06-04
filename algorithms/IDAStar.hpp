#pragma once

#include <iostream>
#include <vector>
#include <unordered_set>
#include <climits>
#include <algorithm>
#include <chrono>
#include "../domain/State.hpp"
#include "IAlgorithm.hpp"
#include "../heuristics/IHeuristic.hpp"

class IDAStar : public IAlgorithm {
private:
    IHeuristic* heuristic;

    long long total_nodes_expanded;
    int final_cost;
    bool goal_found;
    bool timeout_occurred;
    bool exhausted_search;
    std::vector<std::vector<int>> solution_path_boards;

    int search(State* current, int threshold, std::unordered_set<uint64_t>& path_hashes, 
               int& nodes_expanded, IHeuristic* solver_heuristics, State*& goal_state) {
        
        int f = current->get_f(); 

        if (f > threshold) return f;
        
        if (current->is_goal()) {
            goal_state = current;
            return -1; 
        }

        int min_exceeded = INT_MAX;
        std::vector<State*> children = current->generate_successors(current);
        nodes_expanded++;

        for (State* child : children) {
            if (goal_state != nullptr) {
                delete child;
                continue;
            }

            uint64_t hash = child->get_hash();

            if (path_hashes.find(hash) == path_hashes.end()) {
                child->set_h(solver_heuristics->calculate(child->get_board()));
                path_hashes.insert(hash); 
                
                int t = search(child, threshold, path_hashes, nodes_expanded, solver_heuristics, goal_state);
                
                if (t == -1) {
                } else {
                    if (t < min_exceeded) min_exceeded = t;
                    path_hashes.erase(hash); 
                    delete child;            
                }
            } else {
                delete child; 
            }
        }
        
        if (goal_state != nullptr) return -1;
        return min_exceeded;
    }

public:
    IDAStar(IHeuristic* h) 
        : heuristic(h), total_nodes_expanded(0), final_cost(0), 
          goal_found(false), timeout_occurred(false), exhausted_search(false) {}

    void solve(const std::vector<int>& initial_board) override {
        total_nodes_expanded = 0;
        final_cost = 0;
        goal_found = false;
        timeout_occurred = false;
        exhausted_search = false;
        solution_path_boards.clear();

        IHeuristic* solver_heuristics = heuristic;

        State* start_state = new State(initial_board);
        start_state->set_h(solver_heuristics->calculate(start_state->get_board()));

        int threshold = start_state->get_f(); 
        
        std::unordered_set<uint64_t> path_hashes;
        path_hashes.insert(start_state->get_hash());

        State* goal_state = nullptr;

        std::cout << "\nIniciando IDA* Sequencial...\n";

        while (true) {
            int nodes_this_iteration = 0; 
            std::cout << ">> [IDA*] Buscando no limite de custo: " << threshold << " movimentos...\n";
            
            auto start_time = std::chrono::high_resolution_clock::now();
            int t = search(start_state, threshold, path_hashes, nodes_this_iteration, solver_heuristics, goal_state);
            auto end_time = std::chrono::high_resolution_clock::now();
            
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            total_nodes_expanded += nodes_this_iteration;
            
            if (t == -1) {
                std::cout << "   ↳ Concluido em: " << duration << " ms | Nos expandidos nesta rodada: " << nodes_this_iteration << "\n";
                
                goal_found = true;
                final_cost = goal_state->get_g();
                
                std::vector<State*> path;
                State* step = goal_state;
                while (step != nullptr) {
                    path.push_back(step);
                    step = step->get_parent();
                }
                std::reverse(path.begin(), path.end());
                
                for (const State* s : path) {
                    solution_path_boards.push_back(s->get_board());
                }
                
                for (State* s : path) {
                    delete s; 
                }
                break; 
            }
            
            if (duration > 1000 * 60 * 5) {
                timeout_occurred = true;
                std::cout << "   ↳ [FALHA] Limite de tempo excedido nesta rodada.\n";
                delete start_state;
                break;
            }
            
            if (t == INT_MAX) {
                exhausted_search = true;
                delete start_state;
                break;
            }
            
            std::cout << "   ↳ Concluido em: " << duration << " ms | Nos expandidos nesta rodada: " << nodes_this_iteration << "\n";
            threshold = t; 
        }
        //print_stats();
        //print_solution();
    }


    void print_stats() const override {
        std::cout << "\n==================================================\n";
        if (goal_found) {
            std::cout << " Objetivo encontrado com sucesso!\n";
            std::cout << " Custo total (movimentos): " << final_cost << "\n";
        } else if (timeout_occurred) {
            std::cout << " [FALHA] Execucao abortada por estourar o tempo limite (5 minutos).\n";
        } else if (exhausted_search) {
            std::cout << " [FALHA] O espaco de busca foi esgotado e nenhuma solucao foi encontrada.\n";
        } else {
            std::cout << " [STATUS] Processamento nao iniciado ou finalizado sem resposta.\n";
        }
        std::cout << " Total de nos expandidos (todas as rodadas): " << total_nodes_expanded << "\n";
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