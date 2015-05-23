#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <utility>
#include <iterator>
#include <vector>
#include <stdexcept>
#include <cassert>
#include <ostream>

#include <manu343726/range/v3/all.hpp>
#include "utils.hpp"

struct adjacency_matrix {
private:
    struct node_proxy;

public:
    using edge_t = std::pair<std::size_t, std::size_t>;

    adjacency_matrix(bool directed = false) : _directed{ directed }
    {}

    adjacency_matrix(std::size_t nodes_count, bool directed = false) :
        _directed{directed} {
        _matrix.resize(nodes_count);

        for (auto &row : _matrix)
            row.resize(nodes_count, false); // allocate nodes_count sized row and fill it with "false"s
    }

    adjacency_matrix(std::initializer_list<std::initializer_list<int>> pairs, std::size_t nodes_count, bool directed = false) :
        adjacency_matrix{nodes_count, directed}
    {
        add_edges(pairs);
    }

    bool directed() const noexcept
    {
        return _directed;
    }

    std::size_t nodes_count() const
    {
        return _matrix.size();
    }

    void clear()
    {
        for (auto &row : _matrix)
            for (auto e : row)
                e = false;
    }

    auto edges() const
    {
        auto begin = [this](std::size_t i) -> std::size_t
        {
            return directed() ? 0u : i;
        };

        return ranges::view::for_each(ranges::view::iota(0u, nodes_count() - 1), [=](std::size_t i)
        {
            std::size_t b = begin(i);

            return ranges::view::for_each(ranges::view::iota(b, nodes_count() - 1), [=](std::size_t j)
            {
                return ranges::yield_if(_matrix[i][j], std::make_pair(i, j));
            });
        }) | ranges::view::bounded;
    }

    void add_edges(std::initializer_list<std::initializer_list<int>> pairs) {
        _apply_edges(pairs, true);
    }

    void remove_edges(std::initializer_list<std::initializer_list<int>> pairs) {
        _apply_edges(pairs, false);
    }

    void edges(std::initializer_list<std::initializer_list<int>> pairs) {
        clear();
        add_edges(pairs);
    }

    bool operator()(std::size_t i, std::size_t j) const noexcept {
        return _matrix[i][j];
    }

    node_proxy operator()(std::size_t i, std::size_t j) noexcept {
        return {*this, i, j};
    }

    bool at(std::size_t i, std::size_t j) const {
        if (i < nodes_count() && j < nodes_count())
            return (*this)(i, j);
        else
            throw std::out_of_range{"adjacency_matrix::at(i,j): Index out of range"};
    }

    node_proxy at(std::size_t i, std::size_t j) {
        if (i < nodes_count() && j < nodes_count())
            return (*this)(i, j);
        else
            throw std::out_of_range{"adjacency_matrix::at(i,j): Index out of range"};
    }

    auto neighbors(std::size_t node) const
    {
        return ranges::view::iota(0u, nodes_count() - 1) |
               ranges::view::remove_if([=](int i){ return !_matrix[node][i]; }) |
               ranges::view::bounded;
    }

    void reserve(std::size_t count)
    {
        _matrix.reserve(count);
    }

    void add_node()
    {
        _matrix.emplace_back(nodes_count() + 1, false);
    }

    friend std::ostream& operator<<(std::ostream& os, const adjacency_matrix& m)
    {
        for(std::size_t i = 0; i < m.nodes_count(); ++i)
        {
            os << "node " << i << ": ";

            for(std::size_t j = 0; j< m.nodes_count(); ++j)
                os << "[" << j << "," << std::boolalpha << m._matrix[i][j] << "] ";

            os << "\n";
        }

        return os;
    }

private:
    void _apply_edges(std::initializer_list<std::initializer_list<int>> pairs, bool value)
    {
        for(auto pair : pairs)
        {
            assert(std::end(pair) - std::begin(pair) == 2);

            int a = *(std::begin(pair));
            int b = *(std::begin(pair) + 1);

            _matrix[a][b] = value;

            if(!directed())
                _matrix[b][a] = value;
        }
    }

    struct node_proxy
    {
        node_proxy(adjacency_matrix& matrix, std::size_t _i, std::size_t _j) :
            _ref{matrix},
            i{_i},
            j{_j}
        {}

        bool operator=(bool b)
        {
            _ref.get()._matrix[i][j] = b;

            if(!_ref.get().directed())
                _ref.get()._matrix[j][i] = b;

            return b;
        }

        std::reference_wrapper<adjacency_matrix> _ref;
        std::size_t i, j;
    };

    std::vector<std::vector<bool>> _matrix;
    bool _directed;
};

template<typename Node>
struct graph {
    graph(bool directed = false) : _matrix{ directed }
    {};

    struct edge_t
    {
        std::size_t first_index, second_index;
        const Node* first_ptr;
        const Node* second_ptr;

        edge_t(std::size_t i, std::size_t j, const Node* first_, const Node* second_) :
            first_index{ i },
            second_index{ j },
            first_ptr{ first_ },
            second_ptr{ second_ }
        {};

        const Node& first() const
        {
            return *first_ptr;
        }

        const Node& second() const
        {
            return *second_ptr;
        }
    };

    void reserve(std::size_t count)
    {
        _matrix.reserve(count);
        _nodes.reserve(count);
    }

    template<typename... Args>
    void add_node(Args&&... args)
    {
        _nodes.emplace_back(std::forward<Args>(args)...);
        _matrix.add_node();
    }

    auto neighbors(std::size_t node) const
    {
        return ranges::view::transform(_matrix.neighbors(node), [this](std::size_t i)
        {
            return _nodes[i];
        }) | ranges::view::bounded;
    }

    auto edges() const
    {
        return ranges::view::transform(_matrix.edges(), [this](auto edge)
        {
            return edge_t{edge.first, edge.second,
                          &_nodes[edge.first], &_nodes[edge.second]};
        }) | ranges::view::bounded;
    }

    const adjacency_matrix& adjacency() const
    {
        return _matrix;
    }

    adjacency_matrix& adjacency()
    {
        return _matrix;
    }

    using node_id_t = decltype(std::declval<Node>().id());

    auto node_index(const node_id_t& id) const
    {
        auto it = std::find_if(std::begin(_nodes), std::end(_nodes), [=](const Node& n)
        {
            return id == n.id();
        });

        return it - std::begin(_nodes);
    }

    auto operator()(const node_id_t& a, const node_id_t& b) const
    {
        return _matrix(node_index(a), node_index(b));
    }

    auto operator()(const node_id_t& a, const node_id_t& b)
    {
        return _matrix(node_index(a), node_index(b));
    }

private:
    std::vector<Node> _nodes;
    adjacency_matrix _matrix;

public:
    METHOD_FROM(directed, _matrix)
    METHOD_FROM(add_edges, _matrix)
    METHOD_FROM(remove_edges, _matrix)
    METHOD_FROM(operator(), _matrix)
    METHOD_FROM(at, _matrix)
};

#endif /* GRAPH_HPP */