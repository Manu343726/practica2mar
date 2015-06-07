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
    std::vector<int> solution; // n colors, one per node
    std::size_t optimistic = 0;
    std::size_t pesimistic = 0;
    int last_color = 0;
    std::size_t k = 0;

    TreeNode() : id_{last_id_}
    {
        TreeNode::last_id_ += 1;
    }

    TreeNode(const std::vector<int>& colors, std::size_t optimistic_ = 0, std::size_t pesimistic_ = 0, std::size_t real_cost_ = 0) :
        id_{last_id_},
        solution{colors},
        optimistic{optimistic_},
        pesimistic{pesimistic_},
        last_color{real_cost_}
    {
        TreeNode::last_id_ += 1;
    }

    TreeNode(std::size_t nodes_count, std::size_t optimistic_ = 0, std::size_t pesimistic_ = 0, std::size_t real_cost_ = 0) :
        id_{last_id_},
        solution(nodes_count, -1),
        optimistic{optimistic_},
        pesimistic{pesimistic_},
        last_color{real_cost_}
    {
        TreeNode::last_id_ += 1;
    }

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

    std::size_t id() const
    {
        return id_;
    }

    friend std::ostream& operator<<(std::ostream& os, const TreeNode& node)
    {
        return os << "Node (id=" << node.id() << ") "
                  << " - last color: " << node.last_color << ' '
                  << " - k: " << node.k << ' ';
    }

private:
    std::size_t id_;
    static std::size_t last_id_;
};

std::size_t TreeNode::last_id_ = 0;

template <class T, class S, class C>
S& Container(std::priority_queue<T, S, C>& q) {
    struct HackedQueue : private std::priority_queue<T, S, C> {
        static S& Container(std::priority_queue<T, S, C>& q) {
            return q.*&HackedQueue::c;
        }
    };
    return HackedQueue::Container(q);
}

template<typename Node, typename Optimistic, typename Pesimistic, typename Bool>
std::vector<int> colorize(const graph<Node>& graph, Optimistic optimistic, Pesimistic pesimistic, Bool debug_output = std::false_type{})
{
    TreeNode root{graph.nodes_count()};
    root.last_color = 0;
    root.k = 0;
    root[root.k] = 0;
    root.pesimistic = pesimistic(root);
    root.optimistic = optimistic(root);

    std::priority_queue<TreeNode> queue;
    queue.push(root);

    std::size_t best_cost = graph.nodes_count();
    std::vector<int> best_solution = root.solution;
    const std::size_t threshold = graph.nodes_count()*graph.nodes_count();

    while(!queue.empty() && queue.top().optimistic < best_cost)
    {
        //Having an infinite growing queue has no sense since as it continues growing
        //last nodes are less likely to be selected. Then define a size threshold to prevent
        //queue keep growing.
        if(queue.size() > threshold)
        {
            if(queue.size() == threshold + 1)
                queue.pop();
            else
            {
                decltype(queue) tmp;
                Container(tmp).reserve(threshold/2);

                for(std::size_t i = 0; i < threshold/2; ++i)
                {
                    tmp.push(std::move(queue.top()));
                    queue.pop();
                }

                std::swap(queue, tmp);
            }
        }

        if(debug_output())
        {
            std::cout << "Queue size: " << queue.size()
                      << " best cost: " << best_cost << ' '
                      << queue.top() << '\n';
        }

        TreeNode x = std::move(queue.top()); queue.pop();

        auto neighbors = [&](std::size_t node)
        {
            return graph.neighbors(node) | ranges::view::remove_if([&](const auto& neighbor)
            {
                return x[neighbor] < 0; //discard colored nodes
            });
        };

        auto colors = [&](std::size_t node)
        {
            return neighbors(node) | ranges::view::transform([&](const auto& neighbor)
            {
                return x[neighbor];
            });
        } ;

        for(int color = 0; color <= x.last_color+1; ++color)
        {
            auto neighbors_colors = colors(x.k+1);

            if(ranges::find(neighbors_colors, color) == std::end(neighbors_colors))
            {
                TreeNode y = x;
                y.k += 1;
                y[y.k] = color;
                y.last_color = std::max(color, x.last_color);
                y.optimistic = optimistic(y);
                y.pesimistic = pesimistic(y);

                if(y.optimistic < best_cost)
                {
                    if(y.k == graph.nodes_count() - 1)
                    {
                        best_cost = y.last_color;
                        best_solution = std::move(y.solution);

                        if(debug_output())
                        {
                            std::cout << "NEW SOLUTION: " << y << '\n';
                        }
                    }
                    else
                    {
                        queue.push(std::move(y));
                        best_cost = std::min(best_cost, pesimistic(y));

                        std::cout << "NEW PARTIAL SOLUTION: " << y << '\n';
                    }
                }
            }
        }
    }

    return best_solution;
}

int main(int argn, const char* argv[])
{
    std::size_t nodes = argn > 1 ? std::atoi(argv[1]) : 10;

    auto graph = random_graph<GraphNode, true, true>(nodes,std::log10(nodes));

    int max_color = -1;

    auto optimistic = [](const TreeNode& node)
    {
        return node.last_color;
    };

    auto pesimistic = [&](const TreeNode& node)
    {
        return graph.nodes_count();
    };

    auto colors = colorize(graph, optimistic, pesimistic, std::true_type{});

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
