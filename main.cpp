//
// Created by manu343726 on 20/05/15.
//

#include "graph.hpp"
#include <iostream>

int main()
{
    adjacency_matrix m{{{0,1}, {1,2}}, 3};

    for(auto edge : m.edges())
        std::cout << "(" << edge.first << "," << edge.second << ")" << std::endl;
}
