Texture2D tex : register(t0);
SamplerState smp : register(s0);

struct Vs_out
{
    float4 position : SV_POSITION; // xyzw
    float2 texcoord : TEXCOORD; // uv
};

float4 main(Vs_out vs_out) : SV_Target
{
    return tex.SampleLevel(smp, vs_out.texcoord, 0.0);
}
