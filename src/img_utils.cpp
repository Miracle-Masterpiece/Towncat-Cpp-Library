#include <internal/img_utils.hpp>
#include <cpp/lang/array.hpp>
#include <cpp/lang/math.hpp>
#include <cassert>

/**
 * Image resizing implementation using Mitchell-Netravali cubic filter.
 * 
 * This module provides high-quality image resizing using a separable filter approach
 * with the Mitchell-Netravali cubic kernel (B = 1/3, C = 1/3).
 * 
 * The resizing is performed in two passes: first horizontally, then vertically.
 * Each pass uses the Mitchell-Netravali filter with a radius of 2 for smooth,
 * artifact-free scaling.
 * 
 * The algorithm works by:
 * 1. Precomputing filter contributions for each output pixel in each dimension
 * 2. Applying horizontal convolution to a temporary buffer
 * 3. Applying vertical convolution to produce the final output
 * 
 * @example
 *      // Resize RGB image to 800x600
 *      int result = tc::internal::resize(input_data, 1920, 1080, 
 *                                         output_data, 800, 600, 3);
 *      if (result == 0) {
 *          // Success
 *      }
 */
namespace tc
{
namespace internal
{

/**
 * Per-pixel filter contribution data.
 * 
 * For each output pixel, this structure stores the range of input pixels
 * that contribute to it, along with their corresponding weights.
 * 
 * @note
 *      The weights are normalized so that they sum to 1.0, preserving
 *      overall image brightness during resizing.
 */
struct contribution {
    // Starting index of contributing input pixels.
    int start;
    
    // Ending index of contributing input pixels (inclusive)
    int end;
    
