#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include "State.hpp"
#include "Heuristics.hpp"

void solve_astar(const std::vector<int>& initial_board) {
    
    Heuristics solver_heuristics(initial_board.size());

    // Fila de prioridade (Open Set)
    std::priority_queue<State*, std::vector<State*>, CompareF> open_set;

    // Tabela Hash (Closed Set)
    std::unordered_map<uint64_t, State*> closed_set;

    // Inicialização da raiz
    State* start_state = new State(initial_board);
    start_state->set_h(solver_heuristics.calculate(start_state->get_board()));
    
    open_set.push(start_state);
    closed_set[start_state->get_hash()] = start_state;

    int nodes_expanded = 0; // Estatística para o seu relatório

    // Laço Principal
    while (!open_set.empty()) {
        // 1. Pega o estado mais promissor da fronteira
        State* current = open_set.top();
        open_set.pop();

        // 2. Verifica se chegou ao objetivo
        if (current->is_goal()) {
            std::cout << "Objetivo encontrado!\n";
            std::cout << "Custo total (movimentos): " << current->get_g() << "\n";
            std::cout << "Nos expandidos: " << nodes_expanded << "\n";
            
            // --- RECONSTRUÇÃO DO CAMINHO ---
            std::vector<State*> path;
            State* step = current;
            while (step != nullptr) {
                path.push_back(step);
                step = step->get_parent();
            }
            std::reverse(path.begin(), path.end());
            
            // Imprime o caminho (opcional, comente se o tabuleiro for muito grande)
            for (const State* s : path) {
                s->print();
                std::cout << "-----\n";
            }

            // --- FAXINA FINAL DE MEMÓRIA (MUITO IMPORTANTE) ---
            // Como guardamos TODOS os ponteiros gerados no closed_set,
            // basta iterar por ele e deletar um por um.
            for (auto& pair : closed_set) {
                delete pair.second;
            }
            
            return; // Encerra a função com sucesso
        }

        nodes_expanded++;

        // 3. Gera os sucessores do estado atual
        std::vector<State*> children = current->generate_successors(current);

        for (State* child : children) {
            uint64_t child_hash = child->get_hash();

            // Verifica se o estado JÁ FOI visitado
            if (closed_set.find(child_hash) != closed_set.end()) {
                // PREVENÇÃO DE VAZAMENTO DE MEMÓRIA:
                // Se já visitamos esse tabuleiro, este objeto 'child' recém-criado 
                // é lixo. Deletamos IMEDIATAMENTE.
                delete child;
            } else {
                // Se é um estado novo, calculamos a heurística e adicionamos nas estruturas
                child->set_h(solver_heuristics.calculate(child->get_board()));
                
                closed_set[child_hash] = child;
                open_set.push(child);
            }
        }
    }

    std::cout << "O algoritmo esgotou o espaco de busca e nao encontrou solucao.\n";
    
    // Faxina de memória caso o objetivo não seja encontrado (ex: tabuleiro insolúvel)
    for (auto& pair : closed_set) {
        delete pair.second;
    }
}