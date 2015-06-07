//
// Created by manu343726 on 20/05/15.
//

#include "graph.hpp"
#include <iostream>
#include <string>
#include <random>
#include <chrono>
#include <numeric>
#include <queue>

struct GraphNode
{
    int color = -1;
};

struct TreeNode
{
    static std::size_t new_id()
    {
        static std::size_t id = 0;

        return id++;
    }

    std::vector<int> solution; // n colors, one per node
    int optimistic = 0;
    int pessimistic = 0;
    int last_color = 0;
    std::size_t k = 0;
    std::size_t id = new_id();

    TreeNode() = default;

    TreeNode(const std::vector<int>& colors, int optimistic_ = 0, int pessimistic_ = 0, int real_cost_ = 0) :
        solution{colors},
        optimistic{optimistic_},
        pessimistic{pessimistic_},
        last_color{real_cost_}
    {}

    TreeNode(std::size_t nodes_count, int optimistic_ = 0, int pessimistic_ = 0, int real_cost_ = 0) :
        solution(nodes_count, -1),
        optimistic{optimistic_},
        pessimistic{pessimistic_},
        last_color{real_cost_}
    {}

    template<typename Node>
    int operator[](const Node& node) const
    {
        return solution[node.id()];
    }

    template<typename Node>
    int& operator[](const Node& node)
    {
        return solution[node.id()];
    }

    int operator[](std::size_t i) const
    {
        return solution[i];
    }

    int& operator[](std::size_t i)
    {
        return solution[i];
    }

    friend bool operator<(const TreeNode& lhs, const TreeNode& rhs)
    {
        return lhs.optimistic > rhs.optimistic;
    }

    friend std::ostream& operator<<(std::ostream& os, const TreeNode& node)
    {
        return os << "(opt=" << node.optimistic << ",pes=" << node.pessimistic
                  << ",col=" << node.last_color << ",k=" << node.k
                  << ",id=" << node.id << ')';
    }
};

template <class T, class S, class C>
S& Container(std::priority_queue<T, S, C>& q) {
    struct HackedQueue : private std::priority_queue<T, S, C> {
        static S& Container(std::priority_queue<T, S, C>& q) {
            return q.*&HackedQueue::c;
        }
    };
    return HackedQueue::Container(q);
}

template<typename T,
         typename Container = std::vector<T>,
         typename Comp = std::less<typename Container::value_type>
        >
struct priority_queue : public std::priority_queue<T, Container, Comp>
{
    using base = std::priority_queue<T, Container, Comp>;

    template<typename Pred>
    void remove_if(Pred predicate)
    {
        base::c.erase(std::remove_if(std::begin(base::c), std::end(base::c), predicate), std::end(base::c));
        std::make_heap(std::begin(base::c), std::end(base::c), base::comp);
    }
};

using my_clock_t = std::chrono::high_resolution_clock;
using my_duration_t = my_clock_t::duration;

struct TestResults
{
    my_duration_t runt_time = my_duration_t::zero();
    std::size_t nodes_expanded = 0;
    std::size_t nodes_discarded = 0;
    std::size_t solutions_found = 0;
    std::vector<int> solution;
    int last_color = -1;
};

