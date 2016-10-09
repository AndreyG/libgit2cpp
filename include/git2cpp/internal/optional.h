#pragma once

#ifdef USE_BOOST
    #include <boost/optional.hpp>
    #include <boost/utility/typed_in_place_factory.hpp>
#else
    #include <memory>
    #include <cstddef>
#endif

namespace git {
namespace internal
{
#ifdef USE_BOOST
    using boost::optional;
    using boost::in_place;
    using boost::none_t;
    using boost::none;

    template<typename T, typename ...Args>
    optional<T> & emplace(optional<T> & opt, Args&&... args){
        opt = in_place<T>(std::forward<Args>(args)...);
        return opt;
    }
#else
    typedef nullptr_t none_t;
    const constexpr none_t none = nullptr;

    //Simple (and incomplete) optional implementations
    template<typename T>
    class optional
    {
    private: //type alias
        typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type StorageT;

    public: //type traits
        enum non_movable_t{};
        enum non_copyable_t{};

        static const constexpr bool movable = std::is_move_constructible<T>::value && std::is_move_assignable<T>::value;
        static const constexpr bool copyable = std::is_copy_constructible<T>::value && std::is_copy_assignable<T>::value;
        typedef typename std::conditional<movable, T, non_movable_t>::type MovableT;
        typedef typename std::conditional<copyable, T, non_copyable_t>::type CopyableT;

    public: //initializing and attribution
        constexpr optional(none_t t = none)
            : initialized(false)
        {}

        optional(T const & v) { assign(v);}
        optional(T      && v) { assign(std::move(v));}

        optional(optional const &  other) { assign(other);}
        optional(optional       && other) { assign(std::move(other));}

        optional & operator=(none_t none)  { reset(); return *this;}
        optional & operator=(T && v)       { return assign(std::move(v));}
        optional & operator=(const T & v)  { return assign(v);}

        optional & operator=(optional const &  other){ return assign(other);}
        optional & operator=(optional       && other){ return assign(std::move(other));}

        ~optional() { reset(); }

    public: //getting value
        T const* get_ptr() const { return reinterpret_cast<T const*>(&storage);}
        T*       get_ptr()       { return reinterpret_cast<T*>(&storage);}

        const
        T& value() const         { return *get_ptr(); }
        T& value()               { return *get_ptr(); }

        T const & operator*  ()  const { return value(); }
        T       & operator * ()        { return value(); }

        T const * operator -> () const { return get_ptr(); }
        T       * operator -> ()       { return get_ptr(); }

    public: //querying state
        bool is_initialized() const { return initialized; }

        explicit operator bool() const { return is_initialized(); }

    public: //change state
        void reset()
        {
            if (is_initialized())
            {
                get_ptr()->~T();
                initialized = false;
            }
        }

        template<typename ...Args>
        void emplace(Args&&... args){
            if (is_initialized())
                reset();
            init(std::forward<Args>(args)...);
        }

    private: //assign
        optional & assign(MovableT && value)   {
            return set_from_value(std::move(value));
        }
        optional & assign(CopyableT const & value)  {
            return set_from_value(value);
        }
        optional & assign(optional const & other)  {
            return set_or_reset(other.value(), other.is_initialized());
        }
        optional & assign(optional && other)  {
            return set_or_reset(std::move(other.value()), other.is_initialized());
        }

    protected:
        template<typename U>
        optional & set_or_reset(U&& u, bool isValid)
        {
            if (isValid)
                set_from_value(std::forward<U>(u));
            else
                reset();

            return *this;
        }

        template<typename U>
        optional & set_from_value(U&& u)
        {
            if(is_initialized()){
                reinterpret_cast<T&>(storage) = std::forward<U>(u);
            }
            else{
                init(std::forward<U>(u));
            }

            return *this;
        }

        template<typename ...Args>
        void init(Args&& ... args){
            new (&storage) T(std::forward<Args>(args)...);
            initialized = true;
        }

    protected:
        StorageT storage;
        bool initialized;
    };

    template<typename T, typename ...Args>
    optional<T> & emplace(optional<T> & opt, Args&&... args){
        opt.emplace(std::forward<Args>(args)...);
        return opt;
    }
#endif

}}//namespace git
