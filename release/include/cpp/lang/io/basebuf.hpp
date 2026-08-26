#ifndef FF7FCA86_40F4_490B_80EA_175969C4B4E3
#define FF7FCA86_40F4_490B_80EA_175969C4B4E3

#include <cpp/lang/utils/cond_compile.hpp>
#include <cpp/lang/utils/utils.hpp>
#include <cpp/lang/exceptions.hpp>
#include <cpp/lang/common.hpp>
#include <cpp/lang/system.hpp>
#include <cpp/lang/string.hpp>
#include <cstddef>

namespace tc
{

/**
 * 
 */
class basebuf {
protected:
    char*           m_data;
    std::size_t     m_capacity;
    std::size_t     m_position;
    std::size_t     m_limit;
    std::size_t     m_mark;
    byte_order      m_order;
    bool            m_readonly;

    // Copy constructor deleted
    basebuf(const basebuf&) = delete;
    
    // Copy operator deleted
    basebuf& operator= (const basebuf&) = delete;

    JSTD_DEBUG_CODE(
        void can_write() {
            if (m_readonly)
                throw_except<readonly_exception>("buffer is readonly");
        }
    )

public:
    /**
     * 
     */
    basebuf() : m_data(nullptr), m_capacity(0), m_position(0), m_limit(0), m_mark(npos()), m_order(system::native_byte_order()), m_readonly(false) {}
    
    /**
     * 
     */
    basebuf(char* data, std::size_t cap) : m_data(data), m_capacity(cap), m_position(0), m_limit(cap), m_mark(npos()), m_order(system::native_byte_order()), m_readonly(false) {}

    /**
     * Returns the position of this buffer.
     *
     * @return
     *      The position of this buffer.
     */
    std::size_t position() const {
        return m_position;
    }
    
    /**
     * Sets the new position of this buffer.
     * If the mark is defined and the new position is greater than it, it is reset.
     *
     * @throws illegal_argument_exception (in DEBUG build)
     *      If the preconditions for 'newpos' are not met.
     */
    void position(std::size_t newpos) {
        JSTD_DEBUG_CODE(
            if (newpos >= m_capacity)
                tc::throw_except<illegal_argument_exception>("newpos %zu >= capacity %zu", newpos, capacity());
        );
        if (newpos > m_mark)
            m_mark = npos();
        m_position = newpos;
    }

    /**
     * Returns the limit of this buffer.
     *
     * @return
     *      The limit of this buffer.
     */
    std::size_t limit() const {
        return m_position;
    }
    
    /**
     * Returns the limit of this buffer.
     *
     * @return
     *      The limit of this buffer.
     *
     * @throws illegal_argument_exception (in DEBUG build)
     *      If the 'newlimit' condition is not met.
     */
    void limit(std::size_t newlimit) {
        JSTD_DEBUG_CODE(
            if (newlimit > m_capacity)
                throw_except<illegal_argument_exception>("newlimit %zu >= capacity %zu", newlimit, capacity());
        )
        if (m_mark != npos() && m_mark > m_limit)
        {
            m_mark = npos();
        }
        m_limit = newlimit;
    }

    /**
     * Returns the capacity of this buffer.
     *
     * @return
     *      The capacity of this buffer.
     */
    std::size_t capacity() const {
        return m_capacity;
    }

    /**
     *
     * Returns a pointer to the internal buffer.
     *
     * @return
     *      A pointer to the data.
     */
    char* data() {
        return m_data;
    }
    
    /**
     *
     * Returns a constant pointer to the internal buffer.
     *
     * @return
     *      A constant pointer to the data.
     */
    const char* data() const {
        return m_data;
    }

    /**
     * Returns the byte order of this buffer.
     *
     * The byte order used when reading or writing multi-byte values.
     * The default order is platform-dependent.
     *
     * @return
     *      The byte order of this buffer.
     */
    byte_order order() const {
        return m_order;
    }

    /**
     * Changes the byte order of this buffer.
     *
     * @param order
     *      New byte order:
     *      byte_order::LE (Little-Endian) or byte_order::BE (Big-Endian)
     */
    void order(byte_order order) {
        m_order = order;
    }

