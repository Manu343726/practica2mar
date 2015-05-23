#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <utility>
#include <iterator>
#include <vector>
#include <stdexcept>
#include <cassert>

#include <manu343726/range/v3/all.hpp>
#include "utils.hpp"

struct adjacency_matrix {
private:
    struct vertex_proxy;

public:
    using edge_t = std::pair<std::size_t, std::size_t>;

    adjacency_matrix(std::size_t nodes_count, bool directed = false) :
            _directed{directed} {
        _matrix.resize(nodes_count);

        for (auto &row : _matrix)
            row.resize(nodes_count, false); // allocate nodes_count sized row and fill it with "false"s
    }

    adjacency_matrix(std::initializer_list<std::initializer_list<int>> pairs, std::size_t nodes_count,
                     bool directed = false) :
            adjacency_matrix{nodes_count, directed} {
        add_edges(pairs);
    }

    bool directed() const noexcept {
        return _directed;
    }

    std::size_t nodes_count() const {
        return _matrix.size();
    }

    void clear() {
        for (auto &row : _matrix)
            for (auto e : row)
                e = false;
    }

    auto edges() const {
        auto begin = [this](std::size_t i)
        {
            return directed() ? 0u : i;
        };
        auto end   = [this](std::size_t i)
        {
            return directed() ? nodes_count() - 1 : nodes_count() - i;
        };

        return ranges::view::for_each(ranges::view::iota(0u, nodes_count() - 1), [=](std::size_t i)
        {
            return ranges::view::for_each(ranges::view::iota(begin(i), end(i)), [=](std::size_t j)
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

    vertex_proxy operator()(std::size_t i, std::size_t j) noexcept {
        return {*this, i, j};
    }

    bool at(std::size_t i, std::size_t j) const {
        if (i < nodes_count() && j < nodes_count())
            return (*this)(i, j);
        else
            throw std::out_of_range{"adjacency_matrix::at(i,j): Index out of range"};
    }

    vertex_proxy at(std::size_t i, std::size_t j) {
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

    struct vertex_proxy
    {
        vertex_proxy(adjacency_matrix& matrix, std::size_t _i, std::size_t _j) :
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
struct graph
{

private:
    adjacency_matrix _matrix;
};

#endif /* GRAPH_HPP */