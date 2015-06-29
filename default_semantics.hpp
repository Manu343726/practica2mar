//
// Created by Manu3 on 6/28/2015.
//

#ifndef PRACTICA2MAR_DEFAULT_SEMANTICS_HPP
#define PRACTICA2MAR_DEFAULT_SEMANTICS_HPP

namespace default_semantics
{
    template<typename Handle>
    struct construct
    {
        template<typename... Args>
        Handle operator()(Args&&... args)
        {
            return Handle{std::forward<Args>(args)...};
        }
    };

    template<typename Handle>
    using copy = construct<Handle>;

    template<typename Handle>
    using move = construct<Handle>;

    template<typename Handle>
    struct copy_assign
    {
        Handle& operator()(Handle& lhs, const Handle& rhs)
        {
            return lhs = rhs;
        }
    };

    template<typename Handle>
    struct move_assign
    {
        Handle& operator()(Handle& lhs, Handle&& rhs)
        {
            return lhs = std::move(rhs);
        }
    };

    template<typename Handle>
    struct destroy
    {
        void operator()(Handle& handle)
        {
            //nop
        }
    };

    template<typename Handle>
    struct deref
    {
        Handle& operator()(Handle& handle)
        {
            return handle;
        }

        const Handle& operator()(const Handle& handle)
        {
            return handle;
        }
    };

    struct default_semantic_tag{} default_;
}

template<typename Handle,
        typename Construct  = default_semantics::construct<Handle>,
        typename Copy       = default_semantics::copy<Handle>,
        typename Move       = default_semantics::move<Handle>,
        typename CopyAssign = default_semantics::copy_assign<Handle>,
        typename MoveAssign = default_semantics::move_assign<Handle>,
        typename Destroy    = default_semantics::destroy<Handle>,
        typename Deref      = default_semantics::deref<Handle>
>
struct semantics_builder
{
    semantics_builder(Construct&& construct,
                      Copy&& copy,
                      Move&& move,
                      CopyAssign&& copy_assign,
                      MoveAssign&& move_assign,
                      Destroy&& destroy,
                      Deref&& deref)
            :
            construct_{std::forward<Construct>(construct)},
            copy_{std::forward<Copy>(copy)},
            move_{std::forward<Move>(move)},
            copy_assign_{std::forward<CopyAssign>(copy_assign)},
            move_assign_{std::forward<MoveAssign>(move_assign)},
            destroy_{std::forward<Destroy>(destroy)},
            deref_{std::forward<Deref>(construct)}
    {}

    using handle_type = Handle;
    using value_type = typename std::decay<decltype(deref(std::declval<Handle>()))>::type;

    template<typename... Args>
    Handle construct(Args&&... args)
    {
        return construct_(std::forward<Args>(args)...);
    }

    Handle copy(const Handle& rhs)
    {
        return copy_(rhs);
    }

    Handle move(Handle&& rhs)
    {
        return move_(std::move(rhs));
    }

    Handle& copy_assign(Handle& lhs, const Handle& rhs)
    {
        return copy_assign_(lhs, rhs);
    }

    Handle& move_assign(Handle& lhs, Handle&& rhs)
    {
        return move_assgin_(lhs, std::move(rhs));
    }

    void destroy(Handle& handle)
    {
        return destroy_(handle);
    }

    decltype(auto) deref(const Handle& handle)
    {
        return deref_(handle);
    }

    decltype(auto) deref(Handle& handle)
    {
        return deref_(handle);
    }

private:
    Construct construct_;
    Copy copy_;
    Move move_;
    CopyAssign copy_assign_;
    MoveAssign move_assign_;
    Destroy destroy_;
    Deref deref_;
};

//placeholders broken: types are ok, but values?
template<typename Handle,
        typename Construct,
        typename Copy,
        typename Move,
        typename CopyAssign,
        typename MoveAssign,
        typename Destroy,
        typename Deref
>
semantics_builder<
        Handle,
        typename std::conditional<std::is_same<typename std::decay<Construct>::type, default_semantics::default_semantic_tag>::value,
                default_semantics::construct<Handle>, typename std::decay<Construct>::type
        >::type,
        typename std::conditional<std::is_same<typename std::decay<Copy>::type, default_semantics::default_semantic_tag>::value,
                default_semantics::copy<Handle>, typename std::decay<Copy>::type
        >::type,
        typename std::conditional<std::is_same<typename std::decay<Move>::type, default_semantics::default_semantic_tag>::value,
                default_semantics::move<Handle>, typename std::decay<Move>::type
        >::type,
        typename std::conditional<std::is_same<typename std::decay<CopyAssign>::type, default_semantics::default_semantic_tag>::value,
                default_semantics::copy_assign<Handle>, typename std::decay<CopyAssign>::type
        >::type,
        typename std::conditional<std::is_same<typename std::decay<MoveAssign>::type, default_semantics::default_semantic_tag>::value,
                default_semantics::move_assign<Handle>, typename std::decay<MoveAssign>::type
        >::type,
        typename std::conditional<std::is_same<typename std::decay<Destroy>::type, default_semantics::default_semantic_tag>::value,
                default_semantics::destroy<Handle>, typename std::decay<Destroy>::type
        >::type,
        typename std::conditional<std::is_same<typename std::decay<Deref>::type, default_semantics::default_semantic_tag>::value,
                default_semantics::deref<Handle>, typename std::decay<Deref>::type
        >::type
> build_semantics(Construct&& construct,
                  Copy&& copy,
                  Move&& move,
                  CopyAssign&& copy_assign,
                  MoveAssign&& move_assign,
                  Destroy&& destroy,
                  Deref&& deref)
{
    return {std::forward<Construct>(construct),
            std::forward<Copy>(copy),
            std::forward<Move>(move),
            std::forward<CopyAssign>(copy_assign),
            std::forward<MoveAssign>(move_assign),
            std::forward<Destroy>(destroy),
            std::forward<Deref>(construct),
    };
}

template<typename Semantics,
        typename HandleType
>
struct default_value_semantics
{
    using handle_type = HandleType;

    default_value_semantics() :
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

    template<typename T>
    handle_type& copy_assign(handle_type& handle, const T& other)
    {
        This->deref(handle) = other;
        return handle;
    }

    template<typename T>
    handle_type& move_assign(handle_type& handle, T&& other)
    {
        This->deref(handle) = std::move(other);
        return handle;
    }

private:
    Semantics* This;
};

#endif //PRACTICA2MAR_DEFAULT_SEMANTICS_HPP
