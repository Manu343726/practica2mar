//
// Created by manu343726 on 20/05/15.
//

#include "graph.hpp"
#include <iostream>
#include <random>

struct node
{
    std::string name;

    const std::string& id() const
    {
        return name;
    }

    node(const std::string& name_) :
        name{ name_ }
    {}

    template<typename T>
    node(T&& e) :
        name{std::to_string(std::forward<T>(e))}
    {}

    friend std::ostream& operator<<(std::ostream& os, const node& n)
    {
        return os << n.id();
    }
};

template<typename Node>
auto random_graph(std::size_t nodes, float density)
{
    std::mt19937 prng{ std::random_device{}() };
    std::uniform_int_distribution<std::size_t> dist{0, nodes-1};

    graph<Node> result;
    result.reserve(nodes);

    for(std::size_t i = 0; i < nodes; ++i)
        result.add_node(i);

    for(std::size_t i = 0; i < nodes*density; ++i)
    {
        for(std::size_t j = 0; j < nodes; ++j)
        {
            std::size_t k = dist(prng);

            if(k != j)
                result(j,k) = true;
        }
    }

    return result;
}

int main()
{
    std::cout << std::to_string(23.43) << std::endl;

    auto graph = random_graph<node>(1000,0.1);

    for(auto edge : graph.edges())
        std::cout << "(" << edge.first() << "," << edge.second() << ")" << std::endl;

    for(auto node : graph.neighbors(0))
        std::cout << node << std::endl;
}
