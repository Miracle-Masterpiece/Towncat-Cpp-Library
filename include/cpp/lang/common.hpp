#ifndef B3DA9EB5_FB54_4C0C_93AC_4B68F3BBD56E
#define B3DA9EB5_FB54_4C0C_93AC_4B68F3BBD56E

#include <cstddef>

namespace tc
{

    /**
     * Returns a sentinel value representing "not found" or "no position".
     * 
     * This function returns the maximum value of std::size_t, used as a sentinel
     * to indicate that a position or index could not be found (e.g., in search
     * operations, substring lookup, or container find methods).
     *
     * @note
     *      This is the same value as eof_value() and is provided for semantic
     *      clarity in different contexts:
     *      npos(): "no position" / "item not found" / "end of file" / end of stream
     * 
     * @note
     *      This value is guaranteed to be larger than any valid container index.
     * 
     * @example
     *      std::size_t idx = list.index_of(value);
     *      if (idx == npos()) {
     *          // Value not found in list
     *      }
     */
    inline std::size_t npos() {
        return ~static_cast<std::size_t>(0);
    }

}

#endif /* B3DA9EB5_FB54_4C0C_93AC_4B68F3BBD56E */
