#pragma once

#ifdef USE_BOOST
    #include <boost/optional.hpp>
    #include <boost/utility/typed_in_place_factory.hpp>
#else
    #include <memory>
    #include <cstddef>
#endif

namespace git
{
    namespace internal{

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
        class optional{
        public: //type alias
            typedef typename std::aligned_storage<sizeof(T), alignof(T)>::type
                    StorageT;
            using optionalT = optional<T>;

        public: //type traits
            enum non_movable_t{};
            enum non_copyable_t{};

            static const constexpr bool movable = std::is_move_constructible<T>::value && std::is_move_assignable<T>::value;
            static const constexpr bool copyable = std::is_copy_constructible<T>::value && std::is_copy_assignable<T>::value;
            typedef typename std::conditional<movable, T, non_movable_t>::type MovableT;
            typedef typename std::conditional<copyable, T, non_copyable_t>::type CopyableT;

        public: //initializing and attribution
            constexpr
            optional(none_t t = none){}
            optional(const T & v)   { assign(v);}
            optional(T && v)        { assign(std::forward<T>(v));}

            optional(const optionalT &  other){ assign(other);}
            optional(      optionalT && other){ assign(std::move(other));}

            optionalT & operator=(none_t none)  { reset(); return *this;}
            optionalT & operator=(T && v)       { return assign(std::forward<T>(v));}
            optionalT & operator=(const T & v)  { return assign(v);}

            optionalT & operator=(const optionalT &  other){ return assign(other);}
            optionalT & operator=(      optionalT && other){ return assign(std::move(other));}

            ~optional() { reset(); }

        public: //getting value
            T const* get_ptr() const {return reinterpret_cast<T const*>(&storage);}
            T*       get_ptr()		 {return reinterpret_cast<T*>(&storage);}

            const
            T& value() const         {return *get_ptr();}
            T& value()               {return *get_ptr();}

            const
            T& operator*() const     {return value();}
            T& operator*()           {return value();}

            const
            T* operator->() const    {return get_ptr();}
            T* operator->()          {return get_ptr();}

        public: //querying state
            bool is_initialized() const {return initialized;}

            operator bool()  const { return is_initialized();}

        public: //change state
            void reset(){
                if(is_initialized()){
                    get_ptr()->~T();
                    initialized = false;
                }
            }

            template<typename ...Args>
            void emplace(Args&&... args){
                if(is_initialized()){
                    reset();
                }
                init(std::forward<Args>(args)...);
            }

        protected: //assign

            template<typename U>
            optionalT & assign(U&& u)           {
                static constexpr const bool same_class = std::is_same<std::remove_const<U>, T&>::value;
                static_assert(!same_class            , "Invalid type");
                static_assert(same_class && !copyable, "Type is non copyable");
                static_assert(same_class && !movable , "Type is non movable");

                return *this;
            } //default case

            optionalT & assign(MovableT && value)   {
                return set_from_value(std::move(value));
            }
            optionalT & assign(const CopyableT & value)  {
                return set_from_value(value);
            }
            optionalT & assign(const optionalT & other)  {
                return set_or_reset(other.value(), other.is_initialized());
            }
            optionalT & assign(optionalT && other)  {
                return set_or_reset(std::move(other.value()), other.is_initialized());
            }

        protected:
            template<typename U>
            optionalT & set_or_reset(U&& u, bool isValid)           {
                if(isValid){
                    set_from_value(std::forward<U>(u));
                }
                else{
                    reset();
                }

                return *this;
            }

            template<typename U>
            optionalT & set_from_value(U&& u)           {
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
            bool initialized=false;
        };

        template<typename T, typename ...Args>
        optional<T> & emplace(optional<T> & opt, Args&&... args){
            opt.emplace(std::forward<Args>(args)...);
            return opt;
        }

#endif

    }//namespace internal
}//namespace git
