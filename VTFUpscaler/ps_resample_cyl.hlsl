// Cylindrical resampling, Jinc based (not separable).

#define USE_JINC_BASE
#include "kernel_functions.hlsli"

Texture2D tex : register(t0);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
    float2 dims; // x y
    float2 texel_size; // z w
    int index; // xx
    float radius; // yy
    float ceil_radius; // zz
    float blur; // ww
    
    // Free parameters.
    float p1; // xxx
    float p2; // yyy
    
    // Antiringing strenght.
    bool use_antiringing; // zzz
    float antiringing_amount; // www
}

struct Vs_out
{
    float4 position : SV_POSITION; // xyzw
    float2 texcoord : TEXCOORD; // uv
};

float get_weight(float x)
{
    if (x < radius) {
        [forcecase] switch (index) {
            case VTFU_KERNEL_FUNCTION_LANCZOS:
                return base(x, blur) * jinc(x, radius); // EWA Lanczos.
            case VTFU_KERNEL_FUNCTION_GINSENG:
                return base(x, blur) * sinc(x, radius); // EWA Ginseng.
            case VTFU_KERNEL_FUNCTION_HAMMING:
                return base(x, blur) * hamming(x, radius);
            case VTFU_KERNEL_FUNCTION_POW_COSINE:
                return base(x, blur) * power_of_cosine(x, radius, p1);
            case VTFU_KERNEL_FUNCTION_KAISER:
                return base(x, blur) * kaiser(x, radius, p1);
            case VTFU_KERNEL_FUNCTION_POW_GARAMOND:
                return base(x, blur) * power_of_garamond(x, radius, p1, p2);
            case VTFU_KERNEL_FUNCTION_POW_BLACKMAN:
                return base(x, blur) * power_of_blackman(x, radius, p1, p2);
            case VTFU_KERNEL_FUNCTION_GNW:
                return base(x, blur) * generalized_normal_window(x, p1, p2);
            case VTFU_KERNEL_FUNCTION_SAID:
                return base(x, blur) * said(x, p1, p2);
            case VTFU_KERNEL_FUNCTION_NEAREST:
                return nearest_neighbor(x);
            case VTFU_KERNEL_FUNCTION_LINEAR:
                return linear_kernel(x);
            case VTFU_KERNEL_FUNCTION_BICUBIC:
                return bicubic(x, p1);
            case VTFU_KERNEL_FUNCTION_MOD_FSR:
                return modified_fsr_kernel(x, p1, p2);
            case VTFU_KERNEL_FUNCTION_BCSPLINE:
                return bc_spline(x, p1, p2);
            default: // Black image.
                return 0.0;
        }
    }
    else // x >= radius
        return 0.0;
}

float4 main(Vs_out vs_out) : SV_TARGET
{
    const float2 fcoord = frac(vs_out.texcoord * dims - 0.5);
    const float2 base = vs_out.texcoord - fcoord * texel_size;
    float4 color;
    float4 csum = 0.0; // Weighted color sum.
    float weight;
    float wsum = 0.0; // Weight sum.

    // Antiringing.
    float4 lo = 1e9;
    float4 hi = -1e9;
    
    for (float j = 1.0 - ceil_radius; j <= ceil_radius; ++j) {
        for (float i = 1.0 - ceil_radius; i <= ceil_radius; ++i) {
            color = tex.SampleLevel(smp, base + texel_size * float2(i, j), 0.0);
            weight = get_weight(length(float2(i, j) - fcoord));
            csum += color * weight;
            wsum += weight;

            // Antiringing.
            if (use_antiringing && j >= 0.0 && j <= 1.0 && i >= 0.0 && i <= 1.0) {
                lo = min(lo, color);
                hi = max(hi, color);
            }
        }
    }
    csum /= wsum;

    // Antiringing.
    if (use_antiringing)
        return lerp(csum, clamp(csum, lo, hi), antiringing_amount);

    return csum;
}