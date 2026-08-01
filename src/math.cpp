#include <cpp/lang/math/math.hpp>
#include <cpp/lang/utils/random.hpp>

namespace tc
{
namespace math
{
    double random() {
        thread_local static class random rnd;
        return rnd.next<double>();
    }

}// namespace jstd
}// namespace math