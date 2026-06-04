#pragma once
#include <vector>

class IAlgorithm {
public:
    virtual ~IAlgorithm() = default;
    
    // Função padrão que todos os algoritmos terão para iniciar a busca
    virtual void solve(const std::vector<int>& initial_board) = 0;
};