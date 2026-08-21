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
        auto deleter = [m_alloc = alloc, m_len = len](Tvalue* p){
                                        deallocate_and_destroy_n(p, m_len, m_alloc);
                                    };
        try {
            return shared_ptr<T>(arr, deleter, alloc);
        } catch (...) {
            deallocate_and_destroy_n(arr, len, alloc);
            throw;
        }
    }
    
    template<typename T, typename... Args>
    shared_ptr<T> make_shared(Args&&... args) {
        return allocate_shared<T>(tca::get_default_allocator(), std::forward<Args>(args)...);
    }
}

#endif//JSTD_CPP_LANG_UTILS_SHARED_PTR_H