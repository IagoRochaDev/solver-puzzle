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
#include "../State.hpp"
#include "IAlgorithm.hpp"
#include "../heuristics/IHeuristic.hpp"

class ParallelIDAStar : public IAlgorithm {
private:
    IHeuristic* heuristic;

public:
    ParallelIDAStar(IHeuristic* h) : heuristic(h) {}

    int search_worker(State* current, int threshold, std::vector<uint64_t>& path_stack, 
                    long long& local_nodes, IHeuristic* solver_heuristics, 
                    State*& goal_state, std::atomic<bool>& global_found, std::mutex& goal_mtx) {
        
        // Aborta imediatamente se outra thread já resolveu o quebra-cabeça
        if (global_found.load(std::memory_order_relaxed)) return INT_MAX;

        int f = current->get_f();
        if (f > threshold) return f;
        
        // Objetivo alcançado: Sincronização segura via Mutex
        if (current->is_goal()) {
            std::lock_guard<std::mutex> lock(goal_mtx);
            if (!global_found.load()) { 
                goal_state = current;
                global_found.store(true);
            }
            return -1; 
        }

        int min_exceeded = INT_MAX;
        
        // Incrementa contador local (Isolado na Stack da thread, sem Lock ou Atomic)
        local_nodes++; 
        
        std::vector<State*> children = current->generate_successors(current);

        for (State* child : children) {
            if (global_found.load(std::memory_order_relaxed)) {
                delete child;
                continue;
            }

            uint64_t hash = child->get_hash();

            // OTIMIZAÇÃO DE CICLO: Busca linear em vetor contíguo (Aproveita o Cache L1/L2 da CPU).
            // Como o caminho do IDA* é raso, varrer este vetor é ordens de magnitude mais rápido que unordered_set.
            bool has_cycle = false;
            for (uint64_t h : path_stack) {
                if (h == hash) {
                    has_cycle = true;
                    break;
                }
            }

            if (!has_cycle) {
                child->set_h(solver_heuristics->calculate(child->get_board()));
                path_stack.push_back(hash); // Push na árvore
                
                int t = search_worker(child, threshold, path_stack, local_nodes, solver_heuristics, goal_state, global_found, goal_mtx);
                
                if (t == -1) {
                    // Caminho da vitória encontrado, preserva o nó
                } else {
                    if (t < min_exceeded) min_exceeded = t;
                    path_stack.pop_back(); // Backtracking
                    delete child;          // Libera memória do beco sem saída
                }
            } else {
                delete child; 
            }
        }
        
        if (global_found.load(std::memory_order_relaxed)) return -1;
        return min_exceeded;
    }

    // --- AUXILIAR: RECONSTRÓI O HISTÓRICO DE HASHES PARA CADA THREAD ---
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

    // --- LAÇO PRINCIPAL PARALELO ---
    void solve(const std::vector<int>& initial_board) override {
        
        IHeuristic* solver_heuristics = heuristic;

        State* start_state = new State(initial_board);
        start_state->set_h(solver_heuristics->calculate(start_state->get_board()));

        if (start_state->is_goal()) {
            std::cout << "O tabuleiro inicial ja e o objetivo!\n";
            delete start_state;
            return;
        }

        // OTIMIZAÇÃO DE HARDWARE: Expansão da Raiz (Saturação de Cores)
        // Geramos sub-árvores profundas o suficiente para alimentar todas as threads da CPU uniformemente.
        std::vector<State*> frontier;
        std::unordered_set<State*> frontier_allocated_nodes; // Rastreia alocações para evitar Memory Leaks
        
        frontier.push_back(start_state);
        frontier_allocated_nodes.insert(start_state);

        unsigned int num_cores = std::thread::hardware_concurrency();
        unsigned int target_tasks = num_cores * 4; // Multiplicador para garantir balanceamento de carga dinâmico

        std::cout << "[Config] Detectados " << num_cores << " nucleos de CPU. Gerando tarefas de busca...\n";

        bool goal_found_early = false;

        while (frontier.size() < target_tasks) {
            std::vector<State*> next_frontier;
            bool expanded_any = false;

            for (State* s : frontier) {
                // Se o estado atual já é o objetivo, preserva ele na fronteira e não o expande
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
                        
                        // Se um dos filhos gerados for o objetivo, acende o alerta
                        if (child->is_goal()) {
                            goal_found_early = true;
                        }
                    }
                } else {
                    next_frontier.push_back(s);
                }
            }
            
