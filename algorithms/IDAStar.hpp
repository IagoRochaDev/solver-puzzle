#pragma once

#include <iostream>
#include <vector>
#include <unordered_set>
#include <climits>
#include <algorithm>
#include <chrono>
#include "../State.hpp"
#include "IAlgorithm.hpp"
#include "../heuristics/IHeuristic.hpp"


class IDAStar : public IAlgorithm {
private:
    IHeuristic* heuristic;

public:
    IDAStar(IHeuristic* h) : heuristic(h) {}

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

    
    void solve(const std::vector<int>& initial_board) override {
        
        IHeuristic* solver_heuristics = heuristic;

        
        State* start_state = new State(initial_board);
        start_state->set_h(solver_heuristics->calculate(start_state->get_board()));

        
        int threshold = start_state->get_f(); 
        
        
        std::unordered_set<uint64_t> path_hashes;
        path_hashes.insert(start_state->get_hash());

        long long total_nodes_expanded = 0; 
        State* goal_state = nullptr;

        std::cout << "\nIniciando IDA* Sequencial...\n";

        
        while (true) {
            int nodes_this_iteration = 0; 
            
            std::cout << "\n>> [IDA*] Buscando no limite de custo: " << threshold << " movimentos...\n";
            
            
            auto start_time = std::chrono::high_resolution_clock::now();
            
            int t = search(start_state, threshold, path_hashes, nodes_this_iteration, solver_heuristics, goal_state);
            
            
            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            
            total_nodes_expanded += nodes_this_iteration;
            
            if (t == -1) {
                std::cout << "   ↳ Concluido em: " << duration << " ms | Nos expandidos nesta rodada: " << nodes_this_iteration << "\n";
                std::cout << "\n==================================================\n";
                std::cout << " Objetivo encontrado com sucesso!\n";
                std::cout << " Custo total (movimentos): " << goal_state->get_g() << "\n";
                std::cout << " Total de nos expandidos (todas as rodadas): " << total_nodes_expanded << "\n";
                std::cout << "==================================================\n";
                
                
                std::vector<State*> path;
                State* step = goal_state;
                while (step != nullptr) {
                    path.push_back(step);
                    step = step->get_parent();
                }
                std::reverse(path.begin(), path.end());
                
                
                for (State* s : path) {
                    delete s; 
                }
                break; 
            }
            if (duration > 1000*60*5) {
                std::cout << "   ↳ [FALHA] Nenhuma solucao foi encontrada para menos de "<< t-2 <<" movimentos.\n";
                std::cout << "      ↳ Total Global de Nos Expandidos: " << total_nodes_expanded << "\n";
                std::cout << "      ↳ Total Tempo: " << duration << " ms\n";
                delete start_state;
                break;
            }
            if (t == INT_MAX) {
                std::cout << "   ↳ [FALHA] O espaco de busca foi esgotado e nenhuma solucao foi encontrada.\n";
                delete start_state;
                break;
            }
            
            
            std::cout << "   ↳ Concluido em: " << duration << " ms | Nos expandidos nesta rodada: " << nodes_this_iteration << "\n";
            
            
            threshold = t; 
        }
    }
};