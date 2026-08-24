#ifndef B7D7A5CF_9BC8_4925_BDFD_B4A8369B7151
#define B7D7A5CF_9BC8_4925_BDFD_B4A8369B7151

#include <cpp/lang/string.hpp>

namespace tc
{

class error_category;

/**
 * Returns the system error category.
 * 
 * The system category represents platform-specific error codes.
 * On POSIX systems, these are errno values.
 * On Windows, these are GetLastError() values.
 * 
 * This category provides human-readable error messages using
 * platform-specific APIs:
 * - POSIX: strerror() or strerror_r()
 * - Windows: FormatMessage()
 * 
 * @return
 *      Reference to the global system error category instance.
 * 
 * @note
 *      The returned category is thread-safe and guaranteed to outlive
 *      any code that uses it.
 * 
 * @example
 *      // Creating a system error:
 *      #ifdef _WIN32
 *      HANDLE h = CreateFile(...);
 *      if (h == INVALID_HANDLE_VALUE) {
 *          return error_code(GetLastError(), system_category());
 *      }
 *      #else
 *      int fd = open("/path", O_RDONLY);
 *      if (fd == -1) {
 *          return error_code(errno, system_category());
 *      }
 *      #endif
 * 
 * @see
 *      generic_category()
 *      error_category
 *      error_code
 */
const error_category& system_category();

/**
 * Returns the errno (POSIX) error category.
 * 
 * The errno category represents POSIX-compatible error codes defined
 * in <cerrno>. This category is a subset of the system category that
 * specifically deals with errno values.
 * 
 * On POSIX systems, this category is equivalent to system_category.
 * On Windows, this category provides POSIX-compatible error codes
 * mapped from Windows error codes.
 * 
 * @return
 *      Reference to the global errno error category instance.
 * 
 * @note
 *      The returned category is thread-safe and guaranteed to outlive
 *      any code that uses it.
 * 
 * @example
 *      // Using errno category directly:
 *      if (some_posix_call() == -1) {
 *          return error_code(errno, generic_category());
 *      }
 * 
 * @see
 *      system_category()
 *      error_category
 *      error_code
 */
const error_category& generic_category();

/**
 * Error code wrapper with associated error category.
 * 
 * The error_code class encapsulates an error code value along with its
 * associated error category. It provides functionality to retrieve
 * the error value, the category name, and a human-readable error message.
 * 
 * This class is similar to std::error_code from the C++ standard library.
 * 
 * @note
 *      By default, error_code is constructed with an error value of 0
 *      and a reference to the default category.
 * 
 * @warning
 *      error_code stores a reference to the error_category. The referenced
 *      category must outlive the error_code object.
 * 
 * @example
 *      // Creating an error code from a system error:
 *      int fd = open("/path", O_RDONLY);
 *      if (fd == -1) {
 *          error_code ec(errno, system_category());
 *          const char* name = ec.get_category().get_name();  // "system"
 *          string msg = ec.get_message();                    // "No such file or directory"
 *      }
 * 
 * @see
 *      error_category
 *      system_category()
 *      generic_category()
 */
class error_code {
    
    // Error value
    int val;
    
    // Error category for mapping err value
    const error_category* category;

public:
    /**
     * Default constructor.
     * 
     * Constructs an error code with a value of 0 and a reference to
     * the default error category.
     * 
     * @note
     *      The default category is implementation-defined but typically
     *      points to the system category.
     */
    error_code();

    /**
     * Constructs an error code with the given value and category.
     * 
     * @param err
     *      The error code value.
     * 
     * @param ecat
     *      Reference to the error category associated with this error code.
     * 
     * @warning
     *      The category reference must remain valid for the lifetime
     *      of the error_code object.
     */
    error_code(int err, const error_category& ecat);
    
    /**
     * Copy constructor.
     * 
     * @param other
     *      The error code to copy from.
     */
    error_code(const error_code&) = default;
    
    /**
     * Move constructor.
     * 
     * @param other
     *      The error code to move from.
     */
    error_code(error_code&&) = default;
    