    // Normalized weights for each contributing pixel.
    tc::array<float> weights;
};

/**
 * Mitchell-Netravali cubic filter kernel.
 * 
 * A cubic spline filter with B = 1/3 and C = 1/3. 
 * 
 * @param x
 *      The distance from the center (absolute value).
 * 
 * @return
 *      The filter weight at distance x.
 * 
 * @note
 *      This filter provides a good balance between sharpness and ringing.
 *      It is widely used in image processing applications.
 * 
 * @see
 *      https://en.wikipedia.org/wiki/Mitchell%E2%80%93Netravali_filters
 */
static float mitchel(float x) {
    x = tc::math::abs(x);
    
    const float B = 1.0f / 3.0f;
    const float C = 1.0f / 3.0f;
    
    if (x < 1)
    {
        return ((12 - 9*B - 6*C) * (x*x*x) + (-18 + 12*B + 6*C) * (x*x) + (6 - 2*B)) * (1.0f / 6.0f);
    }
    else if (x < 2)
    {
        return ((-B - 6*C) * (x*x*x) + (6*B + 30*C) * (x*x) + (-12*B - 48*C) * x + (8*B + 24*C)) * (1.0f / 6.0f);
    }
    else
    {
        return 0;
    }
}

#if 0
static float linear(float x) {
    x = tc::math::abs(x);
    
    if (x < 1.0f)
    {
        return 1.0f - x;
    }
    
    return 0.0f;
}

static float sinc(float x) {
    return math::sin(x * math::PI) / (x * math::PI);
}

static float lanczos(float x) {
    if (x == 0) return 1;
    const float RAD = 2.0f;
    
    x = math::abs(x);
    if (x < RAD)
    {
        return sinc(x) * sinc(x / RAD);
    }

    return 0;
}
#endif

/**
 * Calculates filter weights for a single output pixel.
 * 
 * For a given output pixel position, this function computes the weights
 * for all input pixels that contribute to it based on the Mitchell-Netravali filter.
 * 
 * The algorithm:
 * 1. For each input pixel in the range [start, start+count):
 *    - Calculate the distance from the input pixel center to the output position
 *    - Apply scaling factor if downscaling (scale_factor < 1.0)
 *    - Compute filter weight using Mitchell kernel
 * 2. Normalize weights so they sum to 1.0
 * 
 * @param weights
 *      Output array to store calculated weights.
 * 
 * @param center
 *      The floating-point center position of the output pixel.
 * 
 * @param start
 *      The starting input pixel index.
 * 
 * @param count
 *      The number of contributing input pixels.
 * 
 * @param scale_factor
 *      The scaling factor for this dimension (w_out / w_in or h_out / h_in).
 * 
 * @note
 *      When downscaling (scale_factor < 1.0), the distances are multiplied
 *      by the scale factor to effectively broaden the filter's reach and
 *      prevent aliasing (anti-aliasing).
 * 
 * @note
 *      Weights are normalized by dividing by their sum to preserve
 *      overall brightness in the output image.
 */
static void calc_weights(tc::array<float>& weights, float center, int start, int count, float scale_factor) {
    float sum_weights = 0.0f;
    for (int i = 0; i < count; ++i)
    {
        int x     = start + i;
        float dst = math::abs(static_cast<float>(x) + 0.5f - center);
        
        if (scale_factor < 1.0f)
        {
            dst *= scale_factor;
        }
        
        float weight = mitchel(dst);

        weights[i]   = weight;
        sum_weights += weight;
    }

    if (sum_weights > 0.0f)
    {
        for (int i = 0; i < count; ++i)
        {
        weights[i] /= sum_weights;
        }
    }
}

/**
 * Initializes filter contributions for one dimension.
 * 
 * Precomputes the contribution structure for each output pixel in a dimension.
 * For each output pixel, this function determines which input pixels
 * contribute to it and calculates their weights using the Mitchell-Netravali filter.
 * 
 * @param contrib_param
 *      Pointer to contribution array to fill.
 * 
 * @param in_size
 *      The input dimension size (w_in or h_in).
 * 
 * @param out_size
 *      The output dimension size (w_out or h_out).
 * 
 * @param alloc
 *      Allocator for allocate weight buffers
 * 
 * @note
 *      The contribution array must be pre-allocated with out_size elements.
 * 
 * @note
 *      The sampling radius depends on the scaling direction:
 *      - Upscaling (out_size > in_size): fixed radius of 2 pixels
 *      - Downscaling (out_size < in_size): radius of 2/scale to prevent aliasing
 * 
 * @note
 *      Mitchell-Netravali uses radius 2 (support of 4 pixels on each side).
 *      This provides good quality with reasonable performance.
 */
static void init_contribution(contribution* contrib_param, int w_in, int w_out, tca::allocator* alloc) {

    // The ratio of the new to the old one
    float wscale    = static_cast<float>(w_out) / static_cast<float>(w_in);

    // Sampling radius
    // When downscaling, we need a larger input radius
    const float RAD = 2.0f;
    
    // const float RAD = 1.0f;

    {
        float xrad = wscale < 1.0f ? (RAD / wscale) : RAD;
        for (int x = 0; x < w_out; ++x)
        {
            // Center of the output pixel in input pixel coordinates
            // 
            // The +0.5f aligns pixel centers correctly (pixel center convention)
            float center = (static_cast<float>(x) + 0.5f) / wscale;
            
            // Determine the range of input pixels that contribute

            int start    = math::clamp( 
                static_cast<int>( math::floor(center - xrad) ), 0, w_in - 1 
            );
            int end      = math::clamp(
                static_cast<int>( math::ceil (center + xrad) ), 0, w_in - 1
            );
            
            contribution* contrib = contrib_param + x;

            int count         = end - start + 1;
            contrib->start    = start;
            contrib->end      = end;
            contrib->weights  = array<float>(count, alloc);

            // Calculate actual weights for all contributing input pixels
            calc_weights(contrib->weights, center, start, count, wscale);
        }
    }
}

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
 * @return 0 on success, non-zero on error.
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
 * 
 * @example
 *      // Resize a 1920x1080 RGB image to 800x600
 *      unsigned char* input = load_image("input.jpg");
 *      unsigned char* output = new unsigned char[800 * 600 * 3];
 *      int result = tc::internal::resize(input, 1920, 1080, 
 *                                         output, 800, 600, 3);
 * 
 * @see
 *      init_contribution
 *      mitchel
 *      calc_weights
 */
int resize_image(const unsigned char* in, int w_in, int h_in, unsigned char* out, int w_out, int h_out, int channels, tca::allocator* alloc) {
    #if 0
    tc::array<const unsigned char>  input (in, static_cast<std::size_t> (w_in * h_in * channels));
    tc::array<unsigned char>        output(out, static_cast<std::size_t>(w_out * h_out * channels));
    
    tc::array<float> tmp (w_out * h_in * channels, alloc);

    // An attempt to reduce the number of selections. One large array is allocated that stores data for rows and columns.
    tc::array<contribution> contributes(w_out + h_out, alloc);
    
    tc::array<contribution> contrib_x (contributes.data(),  w_out);    
    tc::array<contribution> contrib_y (contributes.data() + w_out, h_out);

    init_contribution(contrib_x.data(), w_in, w_out, alloc);
    init_contribution(contrib_y.data(), h_in, h_out, alloc);

    {// width
        for (int y = 0; y < h_in; ++y)
        {
            for (int x = 0; x < w_out; ++x)
            {
                int idx = (x + y * w_out) * channels;
                contribution& contrib = contrib_x[x];
                for (int c = 0; c < channels; ++c)
                {
                    float result = 0;
                    int count = (contrib.end - contrib.start) + 1;
                    for (int i = 0; i < count; ++i)
                    {
                        int from_x   = contrib.start + i;
                        int from_idx = (from_x + y * w_in) * channels;
                        result += static_cast<float>(input[from_idx + c]) * contrib.weights[i];
                    }
                    tmp[idx + c] = result;
                }
            }
        }
    }

    {// height
        for (int x = 0; x < w_out; ++x)
        {
            for (int y = 0; y < h_out; ++y)
            {
                int idx = (x + y * w_out) * channels;
                const contribution& contrib = contrib_y[y];
                for (int c = 0; c < channels; ++c)
                {
                    float result = 0;
                    int count = (contrib.end - contrib.start) + 1;
                    for (int i = 0; i < count; ++i)
                    {
                        int from_y   = contrib.start + i;
                        int from_idx = (x + from_y * w_out) * channels;
                        result += tmp[from_idx + c] * contrib.weights[i];
                    }
                    output[idx + c] = static_cast<unsigned char>(math::clamp(result, 0.0f, 255.0f));
                }
            }
        }
    }
    return 0;
    #endif 
}

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
int resize_image_alpha(const unsigned char* in, int w_in, int h_in, unsigned char* out, int w_out, int h_out, int channels, int alpha_index, tca::allocator* alloc) {
    #if 0
    const std::size_t MAX_CHANNELS = 4;

    if (alpha_index >= channels)
        return EINVAL;
        
    tc::array<const unsigned char>  input (in, static_cast<std::size_t> (w_in * h_in * channels));
    tc::array<unsigned char>        output(out, static_cast<std::size_t>(w_out * h_out * channels));
    
    tc::array<float> tmp (w_out * h_in * channels, alloc);

    // An attempt to reduce the number of selections. One large array is allocated that stores data for rows and columns.
    tc::array<contribution> contributes(w_out + h_out, alloc);
    
    tc::array<contribution> contrib_x (contributes.data(),  w_out);    
    tc::array<contribution> contrib_y (contributes.data() + w_out, h_out);

    init_contribution(contrib_x.data(), w_in, w_out, alloc);
    init_contribution(contrib_y.data(), h_in, h_out, alloc);

    {// width
        for (int y = 0; y < h_in; ++y)
        {
            for (int x = 0; x < w_out; ++x)
            {
                int idx = (x + y * w_out) * channels;
                contribution& contrib = contrib_x[x];
                for (int c = 0; c < channels; ++c)
                {
                    float result = 0;
                    int count = (contrib.end - contrib.start) + 1;
                    for (int i = 0; i < count; ++i)
                    {
                        int from_x   = contrib.start + i;
                        int from_idx = (from_x + y * w_in) * channels;
                        float pixel = static_cast<float>(input[from_idx + c]);
                        
                        if (alpha_index >= 0 && c != alpha_index)
                        {
                            float a = static_cast<float>(input[from_idx + alpha_index]) / 255.0f;
                            pixel *= a;
                        }

                        result += pixel * contrib.weights[i];

                    }
                    tmp[idx + c] = result;
                }
            }
        }
    }

    {// height
        for (int x = 0; x < w_out; ++x)
        {
            for (int y = 0; y < h_out; ++y)
            {
                int idx = (x + y * w_out) * channels;
                const contribution& contrib = contrib_y[y];
                
                float pixels[MAX_CHANNELS] = {0.0f};
                for (int c = 0; c < channels; ++c)
                {
                    int count = (contrib.end - contrib.start) + 1;
                    for (int i = 0; i < count; ++i)
                    {
                        int from_y   = contrib.start + i;
                        int from_idx = (x + from_y * w_out) * channels;
                        pixels[c]   += tmp[from_idx + c] * contrib.weights[i];
                    }
                }
                
                float alpha = 1.0f;
                if (alpha_index >= 0)
                {
                    alpha = pixels[alpha_index] / 255.0f;
                }

                for (int c = 0; c < channels; ++c)
                {
                    float pixel = pixels[c];
                    if (alpha_index >= 0 && c != alpha_index)
                    {
                        if (alpha >= 0.001f)
                        {
                            pixel /= alpha;
                        }
                        else
                        {
                            pixel = 0.0f;
                        }
                    }
                    output[idx + c] = static_cast<unsigned char>(math::clamp(pixel, 0.0f, 255.0f));
                }
                
            }
        }
    }
    return 0;
    #endif 
}

} //namespace internal
} //namespace tc