    /**
     * Resets the position of this buffer to the previously marked point.
     * Using this method does not cancel or remove the mark.
     *
     * @throw invalid_mark_exception (in DEBUG build)
     *      If the mark is not set
     */ 
    void reset() {
        JSTD_DEBUG_CODE(
            if (m_mark == npos())
                throw_except<invalid_mark_exception>("mark not setted");
        )
        m_position = m_mark;
    }

    /**
     * Clears this buffer.
     * The position is set to zero,
     * the limit is set to the capacity, and the label is reset.
     *
     * This function does not erase data.
     */
    void clear() {
        m_position  = 0;
        m_limit     = m_capacity;
        m_mark      = npos();
    }

    /**
     * Sets the buffer's mark to its position.
     */
    void mark() {
        m_mark = m_position;
    }

    /**
     * Returns the number of elements between the current position and the limit.
     *
     * @return
     *      The number of remaining elements in this buffer.
     */
    std::size_t remaining() const {
        if (m_limit < m_position)
            return 0;
        return m_limit - m_position;
    }

    /**
     * Reports whether there are elements between the current position and the limit.
     *
     * @return
     *      True if and only if
     *      there is at least one element remaining in the buffer.
     */
    bool has_remaining() const {
        return remaining() > 0;
    }

    /**
     * Tells whether this buffer is read-only.
     *
     * @return
     *      True only if
     *      this buffer is read-only.
     */
    bool is_readonly() const {
        return m_readonly;
    }

    /**
     * Rewinds this buffer.
     * The position is set to zero and the label is reset.
     */
    void rewind() {
        m_position  = 0;
        m_mark      = npos();
    }

    /**
     * Compacts this buffer.
     *
     * Bytes between the current buffer position and its limit, if any, are copied to the beginning of the buffer.
     * That is, the byte at index p = position() is copied to index zero, the byte at index p + 1 is copied to index one, and so on,
     * until the byte at index limit() - 1 is copied to index n = limit() - 1 - p.
     * Then the buffer position is set to n + 1, and its limit is set to its capacity.
     * The mark, if defined, is discarded.
     *
     * The buffer position is set to the number of bytes copied, not zero,
     * so that another relative write method can be called immediately after calling this method.
     *
     * @throws readonly_exception (in DEBUG build)
     *      If this buffer is read-only.
     */
    void compact() {
        JSTD_DEBUG_CODE(can_write();)
        if (remaining() > 0)
        {
            std::size_t rem = remaining();
            std::memmove(m_data, m_data + m_position, rem);
            m_position  = rem;
            m_mark      = npos();
            m_limit     = m_capacity;
        }
    }

    /**
     * Reverses this buffer.
     * The limit is set to the current position,
     * and then the position is set to zero.
     */
    void flip() {
        m_limit     = m_position;
        m_position  = 0;
    }

    /**
     * Absolute insert function.
     * Writes sizeof(T) bytes, in the current byte order, to this buffer at the given index.
     *
     * @param idx
     *      The index at which the value will be written.
     *
     * @param v
     *      The value of the type to write.
     *
     * @throws index_out_of_bound_exception (in DEBUG build)
     *      If the index is not less than or equal to the limit - sizeof(T)
     *
     * @throws readonly_exception (in DEBUG build)
     *      If this buffer is read-only.
     */
    template<typename T>
    void put(std::size_t idx, T v) {
        JSTD_DEBUG_CODE(
            can_write();
            if (m_limit < sizeof(T) || (m_limit - sizeof(T) < idx))
                throw_except<index_out_of_bound_exception>("idx %zu out of bound limit %zu", idx, m_limit);
        )
        if (sizeof(T) > sizeof(char) && m_order != system::native_byte_order())
        {
            utils::swap_copy<T>(m_data + idx, &v);
        }
        else
        {
            std::memcpy(m_data + idx, &v, sizeof(T));
        }
    }
    
