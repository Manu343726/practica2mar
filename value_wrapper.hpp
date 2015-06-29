//
// Created by manu343726 on 8/06/15.
//

#ifndef PRACTICA2MAR_VALUE_WRAPPER_H
#define PRACTICA2MAR_VALUE_WRAPPER_H

#include <memory>

template<typename Semantics>
struct value_wrapper
{
    using value_type = typename Semantics::value_type;
    using handle_type = typename Semantics::handle_type;

    template<typename Arg1, typename Arg2, typename... Tail>
    value_wrapper(Arg1&& arg1, Arg2&& arg2, Tail&&... tail) :
        handle_{semantics_.construct(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2), std::forward<Tail>(tail)...)}
    {}

    template<typename T, typename = typename std::enable_if<!std::is_same<typename std::decay<T>::type, value_wrapper>::value>::type>
    value_wrapper(T&& value) :
        handle_{semantics_.construct(std::forward<T>(value))}
    {}

    template<typename... Args>
    value_wrapper(const Semantics& semantics, Args&&... args) :
        semantics_{semantics},
        handle_{semantics_.construct(std::forward<Args>(args)...)}
    {}

    value_wrapper(const value_wrapper& v) :
        semantics_{v.semantics_},
        handle_{semantics_.copy(v.handle_)}
    {}

    value_wrapper(value_wrapper&& v) noexcept :
        semantics_{std::move(v.semantics_)},
        handle_{semantics_.move(v.handle_)}
    {}

    value_wrapper& operator=(const value_wrapper& v)
    {
        semantics_ = v._semantics;
        semantics_.copy_assign(handle_, v._handle);

        return *this;
    }

    value_wrapper& operator=(value_wrapper&& v) noexcept
    {
        semantics_ = std::move(v.semantics_);
        semantics_.move_assign(handle_, std::move(v.handle_));

        return *this;
    }

    template<typename T>
    value_wrapper& operator=(const T& value)
    {
        semantics_.copy_assign(handle_, value);

        return *this;
    }

    template<typename T>
    value_wrapper& operator=(T&& value)
    {
        semantics_.move_assign(handle_, std::move(value));

        return *this;
    }

    ~value_wrapper()
    {
        semantics_.destroy(handle_);
    }

    const value_type& get() const
    {
        return semantics_.deref(handle_);
    }

    value_type& get()
    {
        return semantics_.deref(handle_);
    }

    operator const value_type&() const
    {
        return get();
    }

    operator value_type&()
    {
        return get();
    }

    auto operator->() const -> decltype(&get())
    {
        return &(get());
    }

    auto operator->() -> decltype(&get())
    {
        return &(get());
    }
public:
    Semantics semantics_;
    handle_type handle_;
};

#endif //PRACTICA2MAR_VALUE_WRAPPER_H
