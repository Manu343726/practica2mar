//
// Created by manu343726 on 8/06/15.
//

#ifndef PRACTICA2MAR_POLYMORPHIC_HPP
#define PRACTICA2MAR_POLYMORPHIC_HPP

#include <memory>
#include <unordered_map>
#include <typeindex>

template<typename Base, template<typename...> class Alloc = std::allocator>
struct poly_allocator
{
    using value_type = Base&;
    using pointer = Base*;

    template<typename T>
    Base* allocate(std::size_t count, const T& type_hint_, typename std::enable_if<!std::is_same<value_type, std::add_lvalue_reference_t<std::decay_t<T>>>::value>::type* = nullptr)
    {
        return get_alloc_<T>().allocate(count);
    }

    template<typename T>
    Base* allocate(std::size_t count, const T& type_hint_, typename std::enable_if<std::is_same<value_type, std::add_lvalue_reference_t<std::decay_t<T>>>::value>::type* = nullptr)
    {
        return get_alloc_(&type_hint_).allocate(count);
    }

    void deallocate(Base* ptr, std::size_t count)
    {
        get_alloc_(ptr).deallocate(ptr, count);
    }

    template<typename T>
    void construct(Base* ptr, const T& value)
    {
        get_alloc_(&value).construct(ptr, value);
    }

    template<typename T>
    void construct(Base* ptr, T&& value)
    {
        get_alloc_(&value).construct(ptr, std::move(value));
    }

    void destroy(Base* ptr)
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

    template<typename Derived, typename = typename std::enable_if<!std::is_same<Base,Derived>::value>::type>
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
            alloc_ = allocs_[typeid(*ptr)].get();
            last_ptr_ = ptr;
        }

        return *alloc_;
    }

    template<typename T>
    allocator_base& get_alloc_()
    {
        const auto& tinfo = typeid(T);

        if(allocs_.find(tinfo) == std::end(allocs_))
            allocs_[tinfo] = std::make_unique<allocator<T>>();

        return *(allocs_[tinfo]);
    }

    static std::unordered_map<std::type_index, std::unique_ptr<allocator_base>> allocs_; //I'm a lazy bastard...
};

template<typename Base, template<typename...> class Alloc>
std::unordered_map<std::type_index, std::unique_ptr<typename poly_allocator<Base,Alloc>::allocator_base>> poly_allocator<Base,Alloc>::allocs_;

#endif //PRACTICA2MAR_POLYMORPHIC_HPP
