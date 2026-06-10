#pragma once
#include <vector>

class IAlgorithm {
public:
    virtual ~IAlgorithm() = default;
    
    
    virtual void solve(const std::vector<int>& initial_board) = 0;

    virtual int get_nodes_expanded() const = 0;

    virtual void print_stats() const = 0;

    virtual void print_solution() const = 0;
};