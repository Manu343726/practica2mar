//
// Created by manu343726 on 8/06/15.
//

#ifndef PRACTICA2MAR_POLYMORPHIC_HPP
#define PRACTICA2MAR_POLYMORPHIC_HPP

#include <memory>
#include <unordered_map>
#include <typeindex>
#include <iostream>
#include <cassert>

template<typename Base, template<typename...> class Alloc = std::allocator>
struct poly_allocator
{
    using value_type = Base;
    using pointer = Base*;


    template<typename T>
    pointer allocate(std::size_t count, const T& type_hint_, typename std::enable_if<!std::is_same<value_type, std::decay_t<T>>::value>::type* = nullptr)
    {
        return get_alloc_<T>().allocate(count);
    }

    template<typename T>
    pointer allocate(std::size_t count, const T& type_hint_, typename std::enable_if<std::is_same<value_type, std::decay_t<T>>::value>::type* = nullptr)
    {
        return get_alloc_(&type_hint_).allocate(count);
    }

    void deallocate(pointer ptr, std::size_t count)
    {
        get_alloc_(ptr).deallocate(ptr, count);
    }

    template<typename T>
    void construct(pointer ptr, const T& value)
    {
        get_alloc_(&value).construct(ptr, value);
    }

    template<typename T>
    void construct(pointer ptr, T&& value)
    {
        get_alloc_(&value).construct(ptr, std::move(value));
    }

    void destroy(pointer ptr)
    {
        get_alloc_(ptr).destroy(ptr);
    }

    struct polymorphic_allocator_tag {};

private:
    struct allocator_base
    {
        virtual ~allocator_base() = default;

        virtual Base* allocate(std::size_t count) = 0;
        virtual void deallocate(Base* ptr, std::size_t count) = 0;
        virtual void construct(Base* ptr, Base&& value) = 0;
        virtual void construct(Base* ptr, const Base& value) = 0;
        void destroy(Base* ptr)
        {
            ptr->~Base();
        }
    };

    template<typename Derived>
    struct allocator : allocator_base
    {
        Base* allocate(std::size_t count) override
        {
            return alloc_.allocate(count);
        }

        void deallocate(Base* ptr, std::size_t count) override
        {
            alloc_.deallocate(reinterpret_cast<Derived*>(ptr), count);
        }

        void construct(Base* ptr, Base&& value) override
        {
            Derived&& value_ = dynamic_cast<Derived&&>(value);
            Derived* ptr_ = reinterpret_cast<Derived*>(ptr);

            new (ptr_) Derived(std::move(value_));
        }

        void construct(Base* ptr, const Base& value) override
        {
            const Derived& value_ = dynamic_cast<const Derived&>(value);
            Derived* ptr_ = reinterpret_cast<Derived*>(ptr);

            new (ptr_) Derived(std::move(value_));
        }
    private:
        Alloc<Derived> alloc_;
    };

    allocator_base& get_alloc_(const Base* ptr)
    {
        static const Base* last_ptr_ = nullptr;
        static allocator_base* alloc_ = nullptr;

        if(ptr != last_ptr_ || alloc_ == nullptr)
        {
            const auto& tinfo = typeid(*ptr);
#if !defined(NDEBUG)
            std::cout << std::hex
                      << __PRETTY_FUNCTION__ << std::endl
                      << "Arena: " << allocs_ptr_ << std::endl
                      << "Current arena entries:" << std::endl;

            for(const auto& entry : allocs_())
                std::cout << "[" << entry.first.name() << "]: " << entry.second.get() << std::endl;
#endif
            alloc_ = allocs_()[tinfo].get();
            last_ptr_ = ptr;

#if !defined(NDEBUG)
            std::cout << "Asking for [" << tinfo.name() << "] allocator." << std::endl
                      << "  allocator at" << std::hex << allocs_()[tinfo].get() << std::endl;
#endif
        }

        return *alloc_;
    }

    template<typename T>
    allocator_base& get_alloc_()
    {
        const auto& tinfo = typeid(T);

#if !defined(NDEBUG)
        std::cout << std::hex
                  << __PRETTY_FUNCTION__ << std::endl
                  << "Arena: " << allocs_ptr_ << std::endl
                  << "Current arena entries:" << std::endl;

        for(const auto& entry : allocs_())
            std::cout << "[" << entry.first.name() << "]: " << entry.second.get() << std::endl;
#endif
        if(allocs_().find(tinfo) == std::end(allocs_())) {
            static_assert(!std::is_same<T, Base>::value, "Instancing Base of the hierarchy");
            allocs_()[tinfo] = std::make_unique<allocator<T>>();
        }

#if !defined(NDEBUG)
        std::cout << "Asking for [" << tinfo.name() << "] allocator." << std::endl
                  << "  allocator at" << std::hex << allocs_()[tinfo].get() << std::endl;
#endif

        return *(allocs_()[tinfo]);
    }

public:
    using arena_t = std::unordered_map<std::type_index, std::unique_ptr<allocator_base>>;

    poly_allocator() :
        allocs_ptr_{&poly_allocator::default_arena_}
    {
#if !defined(NDEBUG)
        std::cout << std::hex
                  << __PRETTY_FUNCTION__ << std::endl
                  << "Arena: " << allocs_ptr_ << std::endl
                  << "Current arena entries:" << std::endl;

        for(const auto& entry : allocs_())
            std::cout << "[" << entry.first.name() << "]: " << entry.second.get() << std::endl;
#endif
    }

    poly_allocator(arena_t* arena) :
            allocs_ptr_{arena}
    {
#if !defined(NDEBUG)
        std::cout << std::hex
                  << __PRETTY_FUNCTION__ << std::endl
                  << "Arena: " << allocs_ptr_ << std::endl
                  << "Current arena entries:" << std::endl;

        for(const auto& entry : allocs_())
            std::cout << "[" << entry.first.name() << "]: " << entry.second.get() << std::endl;
#endif
    }

    poly_allocator(const poly_allocator&) noexcept = default;
    poly_allocator(poly_allocator&&) = default;
    poly_allocator& operator=(const poly_allocator&) = default;
    poly_allocator& operator=(poly_allocator&&) = default;
    ~poly_allocator() = default;

    void rebind_arena(arena_t* arena)
    {
        allocs_ptr_ = arena;
    }

public:
    static arena_t default_arena_;
    arena_t* allocs_ptr_ = nullptr;

    arena_t& allocs_()
    {
        if(allocs_ptr_ == nullptr)
            allocs_ptr_ = &default_arena_;

        return *allocs_ptr_;
    }
};

template<typename Alloc>
using arena_t = typename Alloc::arena_t;

template<typename Base, template<typename...> class Alloc>
std::unordered_map<std::type_index,
                   std::unique_ptr<typename  poly_allocator<Base,Alloc>::allocator_base>
> poly_allocator<Base,Alloc>::default_arena_;

#endif //PRACTICA2MAR_POLYMORPHIC_HPP
