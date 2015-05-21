#ifndef GRAPH_HPP
#define GRAPH_HPP

#include <utility>
#include <iterator>
#include <vector>
#include <stdexcept>
#include <cassert>

#include <manu343726/range/v3/all.hpp>



struct adjacency_matrix
{
    using edge_t = std::pair<std::size_t, std::size_t>;

    adjacency_matrix(std::size_t nodes_count, bool directed = false) :
        _directed{directed}
    {
        _matrix.resize(nodes_count);

        for(auto& row : _matrix)
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
        for(auto& row : _matrix)
            for(auto e : row)
                e = false;
    }

    std::vector<edge_t> edges() const
    {
        std::vector<edge_t> result;
        result.reserve(_matrix.size()*_matrix.size()/2); //Reserve enough memory first, guaranteed O(1) insertion

        for(std::size_t i = 0; i < nodes_count(); ++i)
            for(std::size_t j = i; j < nodes_count() - i + 1; ++j)
                if(_matrix[i][j])
                    result.emplace_back(i,j);

        return result;
    }

    void add_edges(std::initializer_list<std::initializer_list<int>> pairs)
    {
        _apply_edges(pairs, true);
    }

    void remove_edges(std::initializer_list<std::initializer_list<int>> pairs)
    {
        _apply_edges(pairs, false);
    }

    void edges(std::initializer_list<std::initializer_list<int>> pairs)
    {
        clear();
        add_edges(pairs);
    }

    bool operator()(std::size_t i, std::size_t j) const noexcept
    {
        return _matrix[i][j];
    }

    auto operator()(std::size_t i, std::size_t j) noexcept
    {
        return _matrix[i][j];
    }

    bool at(std::size_t i, std::size_t j) const
    {
        if(i < nodes_count() && j < nodes_count())
            return (*this)(i,j);
        else
            throw std::out_of_range{"adjacency_matrix::at(i,j): Index out of range"};
    }

    auto at(std::size_t i, std::size_t j)
    {
        if(i < nodes_count() && j < nodes_count())
            return (*this)(i,j);
        else
            throw std::out_of_range{"adjacency_matrix::at(i,j): Index out of range"};
    }

    auto neighbours(std::size_t node) const
    {
        return ranges::view::iota(0) |
               ranges::view::remove_if([](int i){ return _matrix[node][i];});
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