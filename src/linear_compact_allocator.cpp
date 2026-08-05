// #include <allocators/linear_compact_allocator.hpp>
// #include <cpp/lang/exceptions.hpp>
// #include <exception>
// #include <iostream>

// namespace tca {

//     using namespace tc::internal::sptr;

//     /**
//      * 
//      * ############################################################################
//      *                           H E A D E R 
//      * ############################################################################
//      */
//     Header::Header() : _reference(nullptr), _movfunc(nullptr), _count(0) {}

//     Header::Header(Header&& h) : _reference(h._reference), _movfunc(h._movfunc), _count(h._count) {
//         if (_reference != nullptr) {
//             void* src = h.dataPointer();
//             void* dst = dataPointer();
//             _movfunc(dst, src, _count);
//             _reference->m_object = dst;
//         }
//         h._reference    = nullptr;
//         h._movfunc      = nullptr;
//         //h._size         = 0;
//         h._count        = 0;
//     }
    
//     Header::~Header() {
//         // if (_reference != nullptr) {
//         //     _reference->_data = nullptr;
//         //     _reference = nullptr;
//         // }
//     }

//     void* Header::dataPointer() {
//         char* _this = reinterpret_cast<char*>(this);
//         return _this + byteSize();
//     }

//     /**
//      * 
//      * ############################################################################
//      *                  L I N E A R _ C O M P A C T _ A L L O C A T O R
//      * ############################################################################
//      */

//     compact_linear_allocator::compact_linear_allocator(std::size_t capacity, base_allocator* allocator) : 
//     _allocator(allocator), 
//     m_ctrl_block_allocator(sizeof(tc::internal::sptr::shared_control_block), pool_allocator::DEFAULT_COUNT_BUCKETS, allocator), 
//     _capacity(capacity),
//     _offset(0) {
//         _data           = allocator->allocate_align(capacity, alignof(std::max_align_t));
//         _tmp            = nullptr;
//         _tmpCapacity    = 0;
//     }

//     compact_linear_allocator::compact_linear_allocator(compact_linear_allocator&& alloc) : 
//     _allocator(alloc._allocator),
//     m_ctrl_block_allocator(std::move(alloc.m_ctrl_block_allocator)),
//     _data(alloc._data),
//     _capacity(alloc._capacity),
//     _tmp(alloc._tmp),
//     _tmpCapacity(alloc._tmpCapacity),
//     _offset(alloc._offset)
//     {
//         alloc._allocator    = nullptr;
//         alloc._data         = nullptr;
//         alloc._capacity     = 0;
//         alloc._tmp          = nullptr;
//         alloc._tmpCapacity  = 0;
//         alloc._offset       = 0;
//     }
    
//     compact_linear_allocator& compact_linear_allocator::operator= (compact_linear_allocator&& alloc) {
//         if (&alloc != this) {
//             free();
//             _allocator              = alloc._allocator;
//             m_ctrl_block_allocator  = std::move(alloc.m_ctrl_block_allocator);
//             _data                   = alloc._data;
//             _capacity               = alloc._capacity;
//             _tmp                    = alloc._tmp;
//             _tmpCapacity            = alloc._tmpCapacity;
//             _offset                 = alloc._offset;

//             alloc._allocator    = nullptr;
//             alloc._data         = nullptr;
//             alloc._capacity     = 0;
//             alloc._tmp          = nullptr;
//             alloc._tmpCapacity  = 0;
//             alloc._offset       = 0;
//         }
//         return *this;
//     }

//     compact_linear_allocator::~compact_linear_allocator() {
//         free();
//     }

//     void compact_linear_allocator::free() {
//         if (_allocator != nullptr) {
//             if (_tmp != nullptr)
//                 _allocator->deallocate(_tmp,    _tmpCapacity);
//             if (_data != nullptr)
//                 _allocator->deallocate(_data,   _capacity);
//             _allocator  = nullptr;
//             _tmp        = nullptr;
//             _data       = nullptr;
//         }
//     }

//     void compact_linear_allocator::grow() {
//         grow(_capacity << 1);
//     }

//     void compact_linear_allocator::grow(std::size_t newCapacity) {
        
//         char* newData = reinterpret_cast<char*>(_allocator->allocate_align(newCapacity, alignof(std::max_align_t)));
//         if (newData == nullptr)
//             return;

//         for (std::size_t i = 0; i < _offset; ) {
//             Header* src     = reinterpret_cast<Header*>(reinterpret_cast<char*>(_data) + i);
//             Header* dst = new(newData + i) Header(std::move(*src));
//             i += dst->_reference->m_blocksize;
//         }

