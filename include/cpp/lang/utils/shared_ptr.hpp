#ifndef JSTD_CPP_LANG_UTILS_SHARED_PTR_H
#define JSTD_CPP_LANG_UTILS_SHARED_PTR_H

#include <internal/smart_ptrs/shared_ptr_t.hpp>
#include <internal/smart_ptrs/shared_ptr_array_specialization.hpp>
#include <cpp/lang/traits/type_properties.hpp>
#include <cpp/lang/traits/pure_traits.hpp>

namespace tc
{
    template<typename T, typename T_ = T, typename = typename enable_if<is_same<typename pure_type<T>::type, typename pure_type<T_>::type >::value>::type>
    typename enable_if< !is_array<T>::value, shared_ptr<T> >::type make_shared(T_&& v = T(), tca::allocator* allocator = tca::get_default_allocator()) {
        return shared_ptr<T>(std::forward<T_>(v), allocator);
    }
    
    template<typename T, typename T_ = T>
    typename enable_if< !is_array<T>::value, shared_ptr<T> >::type make_shared(tca::allocator* allocator) {
        return shared_ptr<T>(T(), allocator);
    }

    template<typename T>
    typename enable_if< is_array<T>::value, shared_ptr<T> >::type make_shared(std::size_t len, const typename pure_type<T>::type& val = typename pure_type<T>::type(), tca::allocator* allocator = tca::get_default_allocator()) {
        return shared_ptr<T>(len, val, allocator);
    }
    
    template<typename T>
    typename enable_if< is_array<T>::value, shared_ptr<T> >::type make_shared(std::size_t len, tca::allocator* allocator = tca::get_default_allocator()) {
        return shared_ptr<T>(len, typename pure_type<T>::type(), allocator);
    }
}

#endif//JSTD_CPP_LANG_UTILS_SHARED_PTR_H