    /**
     * Copy assignment operator.
     * 
     * @param other
     *      The error code to copy from.
     * 
     * @return
     *      Reference to this error code.
     */
    error_code& operator= (const error_code&) = default;
    
    /**
     * Move assignment operator.
     * 
     * @param other
     *      The error code to move from.
     * 
     * @return
     *      Reference to this error code.
     */
    error_code& operator= (error_code&&) = default;

    /**
     * Retrieves the error code value.
     * 
     * @return
     *      The numeric error code.
     */
    int get_value() const {
        return val;
    }

    /**
     * 
     */
    operator bool() const {
        return val != 0;
    }

    /**
     * Retrieves the human-readable error message.
     * 
     * @param alloc
     *      Pointer to an allocator to use for memory allocation.
     *      If nullptr or omitted, the default allocator is used
     *      (tca::get_default_allocator()).
     * 
     * @return
     *      A string containing the error message.
     * 
     * @note
     *      The message is obtained from the associated error category
     *      using the stored error value.
     */
    string get_message(tca::allocator* alloc = tca::get_default_allocator()) const;
    
    /**
     * Retrieves a reference to the associated error category.
     * 
     * @return
     *      Const reference to the error category.
     */
    const error_category& get_category() const {
        return *category;
    }
};

/**
 * Error condition wrapper with associated error category.
 * 
 * The error_condition class encapsulates an error condition value along with its
 * associated error category. Unlike error_code which represents a specific
 * system error value, error_condition represents a portable error condition
 * that can be compared across different error categories.
 * 
 * This class is similar to std::error_condition from the C++ standard library.
 * 
 * @note
 *      By default, error_condition is constructed with an error value of 0
 *      and a reference to the default category.
 * 
 * @warning
 *      error_condition stores a reference to the error_category. The referenced
 *      category must outlive the error_condition object.
 * 
 * @example
 *      // Creating an error condition from a generic error:
 *      error_condition ec(ENOENT, generic_category());
 *      const char* name = ec.get_category().get_name();  // "errno"
 *      string msg = ec.get_message();                    // "No such file or directory"
 * 
 * @see
 *      error_category
 *      generic_category()
 *      error_code
 */
class error_condition {
    
    // 
    // The error condition value.
    // 
    int val;
    
    // 
    // Reference to the associated error category.
    // 
    const error_category* category;

public:
    /**
     * Default constructor.
     * 
     * Constructs an error condition with a value of 0 and a reference to
     * the default error category.
     * 
     * @note
     *      The default category is implementation-defined but typically
     *      points to the generic category.
     */
    error_condition();

    /**
     * Constructs an error condition with the given value and category.
     * 
     * @param err
     *      The error condition value.
     * 
     * @param ecat
     *      Reference to the error category associated with this error condition.
     * 
     * @warning
     *      The category reference must remain valid for the lifetime
     *      of the error_condition object.
     */
    error_condition(int err, const error_category& ecat);
    
    /**
     * Copy constructor.
     * 
     * @param other
     *      The error condition to copy from.
     */
    error_condition(const error_condition&) = default;
    
    /**
     * Move constructor.
     * 
     * @param other
     *      The error condition to move from.
     */
    error_condition(error_condition&&) = default;
    
    /**
     * Copy assignment operator.
     * 
     * @param other
     *      The error condition to copy from.
     * 
     * @return
     *      Reference to this error condition.
     */
    error_condition& operator= (const error_condition&) = default;
    
    /**
     * Move assignment operator.
     * 
     * @param other
     *      The error condition to move from.
     * 
     * @return
     *      Reference to this error condition.
     */
    error_condition& operator= (error_condition&&) = default;

    /**
     * Retrieves the error condition value.
     * 
     * @return
     *      The numeric error condition value.
     */
    int get_value() const {
        return val;
    }