    /**
     * Relative insertion function.
     *
     * Writes sizeof(T) bytes, in the current byte order,
     * into this buffer at the current position, and then increments the position by sizeof(T)
     *
     * @throws overflow_exception (in DEBUG build)
     *      If the current position is not less than or equal to the limit - sizeof(T).
     *
     * @throws readonly_exception (in DEBUG build)
     *      If this buffer is read-only.
     */
    template<typename T>
    void put(T v) {
        JSTD_DEBUG_CODE(
            if (m_limit < sizeof(T) || m_limit - sizeof(T) < m_position)
                throw_except<overflow_exception>("buffer owerflow");
        )
        put(m_position, v);
        m_position += sizeof(T);
    }

    /**
     * Absolute access to a value of type T.
     * Reads sizeof(T) bytes at the given index,
     * composing the value of type T in the current byte order.
     *
     * @param idx
     *      The index from which bytes will be read.
     *
     * @throws index_out_of_bound_exception (in DEBUG build)
     *      If the index is not less than or equal to the limit, the buffer sizeof(T)
     */
    template<typename T>
    T get(std::size_t idx) const {
        JSTD_DEBUG_CODE(
            if (m_limit < sizeof(T) || (m_limit - sizeof(T) < idx))
                throw_except<index_out_of_bound_exception>("idx %zu out of bound limit %zu", idx, m_limit);
        )
        
        T x;

        if (sizeof(T) > sizeof(char) && m_order != system::native_byte_order())
        {
            utils::swap_copy<T>(&x, m_data + idx);
        }
        else
        {
            std::memcpy(&x, m_data + idx, sizeof(T));
        }

        return x;
    }

    /**
     * Relative read function of type T.
     * Reads sizeof(T) bytes at the current position of this buffer,
     * concatenating them into value T, according to the current byte order,
     * and then increments the position by sizeof(T)
     *
     * @return
     *      The value of type T at the current position of the buffer.
     *
     * @throws underflow_exception (in DEBUG build)
     *      If the remaining number of bytes in this buffer is less than sizeof(T)
     */
    template<typename T>
    T get() {
        JSTD_DEBUG_CODE(
            if (remaining() < sizeof(T))
                throw_except<underflow_exception>("buffer underflow");
        )
        T x = get<T>(m_position);
        m_position += sizeof(T);
        return x;
    }

    /**
     * Relative insert function.
     * Writes 'len' sizeof(T) bytes, in the current byte order, to this buffer.
     *
     * @param arr
     *      An array of values ​​of type T.
     * 
     * @param len
     *      The number of elements to write.
     *
     * @throws overflow_exception (in DEBUG build)
     *      If sizeof(T) * len < remaining()
     *
     * @throws readonly_exception (in DEBUG build)
     *      If this buffer is read-only.
     */
    template<typename T>
    void puts(const T* arr, std::size_t len) {
        JSTD_CALLTRACE_CODE(
            can_write();
            std::size_t total_need = sizeof(T) * len;
            if (remaining() < total_need)
                throw_except<overflow_exception>("buffer owerflow");
        )
        for (std::size_t i = 0; i < len; ++i)
        {
            utils::swap_copy<T>(m_data + m_position, &arr[i]);
            m_position += sizeof(T);
        }
    }

    /**
     * Relative read method.
     * This function reads 'len' of type T.
     *
     * @param arr
     *      The destination array.
     *
     * @param len
     *      The number of values ​​to read.
     *
     * @throws underflow_exception (in DEBUG build)
     *      If sizeof(T) * len < remaining()
     */
    template<typename T>
    void gets(T* arr, std::size_t len) {
        JSTD_CALLTRACE_CODE(
            can_write();
            std::size_t total_need = sizeof(T) * len;
            if (remaining() < total_need)
                throw_except<underflow_exception>("buffer underflow");
        )
        for (std::size_t i = 0; i < len; ++i)
        {
            utils::swap_copy<T>(&arr[i], m_data + m_position);
            m_position += sizeof(T);
        }
    }

    /**
     * Returns a string representation of this buffer.
     *
     * @return
     *      A string representing information about the buffer.
     */
    string to_string(tca::allocator* alloc = tca::get_default_allocator()) const;
};

} //namespace tc

#endif /* FF7FCA86_40F4_490B_80EA_175969C4B4E3 */
