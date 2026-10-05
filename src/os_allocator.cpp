#include <tc/internal/allocators/os_allocator.hpp>
#include <utility>

#ifdef __linux__
    #define LINUX_OS __linux__
#elif __APPLE__
    #define MAC_OS __APPLE__
#elif _WIN32
    #define WINDOWS_OS _WIN32
#else
#error Platform is not defined
#endif

#if defined(LINUX_OS) || defined(MAC_OS)
    #include <sys/mman.h>
    #include <cstring>
    #include <errno.h>
#elif WINDOWS_OS
    #include <memoryapi.h>    
    #include <windows.h>
#endif

#ifndef NDEBUG
    #include <cstdint>
    #include <cassert>
#endif

namespace tca {

    os_allocator::os_allocator(int protect) : base_allocator(), protect(protect), lastError(NO_ERRORS) {

    }

    os_allocator::os_allocator(os_allocator&& alloc) : base_allocator(std::move(alloc)), protect(alloc.protect), lastError(alloc.lastError) {

    }
    
    os_allocator& os_allocator::operator= (os_allocator&& alloc) {
        if (&alloc != this) {
            base_allocator::operator=(std::move(alloc));
            protect     = alloc.protect;
            lastError   = alloc.lastError;
        }
        return *this;
    }
    
    os_allocator::~os_allocator() {

    }
    
    void* os_allocator::allocate(std::size_t sz) {
        lastError = NO_ERRORS;
        return allocate_align(sz, alignof(char));
    }
    
    int os_allocator::getLastError() const {
        return lastError;
    }

#if defined(LINUX_OS) || defined(MAC_OS)
    
    void* os_allocator::allocate_align(std::size_t sz, std::size_t align) {
        lastError = NO_ERRORS;
        int prot = 0;
        prot |= protect & READ   ? PROT_READ  : 0;
        prot |= protect & WRITE  ? PROT_WRITE : 0;
        prot |= protect & EXEC   ? PROT_EXEC  : 0;
        
        void* block = mmap(nullptr, sz, prot, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
        
        if (block == MAP_FAILED) {
            lastError = errno;
            return nullptr;
        }
        
        return block;
    }
    
    void os_allocator::deallocate(void* ptr, std::size_t sz) {
        lastError = NO_ERRORS;
        if (ptr == nullptr)
            return;
        if (munmap(ptr, sz) != 0)
            lastError = errno;
    }

    const char* os_allocator::getErrorString(int error) {
        return std::strerror(error);        
    }

#elif WINDOWS_OS

    /**
     * @internal
     * Переопределяет маску защиты памяти платформо-независимой библиотеки в Windows маску.
     * 
     * @param prot
     *          Маска защиты библиотеки.
     * 
     * @return
     *          Маска защиты для операционной системы Windows.
     */
    DWORD libProtectToWindowsProtect(int prot) {
        DWORD native_protect = PAGE_NOACCESS;
        
        const int MEM_PROTECT_READ  = os_allocator::READ;
        const int MEM_PROTECT_WRITE = os_allocator::WRITE;
        const int MEM_PROTECT_EXEC  = os_allocator::EXEC;

        if ((prot & MEM_PROTECT_EXEC) && (prot & MEM_PROTECT_READ) && (prot & MEM_PROTECT_WRITE))
            native_protect = PAGE_EXECUTE_READWRITE;
        else if ((prot & MEM_PROTECT_EXEC) && (prot & MEM_PROTECT_READ))
            native_protect = PAGE_EXECUTE_READ;
        else if (((prot & MEM_PROTECT_READ) && (prot & MEM_PROTECT_WRITE)) || (prot & MEM_PROTECT_WRITE))
            native_protect = PAGE_READWRITE;
        else if (prot & MEM_PROTECT_EXEC)
            native_protect = PAGE_EXECUTE;
        else if (prot & MEM_PROTECT_READ)
            native_protect = PAGE_READONLY;
        else 
            return PAGE_NOACCESS;
        
        return native_protect;
    }

    void* os_allocator::allocate_align(std::size_t sz, std::size_t) {
        lastError = NO_ERRORS;
        DWORD prot = libProtectToWindowsProtect(protect);
        void* block = VirtualAlloc(nullptr, sz, MEM_COMMIT | MEM_RESERVE, prot);
        if (block == nullptr)
            lastError = GetLastError();
        return block;
    }

    void os_allocator::deallocate(void* ptr, std::size_t) {
        lastError = NO_ERRORS;
        if (ptr == nullptr)
            return;
        BOOL result = VirtualFree(ptr, 0, MEM_RELEASE);
        if (!result) 
            lastError = GetLastError();
    }

    const char* os_allocator::getErrorString(int error) {
        switch(error) {
            case ERROR_SUCCESS:
                return "The operation completed successfully.";
            case ERROR_INVALID_PARAMETER:
                return "The parameter is incorrect.";
            case ERROR_INVALID_HANDLE:
                return "The handle is invalid.";
            case ERROR_NOT_ENOUGH_MEMORY: 
                return "Not enough memory resources are available to process this command.";
            case ERROR_OUTOFMEMORY:
                return "Not enough storage is available to complete this operation.";            
            case ERROR_INVALID_ADDRESS:
                return "Attempt to access invalid address.";
            default:
                return "Undefined error";
        }
    }

#endif
}