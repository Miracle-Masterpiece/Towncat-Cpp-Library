#ifndef B5464EB9_1F56_49AE_BCA6_C50E374D56C6
#define B5464EB9_1F56_49AE_BCA6_C50E374D56C6

#include <cpp/lang/errors.hpp>

namespace tc
{
namespace internal
{
namespace io
{
    
    /**
     *  Генерирует исключение, связанное с передаваемым 'error_code'
     *  Если значение 'error_code' равно нулю, функция ничего не делает.
     * 
     * @throws io_exception
     *      Если код ошибки указывает на ошибку ввода-вывода.
     * 
     * @throws security_exception
     *      Если код ошибки указывает на ошибку доступа прав.
     */
    void throw_error_code(const error_code& err);

} //namespace io
} //namespace internal
} //namespace tc

#endif /* B5464EB9_1F56_49AE_BCA6_C50E374D56C6 */
