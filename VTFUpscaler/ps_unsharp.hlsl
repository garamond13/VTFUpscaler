// Unsharp mask (separated).

Texture2D tex : register(t0);
Texture2D tex_original : register(t1);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
    float2 texel_size; // x y
    int radius; // z
    float sigma; // w
    
    // Has to be <= 0 on the first pass!
    float amount; // xx
};

struct Vs_out
{
    float4 position : SV_POSITION; // xyzw
    float2 texcoord : TEXCOORD; // uv
};

// Normalized version is divided by sqrt(2 * pi * sigma * sigma).
#define get_weight(x) (exp(-(x) * (x) / (2.0 * sigma * sigma)))

// Samples one axis (x or y) at a time.
float4 main(Vs_out vs_out) : SV_Target
{
    float weight;
    float4 csum = tex.SampleLevel(smp, vs_out.texcoord, 0.0); // Weighted color sum.
    float wsum = 1.0; // Weight sum.
    for (int i = 1; i <= radius; ++i) {
        weight = get_weight(float(i));
        csum += (tex.SampleLevel(smp, vs_out.texcoord + texel_size * float(-i), 0.0) + tex.SampleLevel(smp, vs_out.texcoord + texel_size * float(i), 0.0)) * weight;
        wsum += 2.0 * weight;
    }
    
    // Only used in the last pass.
    if (amount > 0.0) {
        const float4 original = tex_original.SampleLevel(smp, vs_out.texcoord, 0.0);
        return original + (original - csum / wsum) * amount;
    }
    
    return csum / wsum;
}
