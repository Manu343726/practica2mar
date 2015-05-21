//
// Created by manu343726 on 20/05/15.
//

#include "graph.hpp"
#include <iostream>

int main()
{
    adjacency_matrix m{{{0,1}, {1,2}}, 3};

    m(0,2) = true;

    for(auto edge : m.edges())
        std::cout << "(" << edge.first << "," << edge.second << ")" << std::endl;

    //for(int i : m.neighbours(1))
      //  std::cout << i << std::endl;
}
