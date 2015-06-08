//
// Created by manu343726 on 8/06/15.
//

#ifndef PRACTICA2MAR_VALUE_WRAPPER_H
#define PRACTICA2MAR_VALUE_WRAPPER_H

#include <memory>
#include <allocator>

namespace utils
{
    template<typename Semantics>
    struct default_value_semantics
    {
        using value_type = typename Semantics::value_type;
        using handle_type = typename Semantics::handle_type;

        Semantics() :
            This{static_cast<Semantics*>(this)}
        {}

        handle_type copy(const handle_type& handle)
        {
            return This->construct(This->deref(handle));
        }

        handle_type move(handle_type&& handle)
        {
            return This->construct(std::move(This->deref(handle)));
        }

        handle_type& copy_assign(handle_type& handle, const handle_type& other)
        {
            This->deref(handle) = This->deref(other);
            return handle;
        }

        handle_type& move_assign(handle_type& handle, handle_type&& other)
        {
            This->deref(handle) = std::move(This->deref(other));
            return handle;
        }

    private:
        Semantics* This;
    };

    template<typename T, typename Allocator = std::allocator<T>>
    struct ptr_semantics : public default_value_semantics<ptr_semantics<T,Allocator>>
    {
        using value_type = typename Allocator::value_type;
        using handle_type = typename Allocator::ptr_type;

        ptr_constructor(const Allocator& alloc) :
                alloc_{alloc}
        {}

        template<typename... Args>
        handle_type construct(Args&&... args)
        {
            return alloc_.construct(alloc_.allocate(1), std::forward<Args>(args)...);
        }

        void destroy(handle_type handle)
        {
            alloc_.destroy(handle);
            alloc_.deallocate(handle);
        }

        const value_type& deref(handle_type handle) const
        {
            return *handle;
        }

        value_type& deref(handle_type handle)
        {
            return *handle;
        }

    private:
        Allocator alloc_;
    };

    template<typename T,
             typename Handle = T*
             typename Semantics = ptr_semantics<T>>
    struct value_wrapper
    {
        template<typename... Args>
        value_wrapper(Args&&... args) :
            handle_{semantics_.construct(std::foward<Args>(args)...)}
        {}

        template<tyename... Args>
        value_wrapper(const Semantics& semantics,
                      Args&&... args) :
            semantics_{semantics},
            handle_{semantics_.construct(std::foward<Args>(args)...)}
        {}

        value_wrapper(const value_wrapper& v) :
            semantics_{v.semantics_},
            handle_{semantics_.copy(v.handle_)}
        {}

        value_wrapper(value_wrapper&& v) :
            semantics_{std::move(v.semantics_)},
            handle_{semantics_.move(std::move(v.handle_))}
        {}

        value_wrapper& operator=(const value_wrapper& v)
        {
            semantics_ = v._semantics;
            semantics_.copy_assign(handle_, v._handle);

            return *this;
        }

        value_wrapper& operator=(value_wrapper&& v)
        {
            semantics_ = std::move(v._semantics);
            semantics_.move_assign(handle_, std::move(v._handle));

            return *this;
        }

        ~value_semantics()
        {
            semantics_.destroy(handle_);
        }

        const T& get() const
        {
            return semantics_.deref(handle_);
        }

        T& get()
        {
            return semantics_.deref(handle_);
        }

        operator const T&() const
        {
            return get();
        }

        operator T&()
        {
            return get();
        }

        T* operator->()
        {
            return &semantics_.deref(handle_);
        }
    private:
        Handle handle_;
        Semantics semantics_;
    };

    template<typename Base>
    struct polymorphic_ptr
    {
        template<typename Derived, typename... Args>
        polymorphic_ptr(Args&&... args) :
            ptr_{new Derived{std::forward<Args>(args)...}}
        {}

        polymorphic_ptr(Base* ptr) :
            ptr_{ptr}
        {}

        ~polymorphic_ptr()
        {
            delete ptr_;
        }
    private:
        Base* ptr_ = nullptr;
    };
}

#endif //PRACTICA2MAR_VALUE_WRAPPER_H
