#ifndef EC12E84B_2C4A_448B_AA66_F927509E8ECF
#define EC12E84B_2C4A_448B_AA66_F927509E8ECF

#include <climits>
#include <cpp/lang/traits/sfinae.hpp>

namespace tc
{

/**
 * Meta-structure for checking whether a type is primitive.
 * 
 * This trait determines whether type T belongs to primitive types.
 * Primitive types include:
 * - Integer types (char, int, long, etc.).
 * - Floating-point types (float, double).
 * - const versions of these types.
 * 
 * @tparam T 
 *      The type being checked for being primitive.
 */
template<typename T> struct is_primitive            {static const bool value = false;};

template<> struct is_primitive<char>                {static const bool value = true;};
template<> struct is_primitive<const char>          {static const bool value = true;};
template<> struct is_primitive<volatile char>       {static const bool value = true;};
template<> struct is_primitive<const volatile char> {static const bool value = true;};

template<> struct is_primitive<wchar_t>                {static const bool value = true;};
template<> struct is_primitive<const wchar_t>          {static const bool value = true;};
template<> struct is_primitive<volatile wchar_t>       {static const bool value = true;};
template<> struct is_primitive<const volatile wchar_t> {static const bool value = true;};

template<> struct is_primitive<signed char>          {static const bool value = true;};
template<> struct is_primitive<const signed char>    {static const bool value = true;};
template<> struct is_primitive<volatile signed char> {static const bool value = true;};
template<> struct is_primitive<const volatile signed char> {static const bool value = true;};

template<> struct is_primitive<unsigned char>          {static const bool value = true;};
template<> struct is_primitive<const unsigned char>    {static const bool value = true;};
template<> struct is_primitive<volatile unsigned char> {static const bool value = true;};
template<> struct is_primitive<const volatile unsigned char> {static const bool value = true;};

template<> struct is_primitive<short>          {static const bool value = true;};
template<> struct is_primitive<const short>    {static const bool value = true;};
template<> struct is_primitive<volatile short> {static const bool value = true;};
template<> struct is_primitive<const volatile short> {static const bool value = true;};

template<> struct is_primitive<unsigned short>          {static const bool value = true;};
template<> struct is_primitive<const unsigned short>    {static const bool value = true;};
template<> struct is_primitive<volatile unsigned short> {static const bool value = true;};
template<> struct is_primitive<const volatile unsigned short> {static const bool value = true;};

template<> struct is_primitive<int>          {static const bool value = true;};
template<> struct is_primitive<const int>    {static const bool value = true;};
template<> struct is_primitive<volatile int> {static const bool value = true;};
template<> struct is_primitive<const volatile int> {static const bool value = true;};

template<> struct is_primitive<unsigned int>          {static const bool value = true;};
template<> struct is_primitive<const unsigned int>    {static const bool value = true;};
template<> struct is_primitive<volatile unsigned int> {static const bool value = true;};
template<> struct is_primitive<const volatile unsigned int> {static const bool value = true;};

template<> struct is_primitive<long>          {static const bool value = true;};
template<> struct is_primitive<const long>    {static const bool value = true;};
template<> struct is_primitive<volatile long> {static const bool value = true;};
template<> struct is_primitive<const volatile long> {static const bool value = true;};

template<> struct is_primitive<unsigned long>          {static const bool value = true;};
template<> struct is_primitive<const unsigned long>    {static const bool value = true;};
template<> struct is_primitive<volatile unsigned long> {static const bool value = true;};
template<> struct is_primitive<const volatile unsigned long> {static const bool value = true;};

template<> struct is_primitive<long long>          {static const bool value = true;};
template<> struct is_primitive<const long long>    {static const bool value = true;};
template<> struct is_primitive<volatile long long> {static const bool value = true;};
template<> struct is_primitive<const volatile long long> {static const bool value = true;};

template<> struct is_primitive<unsigned long long>          {static const bool value = true;};
template<> struct is_primitive<const unsigned long long>    {static const bool value = true;};
template<> struct is_primitive<volatile unsigned long long> {static const bool value = true;};
template<> struct is_primitive<const volatile unsigned long long> {static const bool value = true;};

template<> struct is_primitive<float>          {static const bool value = true;};
template<> struct is_primitive<const float>    {static const bool value = true;};
template<> struct is_primitive<volatile float> {static const bool value = true;};
template<> struct is_primitive<const volatile float> {static const bool value = true;};

template<> struct is_primitive<double>          {static const bool value = true;};
template<> struct is_primitive<const double>    {static const bool value = true;};
template<> struct is_primitive<volatile double> {static const bool value = true;};
template<> struct is_primitive<const volatile double> {static const bool value = true;};

template<> struct is_primitive<long double>          {static const bool value = true;};
template<> struct is_primitive<const long double>    {static const bool value = true;};
template<> struct is_primitive<volatile long double> {static const bool value = true;};
template<> struct is_primitive<const volatile long double> {static const bool value = true;};

}


