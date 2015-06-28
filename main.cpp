#include "value_wrapper.hpp"
#include "ptr_semantics.hpp"
#include "poly_allocator.hpp"

#include <iostream>
#include <vector>

struct base
{
    virtual ~base() = default;
    virtual void hello() = 0;
};

struct derived1 : base
{
    derived1(int i = 0) : i_{i}
    {}

    void hello() override
    {
        std::cout << "derived1! i=" << i_ << std::endl;
    }

private:
    int i_ = 0;
};

struct derived2 : base
{
    derived2(char c = 0) : c_{c}
    {}

    void hello() override
    {
        std::cout << "derived2! c=" << c_ << std::endl;
    }

private:
    char c_ = 'a';
};

using value_t = value_wrapper<ptr_semantics<base, poly_allocator<base>>>;

int main()
{
    std::vector<value_t> v;

    value_t v1{ derived1{0} };

    v.emplace_back(derived1{0});
    v.emplace_back(derived2{'a'});
    v.emplace_back(derived1{1});
    v.emplace_back(derived2{'b'});

    for(const auto& e : v)
        e.get().hello();
}