            frontier = next_frontier;
            
            // Aborta a expansão se não houver mais filhos ou se o objetivo já estiver na fronteira
            if (!expanded_any || goal_found_early) break; 
        }

        std::cout << "[Config] Fronteira expandida para " << frontier.size() << " ramos independentes.\n\n";

        int threshold = start_state->get_f(); 
        long long total_nodes_expanded = 0;
        
        std::atomic<bool> global_found{false};
        std::mutex goal_mtx;
        State* goal_state = nullptr;

        // --- LOOP DO THRESHOLD (VISUALIZAÇÃO COMPLETA) ---
        while (true) {
            global_found.store(false);
            int next_threshold = INT_MAX;
            std::atomic<long long> nodes_this_iteration{0};
            
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // VISUALIZAÇÃO DO LIMITE ATUAL: Essencial para acompanhar o progresso
            std::cout << ">> [IDA*] Buscando no limite de custo: " << threshold << " movimentos..." << std::endl;
            
            std::vector<std::future<int>> futures;

            // Dispara o processamento paralelo em cima da nossa fronteira expandida
            for (State* task_root : frontier) {
                futures.push_back(std::async(std::launch::async, [&, task_root]() {
                    long long local_nodes = 0;
                    std::vector<uint64_t> path_stack = get_ancestor_hashes(task_root);
                    
                    int result = search_worker(task_root, threshold, path_stack, local_nodes, 
                                            solver_heuristics, goal_state, global_found, goal_mtx);
                    
                    nodes_this_iteration.fetch_add(local_nodes, std::memory_order_relaxed);
                    return result;
                }));
            }

            // Aguarda a sincronização da barreira de threads da iteração atual
            for (auto& fut : futures) {
                int t = fut.get();
                if (t != -1 && t < next_threshold) {
                    next_threshold = t;
                }
            }

            auto end_time = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();
            
            total_nodes_expanded += nodes_this_iteration.load();

            // Print detalhado de telemetria por iteração
            std::cout << "   ↳ Concluido em: " << duration << " ms | Nos expandidos nesta rodada: " 
                    << nodes_this_iteration.load() << "\n" << std::endl;

            if (global_found.load()) {
                std::cout << "==================================================\n";
                std::cout << "           SOLUCAO ENCONTRADA COM SUCESSO!        \n";
                std::cout << "==================================================\n";
                std::cout << "Custo Real da Solucao: " << goal_state->get_g() << " movimentos\n";
                std::cout << "Total Global de Nos Expandidos: " << total_nodes_expanded << "\n";
                
                // Recupera e organiza o caminho dos estados
                std::vector<State*> path;
                State* step = goal_state;
                while (step != nullptr) {
                    path.push_back(step);
                    step = step->get_parent();
                }
                std::reverse(path.begin(), path.end());
                
                std::unordered_set<State*> winning_path_set(path.begin(), path.end());

                // Limpeza cirúrgica contra Memory Leaks (Deleta o que não for do caminho vencedor)
                for (State* s : frontier_allocated_nodes) {
                    if (winning_path_set.find(s) == winning_path_set.end()) {
                        delete s;
                    }
                }
                
                // Deleta os nós do caminho vencedor
                for (State* s : path) {
                    delete s; 
                }
                break;
            }
            if (/*next_threshold > 60 ||*/ duration > 1000*30/*1000*60*60*3*/) {
                std::cout << ">> [FALHA] Nenhuma solucao foi encontrada para menos de "<< next_threshold-2 <<" movimentos.\n";
                std::cout << "   ↳ Total Global de Nos Expandidos: " << total_nodes_expanded << "\n";
                std::cout << "   ↳ Total Tempo: " << duration << " ms\n";
                for (State* s : frontier_allocated_nodes) delete s;
                break;
            }
            if (next_threshold == INT_MAX) {
                std::cout << "O espaco de busca foi completamente esgotado sem solucao.\n";
                for (State* s : frontier_allocated_nodes) delete s;
                break;
            }
            
            threshold = next_threshold; 
        }
    }
};