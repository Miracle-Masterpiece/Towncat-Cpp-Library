#ifndef EC12E84B_2C4A_448B_AA66_F927509E8ECF
#define EC12E84B_2C4A_448B_AA66_F927509E8ECF

#include <climits>

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

}



#endif /* EC12E84B_2C4A_448B_AA66_F927509E8ECF */
