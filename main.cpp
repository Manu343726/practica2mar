//
// Created by manu343726 on 20/05/15.
//

#include "graph.hpp"
#include <iostream>
#include <string>
#include <random>
#include <chrono>

struct node
{
    int id_;
    int color = -1;

    int id() const
    {
        return id_;
    }

    node(int id) :
        id_{ id }
    {}

    friend std::ostream& operator<<(std::ostream& os, const node& n)
    {
        return os << n.id();
    }
};

template<typename Node, bool node_debug_output = false, bool edge_debug_output = false>
auto random_graph(std::size_t nodes, float density)
{
    std::chrono::high_resolution_clock::time_point begin;
    std::mt19937 prng{ std::random_device{}() };
    std::uniform_int_distribution<std::size_t> dist{0, nodes-1};

    graph<Node> result;
    result.reserve(nodes);

    for(std::size_t i = 0; i < nodes; ++i)
    {
        if(node_debug_output)
        {
            std::cout << "Adding node (" << i << ")... ";
            begin = std::chrono::high_resolution_clock::now();
        }
        result.add_node(i);
        if(node_debug_output)
        {
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
            std::cout <<  elapsed.count() << "ms\n";
        }
    }

    for(std::size_t i = 0; i < nodes*density; ++i)
    {
        if(edge_debug_output)
            std::cout << "Generating edges (Pass " << i << ")\n";

        for(std::size_t j = 0; j < nodes; ++j)
        {
            std::size_t k = dist(prng);

            if(k != j)
                result(j,k) = true;
        }
    }

    return std::move(result);
}

auto non_colored_neighbors(const graph<node>& g, std::size_t node)
{
    return g.neighbors(node) | ranges::view::remove_if([](auto n)
           {
               return n.color >= 0;
           }) | ranges::view::bounded;
}

template<typename Range>
int first_color_available(Range nodes)
{
    int color = -1;

    for(auto node : ranges::view::bounded(nodes))
        color = std::max(color, node.color);

    return color;
}

int first_color_available(const graph<node>& g, std::size_t node)
{
    return first_color_available(g.neighbors(node));
}

int main()
{
    auto graph = random_graph<node, true, true>(1000,0.001);

    for(auto edge : graph.edges())
        std::cout << "(" << edge.first() << "," << edge.second() << ")" << std::endl;

    for(std::size_t i = 0; i < graph.nodes_count(); ++i) 
    {
        for(auto node : ranges::view::bounded(non_colored_neighbors(graph, 0)))
            std::cout << node << std::endl;

        std::cout << "First color: " << first_color_available(graph, i) << std::endl;
    }
}