    /**
     * Retrieves the human-readable error message.
     * 
     * @param alloc
     *      Pointer to an allocator to use for memory allocation.
     *      If nullptr or omitted, the default allocator is used
     *      (tca::get_default_allocator()).
     * 
     * @return
     *      A string containing the error message.
     * 
     * @note
     *      The message is obtained from the associated error category
     *      using the stored error value.
     */
    string get_message(tca::allocator* alloc = tca::get_default_allocator()) const;
    
    /**
     * Retrieves a reference to the associated error category.
     * 
     * @return
     *      Const reference to the error category.
     */
    const error_category& get_category() const {
        return *category;
    }
};


/**
 * Abstract base class for error category providers.
 * 
 * The error_category class defines the interface for all error categories
 * in the library. Each category represents a domain of error codes and
 * provides human-readable names and messages for errors within that domain.
 * 
 * This design follows the same pattern as std::error_category from the
 * C++ standard library, providing a flexible and extensible error handling
 * system.
 * 
 * @note
 *      Derived classes must implement both pure virtual functions:
 *      - get_name()
 *      - get_message()
 * 
 * @example
 *      // Custom error category implementation:
 *      class my_error_category : public error_category {
 *      public:
 *          const char* get_name() const override {
 *              return "my_category";
 *          }
 *          
 *          string get_message(int err, tca::allocator* alloc) const override {
 *              switch (err) {
 *                  case 1:  return tc::string("Error code 1", alloc);
 *                  case 2:  return tc::string("Error code 2", alloc);
 *                  default: return tc::string("Unknown error", alloc);
 *              }
 *          }
 *      };
 * 
 * @see
 *      error_code
 *      system_category()
 *      generic_category()
 */
class error_category {
public:
    /**
     * Virtual destructor.
     * 
     * Ensures proper cleanup of derived class objects when deleted
     * through a pointer to the base class.
     */
    virtual ~error_category();

    /**
     * Retrieves the name of the error category.
     * 
     * This function returns a unique, human-readable name that identifies
     * the error category. The name should be stable and consistent across
     * all instances of the same category.
     * 
     * @return
     *      A C-string containing the category name (e.g., "system", "errno").
     * 
     * @note
     *      The returned string must remain valid for the lifetime of the
     *      category object. Typically, this is implemented by returning
     *      a string literal or a static string.
     * 
     * @warning
     *      This is a pure virtual function and must be implemented by
     *      every derived class.
     */
    virtual const char* get_name() const = 0;

    /**
     * Retrieves the human-readable message for an error code.
     * 
     * This function converts a numeric error code within this category
     * into a human-readable string description.
     * 
     * @param err
     *      The error code value to look up within this category.
     * 
     * @param alloc
     *      Pointer to an allocator to use for memory allocation.
     *      If nullptr or omitted, the default allocator is used
     *      (tca::get_default_allocator()).
     * 
     * @return
     *      A string containing the human-readable error message.
     * 
     * @note
     *      This is a pure virtual function and must be implemented by
     *      every derived class.
     * 
     * @note
     *      The implementation should handle unknown error codes gracefully,
     *      typically returning a message like "Unknown error" or
     *      "Unknown error code X".
     * 
     * @warning
     *      The allocator parameter is optional and defaults to the
     *      global default allocator. Use this parameter to control
     *      memory allocation behavior in custom allocator environments.
     */
    virtual string get_message(int err, tca::allocator* alloc = tca::get_default_allocator()) const = 0;

    /**
     * Compares this error category with another for equality.
     * 
     * Two error categories are equal if they are the same object.
     * Comparison is performed by pointer address.
     * 
     * @param ecat
     *      The error category to compare against.
     * 
     * @return 
     *      true if both categories are the same object, false otherwise.
     * 
     * @note
     *      Error categories are typically implemented as singletons,
     *      so address comparison is sufficient.
     * 
     * @example
     *      const auto& sys = system_category();
     *      const auto& sys2 = system_category();
     *      const auto& gen = generic_category();
     *      
     *      sys == sys2;  // true (same singleton)
     *      sys == gen;   // false (different categories)
     */
    bool operator== (const error_category& ecat) const {
        return &ecat == this;
    }

