#ifndef OPTIONAL_H
#define OPTIONAL_H

#ifdef USE_BOOST
    #include <boost/optional.hpp>
    #include <boost/utility/typed_in_place_factory.hpp>
#else
    #include <memory>
#endif

namespace git
{
    namespace internal{

#ifdef USE_BOOST
        using namespace boost;
#else
        using none_t = nullptr_t;
        const auto none = nullptr;

        //Simple optional implementations
        template<typename T>
        class optional{
        public:
            using Pointer = std::shared_ptr<T>;

            enum invalid_type1{};
            enum invalid_type2{};

            typedef typename std::conditional<std::is_move_constructible<T>::value, T, invalid_type1>::type MovableT;
            typedef typename std::conditional<std::is_copy_constructible<T>::value, T, invalid_type2>::type CopyableT;

            template<typename U>
            static Pointer make(U&& u)           { return Pointer();} //default case
            static Pointer make(MovableT && u)   { return std::make_shared<T>(std::move(u));}
            static Pointer make(CopyableT && u)  { return std::make_shared<T>(std::move(u));}

        public:
            optional(none_t t = none){}

            optional(std::shared_ptr<T> ptr) : ptr(ptr)
            {}

            optional(T && v)
                : ptr(make(std::forward<T>(v)))
            {}
            optional(const T & v)
                : ptr(make(std::forward<T>(v)))
            {}


            T const* get_ptr() const {return ptr.get();}
            T*       get_ptr()		 {return ptr.get();};

            T& operator*() const     {return *ptr.get();}
            T* operator->() const    {return ptr.get();}

            bool is_initialized() const {return (bool)ptr;}

            operator bool()  const { return (bool)ptr;}
        private:
            std::shared_ptr<T> ptr;
        };


        template<typename T, typename ...Args>
        std::shared_ptr<T> in_place(Args&&... args){
            return std::make_shared<T>(std::forward<Args>(args)...);
        }

#endif

    }//namespace internal
}//namespace git

#endif // OPTIONAL_H
