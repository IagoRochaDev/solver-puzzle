#pragma once

#include <iostream>
#include <vector>
#include <climits>
#include <algorithm>
#include <future>
#include <atomic>
#include <mutex>
#include <chrono>
#include <thread>
#include <unordered_set>
#include "../domain/State.hpp"
#include "IAlgorithm.hpp"
#include "../heuristics/IHeuristic.hpp"

class ParallelIDAStar : public IAlgorithm {
private:
    IHeuristic* heuristic;

    long long total_nodes_expanded;
    int final_cost;
    bool goal_found;
    bool timeout_occurred;
    bool exhausted_search;
    std::vector<std::vector<int>> solution_path_boards;

    int search_worker(State* current, int threshold, std::vector<uint64_t>& path_stack, 
                    long long& local_nodes, IHeuristic* solver_heuristics, 
                    State*& goal_state, std::atomic<bool>& global_found, std::mutex& goal_mtx) {
        
        if (global_found.load(std::memory_order_relaxed)) return INT_MAX;

        int f = current->get_f();
        if (f > threshold) return f;
        
        if (current->is_goal()) {
            std::lock_guard<std::mutex> lock(goal_mtx);
            if (!global_found.load()) { 
                goal_state = current;
                global_found.store(true);
            }
            return -1; 
        }

        int min_exceeded = INT_MAX;
        local_nodes++; 
        
        std::vector<State*> children = current->generate_successors(current);

        for (State* child : children) {
            if (global_found.load(std::memory_order_relaxed)) {
                delete child;
                continue;
            }

            uint64_t hash = child->get_hash();
            bool has_cycle = false;
            for (uint64_t h : path_stack) {
                if (h == hash) {
                    has_cycle = true;
                    break;
                }
            }

            if (!has_cycle) {
                child->set_h(solver_heuristics->calculate(child->get_board()));
                path_stack.push_back(hash); 
                
                int t = search_worker(child, threshold, path_stack, local_nodes, solver_heuristics, goal_state, global_found, goal_mtx);
                
                if (t == -1) {
                } else {
                    if (t < min_exceeded) min_exceeded = t;
                    path_stack.pop_back(); 
                    delete child;          
                }
            } else {
                delete child; 
            }
        }
        
        if (global_found.load(std::memory_order_relaxed)) return -1;
        return min_exceeded;
    }

    std::vector<uint64_t> get_ancestor_hashes(State* node) {
        std::vector<uint64_t> hashes;
        State* curr = node;
        while (curr != nullptr) {
            hashes.push_back(curr->get_hash());
            curr = curr->get_parent();
        }
        std::reverse(hashes.begin(), hashes.end());
        return hashes;
    }

public:
    ParallelIDAStar(IHeuristic* h) 
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

        if (start_state->is_goal()) {
            goal_found = true;
            final_cost = 0;
            solution_path_boards.push_back(start_state->get_board());
            delete start_state;
            return;
        }

        std::vector<State*> frontier;
        std::unordered_set<State*> frontier_allocated_nodes; 
        
        frontier.push_back(start_state);
        frontier_allocated_nodes.insert(start_state);

        unsigned int num_cores = std::thread::hardware_concurrency();
        unsigned int target_tasks = num_cores * 4; 

        std::cout << "\nIniciando IDA* Paralelo...\n";
        std::cout << "[Config] Detectados " << num_cores << " nucleos de CPU. Gerando tarefas de busca...\n";

        bool goal_found_early = false;

        while (frontier.size() < target_tasks) {
            std::vector<State*> next_frontier;
            bool expanded_any = false;

            for (State* s : frontier) {
                if (s->is_goal()) {
                    next_frontier.push_back(s);
                    goal_found_early = true;
                    continue;
                }

                std::vector<State*> children = s->generate_successors(s);
                if (!children.empty()) {
                    expanded_any = true;
                    for (State* child : children) {
                        child->set_h(solver_heuristics->calculate(child->get_board()));
                        next_frontier.push_back(child);
                        frontier_allocated_nodes.insert(child);
                        
                        if (child->is_goal()) {
                            goal_found_early = true;
                        }
                    }
                } else {
                    next_frontier.push_back(s);
                }
            }
            frontier = next_frontier;
            if (!expanded_any || goal_found_early) break; 
        }

        std::cout << "[Config] Fronteira expandida para " << frontier.size() << " ramos independentes.\n";

        int threshold = start_state->get_f(); 
        std::atomic<bool> global_found_flag{false};
        std::mutex goal_mtx;
        State* goal_state = nullptr;

        while (true) {
            global_found_flag.store(false);
            int next_threshold = INT_MAX;
            std::atomic<long long> nodes_this_iteration{0};
            
            auto start_time = std::chrono::high_resolution_clock::now();
            std::cout << ">> [IDA*] Buscando no limite de custo: " << threshold << " movimentos..." << std::endl;
            
            std::vector<std::future<int>> futures;

            for (State* task_root : frontier) {
                futures.push_back(std::async(std::launch::async, [&, task_root]() {
                    long long local_nodes = 0;
                    std::vector<uint64_t> path_stack = get_ancestor_hashes(task_root);
                    
                    int result = search_worker(task_root, threshold, path_stack, local_nodes, 
                                            solver_heuristics, goal_state, global_found_flag, goal_mtx);
                    
                    nodes_this_iteration.fetch_add(local_nodes, std::memory_order_relaxed);
                    return result;
                }));
            }

            for (auto& fut : futures) {
                int t = fut.get();
                if (t != -1 && t < next_threshold) {
                    next_threshold = t;
                }
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            total_nodes_expanded += nodes_this_iteration.load();

            std::cout << "   ↳ Concluido em: " << duration << " ms | Nos expandidos nesta rodada: " 
                      << nodes_this_iteration.load() << "\n" << std::endl;

            if (global_found_flag.load()) {
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

                std::unordered_set<State*> winning_path_set(path.begin(), path.end());
                for (State* s : frontier_allocated_nodes) {
                    if (winning_path_set.find(s) == winning_path_set.end()) {
                        delete s;
                    }
                }
                for (State* s : path) {
                    delete s; 
                }
                break;
            }

            if (duration > 1000 * 30) {
                timeout_occurred = true;
                for (State* s : frontier_allocated_nodes) delete s;
                break;
            }

            if (next_threshold == INT_MAX) {
                exhausted_search = true;
                for (State* s : frontier_allocated_nodes) delete s;
                break;
            }
            
            threshold = next_threshold; 
        }
        
    }

    void print_stats() const override {
        std::cout << "\n==================================================\n";
        if (goal_found) {
            std::cout << "           SOLUCAO ENCONTRADA COM SUCESSO!        \n";
            std::cout << "==================================================\n";
            std::cout << " Custo da Solucao: " << final_cost << " movimentos\n";
        } else if (timeout_occurred) {
            std::cout << " [FALHA] Execucao abortada por estourar o tempo limite.\n";
            std::cout << "==================================================\n";
        } else if (exhausted_search) {
            std::cout << " [FALHA] O espaco de busca foi esgotado sem solucao.\n";
            std::cout << "==================================================\n";
        } else {
            std::cout << " [STATUS] Processamento finalizado sem resposta.\n";
            std::cout << "==================================================\n";
        }
        std::cout << " Total de Nos Expandidos: " << total_nodes_expanded << "\n";
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