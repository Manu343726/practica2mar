#define NDEBUG
#include "value_wrapper.hpp"
#include "ptr_semantics.hpp"
#include "poly_allocator.hpp"

#include <iostream>
#include <vector>
#include <thread>

struct base
{
    virtual ~base() = default;
    virtual void hello() const = 0;
};

struct derived1 : base
{
    derived1(int i = 0) : i_{i}
    {}

    void hello() const override
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

    void hello() const override
    {
        std::cout << "derived2! c=" << c_ << std::endl;
    }

private:
    char c_ = 'a';
};

using value_t = value_wrapper<ptr_semantics<base, poly_allocator<base>>>;

template<typename T>
void print_as_words(const T* buff, std::size_t size)
{
    const char* ptr = reinterpret_cast<const char*>(buff);

    for(std::size_t i = 0; i < size*sizeof(T); i += sizeof(std::int32_t))
    {
        auto word = *reinterpret_cast<const std::int32_t*>(ptr + i);

        std::cout << std::dec << "[word " << i/sizeof(std::int32_t) << ": " << std::hex << word << "]" << std::endl;
    }

}

int main()
{
    constexpr std::size_t n = sizeof(value_t)/sizeof(base&);
    std::vector<value_t> v;
    //v.reserve(10000/2);

    value_t v1{ derived1{0} };

    for(std::size_t i = 0; i < n; ++i)
    {
        std::cout << "value_type: " << typeid(typename decltype(v)::value_type).name() << std::endl;
        std::cout << std::dec << "i=" << i << " (sizeof(base&)=" << sizeof(base&)  << ", sizeof(value_t)=" << sizeof(value_t) << ")" << std::endl;
        std::cout << std::hex;
        for(const auto& e : v)
            e->hello();

        print_as_words(v.data(), v.size());
        std::this_thread::sleep_for(std::chrono::seconds(1));
        v.emplace_back(std::move(derived1{(int)0xBADA110C}));
    }

    for(const auto& e : v)
        e->hello();
}