//         _allocator->deallocate(_data, _capacity);
//         _data       = newData;
//         _capacity   = newCapacity;
//     }
    
//     void compact_linear_allocator::moveHeader(Header* dst, Header* src) {
        
//         /**
//          * Если размер объекта больше, чем буфер для временных объектов,
//          * то происходит его увеличение.
//          */
//         {
//             std::size_t sz = src->_reference->m_blocksize;
//             if (_tmpCapacity < sz) {
//                 if (_tmp != nullptr)
//                     _allocator->deallocate(_tmp, _tmpCapacity);
//                 _tmp            = _allocator->allocate_align(sz, alignof(std::max_align_t));
//                 _tmpCapacity    = sz;
//             }
//         }
//         Header* tmp = new(_tmp) Header(std::move(*src)); 
//         new (dst) Header(std::move(*tmp));
//     }

//     void compact_linear_allocator::compact() {
//         std::size_t readPointer     = 0;
//         std::size_t writePointer    = 0;
//         char* const data = reinterpret_cast<char*>(_data);
//         while (readPointer < _offset) {
//             Header* srcHeader = reinterpret_cast<Header*>(data + readPointer);
            
//             assert(srcHeader != nullptr);
//             assert(srcHeader->_reference != nullptr);

//             std::size_t size = srcHeader->_reference->m_blocksize;
//             if (srcHeader->_reference->m_strong_refs == 0 && srcHeader->_reference->m_weak_refs == 0 ) {
//                 readPointer += size;
//                 srcHeader->_reference->~shared_control_block();    
//                 m_ctrl_block_allocator.deallocate(srcHeader->_reference);
//                 continue;
//             }

//             Header* dstHeader = reinterpret_cast<Header*>(data + writePointer);
            
//             if (dstHeader != srcHeader)
//                 moveHeader(dstHeader, srcHeader);

//             writePointer    += size;
//             readPointer     += size;
//         }

//         _offset = writePointer;
//     }

//     tc::internal::sptr::shared_control_block* compact_linear_allocator::allocate(std::size_t sz, std::size_t count, void (*move_func)(void*, void*, std::size_t)) {
//         std::size_t total = align_up(Header::byteSize() + sz * count, alignof(std::max_align_t));
        
//         if (_offset + total > _capacity) {
//             compact();
//             if (_offset + total > _capacity) {
                
//                 std::size_t tmpCapacity = _capacity;
//                 //Попытка увеличить ёмкость, пока ёмкость не будет больше запрашиваемого блока памяти.
//                 while (tmpCapacity < (_capacity + total))
//                     tmpCapacity <<= 1; //aka tmpCapacity *= 2
                
//                 grow(tmpCapacity);
 
//                 if (_offset + total > _capacity) //Выделить не удалось, возврат нулевого shared_ptr
//                     return nullptr;
//             }
//         }
    
//         tc::internal::sptr::shared_control_block* ctrl_block = (tc::internal::sptr::shared_control_block*) m_ctrl_block_allocator.allocate();
//         if (ctrl_block == nullptr)  //если котролирующий блок не удалось выделить, возвращает нулевой shared_ptr
//             return nullptr;
        
//         char* start_new_object = reinterpret_cast<char*>(_data);
//         //Header* header = new(start_new_object + _offset) Header();
//         Header* header = (Header*) (start_new_object + _offset);
//         header->_movfunc                        = move_func;
//         header->_count                          = count;
//         header->_reference                      = ctrl_block;
//         _offset += total;
        
//         new (ctrl_block) tc::internal::sptr::shared_control_block(nullptr, header->dataPointer(), total);
//         return ctrl_block;
//     }

//     void compact_linear_allocator::print() const {
//         std::printf("================== C O M P A C T   A L L O C A T O R ==================\n");
//         std::printf("[capacity: %zu, tmpbuf %zu]\n", _capacity, _tmpCapacity);
//         for (std::size_t i = 0; i < _offset; ) {
//             Header* h = reinterpret_cast<Header*>(reinterpret_cast<char*>(_data) + i);
//             //std::printf("[size: %llu, 0x%llx]\n", (unsigned long long) h->_reference->m_blocksize, (unsigned long long) h->_reference);
//             typedef unsigned long long ul;
//             std::printf("[size: %llu, 0x%llx, strong=%llu, weak=%llu]\n", (ul) h->_reference->m_blocksize, (ul) h->_reference, 
//                         (ul) h->_reference->m_strong_refs, (ul) h->_reference->m_weak_refs);
//             i += h->_reference->m_blocksize;
//         }
//     }
// }