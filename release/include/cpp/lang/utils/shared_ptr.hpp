#ifndef JSTD_CPP_LANG_UTILS_SHARED_PTR_H
#define JSTD_CPP_LANG_UTILS_SHARED_PTR_H

#include <internal/smart_ptrs/shared_ptr_t.hpp>
#include <internal/smart_ptrs/shared_ptr_array_specialization.hpp>
#include <cpp/lang/traits/type_properties.hpp>
#include <cpp/lang/traits/pure_traits.hpp>

namespace tc
{
 
    template<typename T, typename... Args>
    typename enable_if< !is_array<T>::value, shared_ptr<T> >::type allocate_shared(tca::allocator* alloc, Args&&... args) {
        typedef typename remove_cv<T>::type Tvalue;

        typedef internal::obj_inline_control_block<Tvalue> control_block_type;
        
        void* block = alloc->allocate_align(sizeof(control_block_type), alignof(control_block_type));
        if (!block)
            throw_except<out_of_memory_error>("out of memory");

        try {
           
            control_block_type* control  = new (block) 
                                                        control_block_type(alloc, std::forward<Args>(args)...);
            Tvalue* obj = control->get_object();
           
            return shared_ptr<T>(obj, control);
        } catch (...) {
            alloc->deallocate(block);
            throw;
        }
    }

    template<typename T>
    typename enable_if< is_array<T>::value, shared_ptr<T> >::type allocate_shared(tca::allocator* alloc, std::size_t len, const typename remove_extent<T>::type& val = typename remove_extent<T>::type()) {
        
        typedef typename remove_cv<typename remove_extent<T>::type>::type Tvalue;
        Tvalue* arr = allocate_and_initialize_n<Tvalue>(len, alloc, val);
        
        //Первый раз использую auto... Stupid fuck..
        auto deleter = [alloc, len](Tvalue* p) {
                            deallocate_and_destroy_n(p, len, alloc);
                        };
        try {
            return shared_ptr<T>(arr, deleter, alloc);
        } catch (...) {
            deallocate_and_destroy_n(arr, len, alloc);
            throw;
        }
    }
    
    template<typename T, typename... Args>
    typename enable_if< !is_array<T>::value, shared_ptr<T> >::type make_shared(Args&&... args) {
        return allocate_shared<T>(tca::get_default_allocator(), std::forward<Args>(args)...);
    }
    
    template<typename T>
    typename enable_if< is_array<T>::value, shared_ptr<T> >::type make_shared(std::size_t len, const typename remove_extent<T>::type& val = typename remove_extent<T>::type()) {
        return allocate_shared<T>(tca::get_default_allocator(), len, val);
    }

    template<typename T, typename DELETER>
    shared_ptr<T> allocate_wrap_shared(tca::allocator* alloc, T* obj, DELETER deleter) {
        typedef internal::ptr_control_block<T, DELETER> control_block_type;
        void* mem = alloc->allocate_align(sizeof(control_block_type), alignof(control_block_type));
        if (!mem)
            throw_except<out_of_memory_error>("out of memory");
        try {
            control_block_type* control = new (mem) control_block_type(obj, std::move(deleter), alloc);
            return shared_ptr<T>(obj, control);
        } catch (...) {
            alloc->deallocate(mem);
            throw;
        }
    }
    
    template<typename T, typename DELETER>
    shared_ptr<T> wrap_shared(T* obj, DELETER deleter) {
        return allocate_wrap_shared(tca::get_default_allocator(), obj, std::move(deleter));
    }
}

#endif//JSTD_CPP_LANG_UTILS_SHARED_PTR_H