namespace tc
{

// Is unsigned 
template <typename T> struct is_unsigned{static const bool value = false;};

#if WCHAR_MIN >= 0
 template <> struct is_unsigned<wchar_t>              {static const bool value = true;};
#endif

#if CHAR_MIN >= 0
 template <> struct is_unsigned<char>                {static const bool value = true;};
#endif

template <> struct is_unsigned<unsigned char>       {static const bool value = true;};
template <> struct is_unsigned<unsigned short>      {static const bool value = true;};
template <> struct is_unsigned<unsigned int>        {static const bool value = true;};
template <> struct is_unsigned<unsigned long>       {static const bool value = true;};
template <> struct is_unsigned<unsigned long long>  {static const bool value = true;};


// Is signed
template <typename T> struct is_signed{static const bool value = false;};

#if WCHAR_MIN < 0
 template <> struct is_unsigned<wchar_t>         {static const bool value = true;};
#endif

#if CHAR_MIN < 0
 template <> struct is_signed<char>              {static const bool value = true;};
#endif

template <> struct is_signed<signed char>       {static const bool value = true;};
template <> struct is_signed<signed short>      {static const bool value = true;};
template <> struct is_signed<signed int>        {static const bool value = true;};
template <> struct is_signed<signed long>       {static const bool value = true;};
template <> struct is_signed<signed long long>  {static const bool value = true;};
template <> struct is_signed<float>             {static const bool value = true;};
template <> struct is_signed<double>            {static const bool value = true;};
template <> struct is_signed<long double>       {static const bool value = true;};

// Make unsigned
template<typename T> struct make_unsigned   {typedef T type;};
template<> struct make_unsigned<signed char> {typedef unsigned char type;};
template<> struct make_unsigned<char>        {typedef unsigned char type;};
template<> struct make_unsigned<short>       {typedef unsigned short type;};
template<> struct make_unsigned<int>         {typedef unsigned int type;};
template<> struct make_unsigned<long>        {typedef unsigned long type;};
template<> struct make_unsigned<long long>   {typedef unsigned long long type;};

template<typename T> struct make_signed {typedef T type;};
template<> struct make_signed<char>                 {typedef signed char type;};
template<> struct make_signed<unsigned char>        {typedef signed char type;};
template<> struct make_signed<unsigned short>       {typedef signed short type;};
template<> struct make_signed<unsigned int>         {typedef signed int type;};
template<> struct make_signed<unsigned long>        {typedef signed long type;};
template<> struct make_signed<unsigned long long>   {typedef signed long long type;};

/**
 * Compile-time type selection trait for integer types of a specific bit size.
 * 
 * Provides the signed and unsigned integer types that have exactly `SIZE` bits
 * from the set of fundamental integer types (`char`, `short`, `int`, `long`, `long long`).
 * 
 * @tparam SIZE
 *      The required bit size (must match the size of a fundamental integer type).
 * 
 * @note The selection is performed in descending order of type size.
 *       Larger types are considered first to ensure the most suitable type is chosen
 *       when multiple types have the same size (e.g., `int` and `long` on some platforms).
 * 
 * @note This trait is intended for use in low-level memory manipulation,
 *       serialization, or fixed-size buffer implementations where exact
 *       integer sizes are required.
 * 
 * @invariant The selected type is guaranteed to have exactly `SIZE` bits.
 * 
 * @static_assert SIZE must be equal to the bit size of one of the fundamental
 *                integer types. Otherwise, compilation fails with a diagnostic
 *                message.
 * 
 * @example
 *      // Get types for a 32-bit integer
 *      using signed_32 = int_of_size<32>::stype;   // Typically 'int' or 'long'
 *      using unsigned_32 = int_of_size<32>::utype; // Typically 'unsigned int' or 'unsigned long'
 * 
 *      // Get types for an 8-bit integer
 *      using signed_8 = int_of_size<8>::stype;     // 'signed char'
 *      using unsigned_8 = int_of_size<8>::utype;   // 'unsigned char'
 * 
 *      static_assert(sizeof(signed_32) * CHAR_BIT == 32);
 *      static_assert(sizeof(unsigned_8) * CHAR_BIT == 8);
 * 
 * @example
 *      // Using with template metaprogramming
 *      template<std::size_t Bits>
 *      struct fixed_integer {
 *          using type = typename int_of_size<Bits>::stype;
 *      };
 * 
 *      fixed_integer<16>::type value = 0x7FFF; // 16-bit signed integer
 * 
 */
template<std::size_t SIZE> 
struct int_of {
private:
    static_assert(
        SIZE == sizeof(char)        * CHAR_BIT        ||
        SIZE == sizeof(short)       * CHAR_BIT       ||
        SIZE == sizeof(int)         * CHAR_BIT         ||
        SIZE == sizeof(long)        * CHAR_BIT        ||
        SIZE == sizeof(long long)   * CHAR_BIT,
        "SIZE must be equvalent fundamental types"
    );
    typedef typename select_if<SIZE == sizeof(signed long long) * CHAR_BIT,     signed long long,   long>::type         signed_0;
    typedef typename select_if<SIZE == sizeof(signed long)      * CHAR_BIT,     signed long,        signed_0>::type     signed_1;
    typedef typename select_if<SIZE == sizeof(signed int)       * CHAR_BIT,     signed int,         signed_1>::type     signed_2;
    typedef typename select_if<SIZE == sizeof(signed short)     * CHAR_BIT,     signed short,       signed_2>::type     signed_3;
    
    typedef typename select_if<SIZE == sizeof(unsigned long long) * CHAR_BIT,   unsigned long long, long>::type         unsigned_0;
    typedef typename select_if<SIZE == sizeof(unsigned long)      * CHAR_BIT,   unsigned long,      unsigned_0>::type   unsigned_1;
    typedef typename select_if<SIZE == sizeof(unsigned int)       * CHAR_BIT,   unsigned int,       unsigned_1>::type   unsigned_2;
    typedef typename select_if<SIZE == sizeof(unsigned short)     * CHAR_BIT,   unsigned short,     unsigned_2>::type   unsigned_3;
public:
    typedef typename select_if<SIZE == sizeof(signed char)      * CHAR_BIT,     signed char,    signed_3>::type     stype;
    typedef typename select_if<SIZE == sizeof(unsigned char)    * CHAR_BIT,     unsigned char,  unsigned_3>::type   utype;
};


}



#endif /* EC12E84B_2C4A_448B_AA66_F927509E8ECF */
