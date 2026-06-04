#pragma once
#include <vector>

class IHeuristic {
public:
    // Destrutor virtual é obrigatório em interfaces C++ para evitar vazamento de memória
    virtual ~IHeuristic() = default;

    // O "= 0" transforma isso numa função virtual pura (Interface)
    virtual int calculate(const std::vector<int>& board) const = 0;
};