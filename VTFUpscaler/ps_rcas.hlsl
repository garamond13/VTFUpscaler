// Robust contrast adaptive sharpening
// source: https://github.com/GPUOpen-Effects/FidelityFX-FSR

Texture2D tex : register(t0);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
    float2 texel_size; // x y
    float amount; // z
};

struct Vs_out
{
    float4 position : SV_POSITION; // xyzw
    float2 texcoord : TEXCOORD; // uv
};

// This is set at the limit of providing unnatural results for sharpening.
#define RCAS_LIMIT (0.25 - (1.0 / 16.0))

float4 main(Vs_out vs_out) : SV_Target
{
	// Algorithm uses minimal 3x3 pixel neighborhood.
	//    b 
	//  d e f
	//    h
	const float4 b = tex.SampleLevel(smp, vs_out.texcoord + texel_size * float2( 0.0, -1.0), 0.0);
	const float4 d = tex.SampleLevel(smp, vs_out.texcoord + texel_size * float2(-1.0,  0.0), 0.0);
	const float4 e = tex.SampleLevel(smp, vs_out.texcoord, 0.0);
	const float4 f = tex.SampleLevel(smp, vs_out.texcoord + texel_size * float2(1.0, 0.0), 0.0);
	const float4 h = tex.SampleLevel(smp, vs_out.texcoord + texel_size * float2(0.0, 1.0), 0.0);

	// Min and max of ring.
	const float4 mn4 = min(min(b, min(d, f)), h);
	const float4 mx4 = max(max(b, max(d, f)), h);
	
	// Immediate constants for peak range.
	const float2 peak = float2(1.0, -4.0);
	
	// Limiters.
	const float4 hit_min = min(mn4, e) / (4.0 * mx4);
	const float4 hit_max = (peak.x - max(mx4, e)) / (4.0 * mn4 + peak.y);
	const float4 lobe4 = max(-hit_min, hit_max);
	const float lobe = max(-RCAS_LIMIT, min(max(lobe4.r, max(lobe4.g, max(lobe4.b, lobe4.a))), 0.0)) * amount;
	
	return (lobe * b + lobe * d + lobe * h + lobe * f + e) / (4.0 * lobe + 1.0);
}