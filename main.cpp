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
    int color = -1;
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
        result.add_node();
        if(node_debug_output)
        {
            auto end = std::chrono::high_resolution_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin);
            std::cout <<  elapsed.count() << "ms\n";
        }
    }

    for(std::size_t i = 0; i < density; ++i)
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

template<typename Node>
auto non_colored_neighbors(const graph<Node>& g, std::size_t node)
{
    return g.neighbors(node) | ranges::view::remove_if([](auto n)
    {
       return n.color >= 0;
    });
}

template<typename Node>
auto non_colored_neighbors(graph<Node>& g, std::size_t node)
{
    return g.neighbors(node) | ranges::view::remove_if([](auto n)
    {
       return n.color >= 0;
    });
}

template<typename Range>
int first_color_available(Range nodes)
{
    int color = 0;

    for(auto node : ranges::view::bounded(nodes))
        color = std::max(color, node.color);

    return color;
}

template<typename Node>
int first_color_available(const graph<Node>& g, std::size_t node)
{
    return first_color_available(g.neighbors(node));
}

template<typename Range>
std::size_t length(Range r)
{
    return std::end(r) - std::begin(r);
}

template<typename Node>
void color_graph_naive(graph<Node>& g, std::size_t node)
{
    auto& current = g(node);

    if(current.color < 0)
        current.color = first_color_available(g, node);

    auto neighbors = non_colored_neighbors(g, node);

    for (auto &neighbor : neighbors)
    {
        neighbor.color = current.color + 1;
        color_graph_naive(g, neighbor);
    }
}

template<typename Node>
bool connected(const graph<Node>& g)
{
    return ranges::all_of(g.nodes() | ranges::view::bounded, [&](const auto& node)
    {
       return !ranges::empty(g.neighbors(node.id()));
    });
}

int main(int argn, const char* argv[])
{
    std::size_t nodes = argn > 1 ? std::atoi(argv[1]) : 10;

    auto graph = random_graph<node, true, true>(nodes,std::log10(nodes));

    if(!connected(graph))
        std::cout << "WARNING: Non-connected graph" << std::endl;

    color_graph_naive(graph, 0);

    int max_color = -1;

    for(std::size_t i = 0; i < nodes; ++i)
    {
        std::cout << "(" << i << ") color: " << graph(i).color
                  << " neighbors: ";

        max_color = std::max(graph(i).color, max_color);

        for(auto neighbor : graph.neighbors(i))
            std::cout << "(" << neighbor.id() << ") ";

        std::cout << std::endl;
    }

    std::cout << "Max color: " << max_color << std::endl;
}
