#ifndef C59AB98A_0F8F_4939_9471_57330AFE30A2
#define C59AB98A_0F8F_4939_9471_57330AFE30A2

#include <allocators/allocator.hpp>

namespace tc
{
namespace internal
{
    /**
     * Resizes an image using Mitchell-Netravali separable filter.
     * 
     * This function resizes an image to new dimensions using high-quality
     * Mitchell-Netravali cubic resampling. The process is performed in two
     * separable passes:
     * 1. Horizontal resizing (width) to a temporary buffer
     * 2. Vertical resizing (height) to the final output buffer
     * 
     * The separable approach is significantly faster than a full 2D convolution
     * while producing nearly identical results.
     * 
     * The Mitchell-Netravali filter (B = 1/3, C = 1/3) provides:
     * - Excellent image quality with minimal ringing
     * - Smooth transitions without overshoot
     * - Good sharpness retention
     * - Ideal for photographic images
     * 
     * @param in
     *      Input image data in row-major order.
     * 
     * @param w_in
     *      Input image width.
     * 
     * @param h_in
     *      Input image height.
     * 
     * @param out
     *      Output image data buffer (must be pre-allocated).
     * 
     * @param w_out
     *      Desired output width.
     * 
     * @param h_out
     *      Desired output height.
     * 
     * @param channels
     *      Number of color channels (1 for grayscale, 3 for RGB, 4 for RGBA).
     * 
     * @param alloc
     *      Allocator for temporary buffers.
     * 
     * @return 0
     *      On success, non-zero on error.
     * 
     * @note
     *      The algorithm uses a two-pass separable convolution:
     *      - First pass: Resize width using horizontal filter
     *      - Second pass: Resize height using vertical filter
     * 
     * @note
     *      Temporary buffer size: w_out * h_in * channels * sizeof(float)
     *      This can be memory-intensive for large images.
     * 
     * @warning
     *      The output buffer must be at least w_out * h_out * channels bytes.
     *      The input and output buffers must not overlap.
     */
    int resize_image(const unsigned char* in, int w_in, int h_in, unsigned char* out, int w_out, int h_out, int channels, tca::allocator* alloc = tca::get_default_allocator());

    /**
     * Resizes an image with alpha channel support using Mitchell-Netravali filter.
     * 
     * This function resizes an image with special handling for alpha channel.
     * Unlike standard resize, it properly handles premultiplied alpha by:
     * 1. During horizontal pass, non-alpha channels are multiplied by their alpha value
     * 2. During vertical pass, the alpha channel is processed separately
     * 3. After convolution, non-alpha channels are divided by alpha to restore
     *    the original color values (un-premultiplication)
     * 
     * This approach ensures that colors at the edges of transparent regions
     * don't bleed incorrectly, which is a common problem when resizing images
     * with alpha channel using naive filtering.
     * 
     * The algorithm:
     * 1. Horizontal pass: multiply each pixel's RGB by its alpha (premultiply)
     * 2. Vertical pass: process RGB and alpha separately
     * 3. Final: divide RGB by alpha to restore original colors (un-premultiply)
     * 
     * @param in
     *      Input image data in row-major order.
     * 
     * @param w_in
     *      Input image width.
     * 
     * @param h_in
     *      Input image height.
     * 
     * @param out
     *      Output image data buffer (must be pre-allocated).
     * 
     * @param w_out
     *      Desired output width.
     * 
     * @param h_out
     *      Desired output height.
     * 
     * @param channels
     *      Number of color channels (3 for RGB, 4 for RGBA).
     * 
     * @param alpha_index
     *      Index of the alpha channel (-1 if no alpha).
     * 
     * @param alloc
     *      Allocator for temporary buffers.
     * 
     * @return 
     *      0 on success, non-zero on error.
     * 
     * @note
     *      This function is essential for resizing images with transparency.
     *      Without this handling, transparent edges would show dark or light
     *      fringes (color bleeding from adjacent pixels).
     * 
     * @note
     *      The function uses the same Mitchell-Netravali filter as the standard
     *      resize, but with additional alpha-aware processing.
     * 
     * @warning
     *      The alpha_index must be valid (0..channels-1) if alpha is present.
     *      For RGB images (no alpha), pass alpha_index = -1.
     * 
     * 
     * This ensures that transparent pixels don't incorrectly contribute color
     * to the final result.
     */
    int resize_image_alpha(const unsigned char* in, int w_in, int h_in, unsigned char* out, int w_out, int h_out, int channels, int alpha_index, tca::allocator* alloc);

} // namespace internal
} // namespace tc

#endif /* C59AB98A_0F8F_4939_9471_57330AFE30A2 */
