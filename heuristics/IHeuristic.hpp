#pragma once
#include <vector>

class IHeuristic {
public:
    
    virtual ~IHeuristic() = default;

    
    virtual int calculate(const std::vector<int>& board) const = 0;
};