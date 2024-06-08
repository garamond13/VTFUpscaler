Texture2D tex : register(t0);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
    float amount; // x
    float size; // y
};

struct Vs_out
{
    float4 position : SV_POSITION; // xyzw
    float2 texcoord : TEXCOORD; // uv
};

float4 main(Vs_out vs_out) : SV_Target
{
    // Generate random noise.
    const float noise = (frac(sin(dot(vs_out.texcoord, float2(12.9898, 78.233))) * 43758.5453) - 0.5) * 2.0;
    
    return tex.SampleLevel(smp, vs_out.texcoord, 0.0) + noise * amount * size;
}