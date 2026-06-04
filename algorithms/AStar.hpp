#pragma once

#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include "../State.hpp"
#include "IAlgorithm.hpp"
#include "../heuristics/IHeuristic.hpp"

class AStar : public IAlgorithm {
private:
    IHeuristic* heuristic;

public:
    AStar(IHeuristic* h) : heuristic(h) {}

    void solve(const std::vector<int>& initial_board) override{
        
        IHeuristic* solver_heuristics = heuristic;

        std::priority_queue<State*, std::vector<State*>, CompareF> open_set;

        std::unordered_map<uint64_t, State*> closed_set;

        State* start_state = new State(initial_board);
        start_state->set_h(solver_heuristics->calculate(start_state->get_board()));
        
        open_set.push(start_state);
        closed_set[start_state->get_hash()] = start_state;

        int nodes_expanded = 0;

        while (!open_set.empty()) {
            State* current = open_set.top();
            open_set.pop();

            if (current->is_goal()) {
                std::cout << "Objetivo encontrado!\n";
                std::cout << "Custo total (movimentos): " << current->get_g() << "\n";
                std::cout << "Nos expandidos: " << nodes_expanded << "\n";
                
                std::vector<State*> path;
                State* step = current;
                while (step != nullptr) {
                    path.push_back(step);
                    step = step->get_parent();
                }
                std::reverse(path.begin(), path.end());
                
                for (const State* s : path) {
                    s->print();
                    std::cout << "-----\n";
                }

                for (auto& pair : closed_set) {
                    delete pair.second;
                }
                
                return;
            }

            nodes_expanded++;

            std::vector<State*> children = current->generate_successors(current);

            for (State* child : children) {
                uint64_t child_hash = child->get_hash();

                if (closed_set.find(child_hash) != closed_set.end()) {
                    delete child;
                } else {
                    child->set_h(solver_heuristics->calculate(child->get_board()));
                    closed_set[child_hash] = child;
                    open_set.push(child);
                }
            }
        }

        std::cout << "O algoritmo esgotou o espaco de busca e nao encontrou solucao.\n";
        
        for (auto& pair : closed_set) {
            delete pair.second;
        }
    }
};