#pragma once
#include <vector>

class IAlgorithm {
public:
    virtual ~IAlgorithm() = default;
    
    
    virtual void solve(const std::vector<int>& initial_board) = 0;
};