    /**
     * Converts an error code value to an error condition.
     * 
     * Converts a numeric error code within this category to a portable
     * error condition. This allows platform-specific error codes to be
     * compared against generic error conditions.
     * 
     * @param err The error code value to convert.
     * @return    An error condition representing the portable equivalent.
     * 
     * @note
     *      This function should be overridden by derived categories to provide
     *      meaningful conversions.
     *      Default implementation:
     *          error_condition error_category::default_error_condition(int err) const {
     *              return error_condition(err, *this);
     *          }
     * 
     * @example
     *      // Windows: ERROR_FILE_NOT_FOUND -> generic::ENOENT
     *      error_condition ec = system_category().default_error_condition(ERROR_FILE_NOT_FOUND);
     */
    virtual error_condition default_error_condition(int err) const;
    
    /**
     * Checks if an error code is equivalent to an error condition.
     * 
     * Determines whether the given error code value (from this category)
     * represents the same error as the provided error condition.
     * 
     * @param err
     *      The error code value to check.
     * 
     * @param cond
     *      The error condition to compare against.
     * 
     * @return
     *      true if equivalent, false otherwise.
     * 
     * @note
     *      This function should be overridden by derived categories to provide
     *      custom equivalence logic.
     * 
     * @example
     *      bool eq = system_category().equivalent(ERROR_FILE_NOT_FOUND, 
     *                                             error_condition(ENOENT, generic_category()));
     *      // true on Windows
     */
    virtual bool equivalent(int err, const error_condition& cond) const;
    
    /**
     * Checks if an error code is equivalent to an error condition.
     * 
     * Determines whether the given error code (from any category) is equivalent
     * to a numeric error condition value from this category.
     * 
     * @param err   The error code to check.
     * @param cond  The error condition value (from this category).
     * @return      true if equivalent, false otherwise.
     * 
     * @note
     *      This function should be overridden by derived categories to provide
     *      custom equivalence logic.
     * 
     * @example
     *      bool eq = system_category().equivalent(error_code(ENOENT, generic_category()), 
     *                                             ERROR_FILE_NOT_FOUND);
     *      // true on Windows
     */
    virtual bool equivalent(const error_code& err, int cond) const;
};

    inline string error_code::get_message(tca::allocator* alloc) const {
        return category->get_message(get_value(), alloc);
    }
    
    inline string error_condition::get_message(tca::allocator* alloc) const {
        return category->get_message(get_value(), alloc);
    }

    /**
     * Compares two error conditions for equality.
     * 
     * Two error conditions are equal if both their values and categories match.
     * 
     * @param l
     *      Left-hand side error condition.
     * 
     * @param r
     *      Right-hand side error condition.
     * 
     * @return
     *      true if equal, false otherwise.
     * 
     * @example
     *      error_condition ec1(ENOENT, generic_category());
     *      error_condition ec2(ENOENT, generic_category());
     *      error_condition ec3(EACCES, generic_category());
     *      
     *      ec1 == ec2;  // true
     *      ec1 == ec3;  // false
     */
    inline bool operator== (const error_condition& l, const error_condition& r) {
        return (l.get_value() == r.get_value()) && (l.get_category() == r.get_category());
    }
    
    /**
     * Compares two error conditions for inequality.
     * 
     * @param l
     *      Left-hand side error condition.
     * 
     * @param r
     *      Right-hand side error condition.
     * 
     * @return
     *      true if not equal, false otherwise.
     */
    inline bool operator!= (const error_condition& l, const error_condition& r) {
        return !(l == r);
    }

    inline bool operator== (const error_code& l, const error_condition& r) {
        if (l.get_category().equivalent(l.get_value(), r))
        {
            return true;
        }
        else if (r.get_category().equivalent(l, r.get_value()))
        {
            return true;
        }
        return false;
    }
}

#endif /* B7D7A5CF_9BC8_4925_BDFD_B4A8369B7151 */