template<typename Node, typename Optimistic, typename Pessimistic, typename Int>
TestResults colorize(const graph<Node>& graph, Optimistic optimistic, Pessimistic pessimistic, Int debug_output = std::integral_constant<int,0>{})
{
    auto begin = my_clock_t::now();

    TreeNode root{graph.nodes_count()};
    root.last_color = 0;
    root.k = 0;
    root[root.k] = 0;
    root.pessimistic = pessimistic(root);
    root.optimistic = optimistic(root);

    priority_queue<TreeNode> queue;
    queue.push(root);

    int best_cost = pessimistic(root);
    TreeNode best_solution = root;
    const std::size_t threshold = std::numeric_limits<std::size_t>::max();
    TestResults results;

    while(!queue.empty() && queue.top().optimistic < best_cost)
    {
        //Having an infinite growing queue has no sense since as it continues growing
        //last nodes are less likely to be selected. Then define a size threshold to prevent
        //queue keep growing. This may discard solutions, but I have no infinite RAM
        if(queue.size() > threshold)
        {
            decltype(queue) tmp;
            Container(tmp).reserve(threshold/4);

            for(std::size_t i = 0; i < threshold/4; ++i)
            {
                tmp.push(std::move(queue.top()));
                queue.pop();
            }

            std::swap(queue, tmp);
        }

        TreeNode x = std::move(queue.top()); queue.pop();

        auto colors = [&](std::size_t node)
        {
            return graph.neighbors(node)
                | ranges::view::transform([&](const auto& neighbor)
                {
                    return x[neighbor];
                })
                | ranges::view::remove_if([&](int color)
                {
                    return color < 0;
                });
        };

        for(int color = 0; color <= x.last_color+1; ++color)
        {
            auto neighbors_colors = colors(x.k+1);

            if(ranges::find(neighbors_colors, color) == std::end(neighbors_colors))
            {
                TreeNode y = x;
                y.id = TreeNode::new_id();
                y.k += 1;
                y[y.k] = color;
                y.last_color = std::max(color, x.last_color);
                y.optimistic = optimistic(y);
                y.pessimistic = pessimistic(y);

                assert(y.optimistic <= y.pessimistic);

                if(y.optimistic < best_cost)
                {
                    if(y.k == graph.nodes_count() - 1)
                    {
                        if(debug_output() >= 3)
                        {
                            std::cout << "NEW SOLUTION: " << y << '\n';
                        }

                        best_cost = y.last_color;
                        best_solution = std::move(y);
                        results.solutions_found++;
                    }
                    else
                    {
                        queue.push(std::move(y));
                        results.nodes_expanded++;
                    }
                }
                else
                    results.nodes_discarded++;
            }

            if(debug_output() >= 2)
            {
                std::cout << "size=" << queue.size()
                << " best=" << best_solution
                << " top=" << queue.top() << std::endl;
            }
        }
    }

    if(debug_output() >= 1)
    {
        std::cout << "FINISHED\n"
                  << "Queue size: " << queue.size()
                  << " best cost: " << best_cost << '\n'
                  << " best solution: " << best_solution << '\n'
                  << "nodes expanded: " << TreeNode::new_id() - 1 << '\n';

        if(!queue.empty())
            std::cout << "best remaining candidate: " << queue.top() << '\n';
    }

    results.runt_time = my_clock_t::now() - begin;
    results.solution = std::move(best_solution.solution);

    return results;
}

template<typename Node>
std::size_t guess_neighbors(const graph<Node> g, std::size_t node)
{
    std::size_t count = 0;

    auto node_neighbors = ranges::view::bounded(g.neighbors(node));
    std::size_t node_neighbors_count = 0;

    for(const auto& first_level : node_neighbors)
    {
        node_neighbors_count++;

        for(const auto& second_level : ranges::view::bounded(g.neighbors(first_level.id())))
        {
            auto it = ranges::find(node_neighbors, second_level);

            if(*it != first_level && it->id() != node && it != std::end(node_neighbors))
                count++;
        }
    }

    node_neighbors_count = std::max(static_cast<std::size_t>(1), node_neighbors_count);

    return count/node_neighbors_count;
}

int main(int argn, const char* argv[])
{
    std::size_t nodes = argn > 1 ? std::atoi(argv[1]) : 30;

    auto graph = random_graph<GraphNode, true, true>(nodes,std::log10(nodes));

    auto optimistic = [&](const TreeNode& node)
    {
        return node.last_color + guess_neighbors(graph, node.k);
    };

    auto pessimistic = [&](const TreeNode& node) -> int
    {
        return std::max(node.optimistic+1, static_cast<int>(node.last_color + graph.nodes_count() - node.k - 1));
    };

    auto optimistic_naive = [&](const TreeNode& node)
    {
        return node.last_color;
    };

    auto pessimistic_naive = [&](const TreeNode& node) -> int
    {
        return graph.nodes_count();
    };

    auto results = colorize(graph, optimistic, pessimistic, std::integral_constant<int,2>{});
    auto colors = std::move(results.solution);

    std::cout << "Run time: " << std::chrono::duration_cast<std::chrono::milliseconds>(results.runt_time).count() << " ms\n"
              << "Nodes expanded: " << results.nodes_expanded
              << "\nNodes discarded: " << results.nodes_discarded
              << "\nSolutions found: " << results.solutions_found << std::endl;

    int max_color = -1;
    for(std::size_t i = 0; i < nodes; ++i)
    {
        std::cout << "(" << i << ") color: " << colors[i]
                  << " neighbors: ";

        max_color = std::max(colors[i], max_color);

        for(auto neighbor : graph.neighbors(i))
            std::cout << "(" << neighbor.id() << "{c:" << colors[neighbor.id()] << "}) ";

        std::cout << std::endl;
    }

    std::cout << "Max color: " << max_color << std::endl;
}
