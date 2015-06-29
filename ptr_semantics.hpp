//
// Created by Manu3 on 6/28/2015.
//

#ifndef PRACTICA2MAR_PTR_SEMANTICS_HPP
#define PRACTICA2MAR_PTR_SEMANTICS_HPP

#include "default_semantics.hpp"

#include <memory>

namespace
{
    template<typename T>
    using void_t = typename std::conditional<true,void,T>::type;

    template<template<typename...> class T>
    struct template_ {};

    template<template<typename...> class T>
    using void_template = typename std::conditional<true,void,template_<T>>::type;


    template<typename T, typename = void>
    struct is_poly_allocator : std::false_type {};

    template<typename T>
    struct is_poly_allocator<T, void_t<typename T::polymorphic_allocator_tag>> : std::true_type {};
}


template<typename T, typename Allocator = std::allocator<T>>
struct ptr_semantics : public default_value_semantics<ptr_semantics<T,Allocator>,T*>
{
    using value_type = T;
    using handle_type = T*;

    ptr_semantics(const Allocator& alloc = Allocator{}) :
            alloc_{alloc}
    {}

    template<typename Arg1, typename Arg2, typename... Tail>
    handle_type construct(Arg1&& arg1, Arg2&& arg2, Tail&&... tail)
    {
        auto ptr_ = alloc_.allocate(1);
        alloc_.construct(ptr_, std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Tail>(tail)...);
        return ptr_;
    }

    template<typename Arg, typename Alloc_ = Allocator>
    handle_type construct(Arg&& value, typename std::enable_if<is_poly_allocator<Alloc_>::value>::type* = nullptr)
    {
        auto ptr_ = alloc_.allocate(1, value);
        alloc_.construct(ptr_, std::forward<Arg>(value));
        return ptr_;
    }

    template<typename Arg, typename Alloc_ = Allocator>
    handle_type construct(Arg&& arg, typename std::enable_if<!is_poly_allocator<Alloc_>::value>::type* = nullptr)
    {
        auto ptr_ = alloc_.allocate(1);
        alloc_.construct(ptr_, std::forward<Arg>(arg));
        return ptr_;
    }

    handle_type move(handle_type& handle)
    {
        handle_type ptr = handle;
        handle = nullptr;
        return ptr;
    }

    void destroy(handle_type handle)
    {
        alloc_.destroy(handle);
        alloc_.deallocate(handle, 1);
    }

    const value_type& deref(handle_type handle) const
    {
        return *handle;
    }

    value_type& deref(handle_type handle)
    {
        return *handle;
    }

public:
    Allocator alloc_;
};


#endif //PRACTICA2MAR_PTR_SEMANTICS_HPP
