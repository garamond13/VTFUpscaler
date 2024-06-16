//!API_V1
//!HOOK MAIN

//!BEGIN_USER_CONFIG

sigma=1.0 // Blur spread or amount, (0.0, 10+].
radius=2 // Kernel radius, (0, 10+].

//!END_USER_CONFIG

//!BEGIN_PASS

Texture2D tex : register(t0);
SamplerState smp : register(s0);

struct Vs_out
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

#define get_weight(x) (exp(-(x) * (x) / (2.0 * sigma * sigma)))

float4 main(Vs_out vs_out) : SV_Target
{
	float2 dims;
	tex.GetDimensions(dims.x, dims.y);
	const float2 texel_size = 1.0 / dims;
	float weight;
	float4 csum = tex.SampleLevel(smp, vs_out.texcoord, 0.0);
	float wsum = 1.0;
	for (int i = 1; i <= radius; ++i) {
		weight = get_weight(i);
		csum += (tex.SampleLevel(smp, vs_out.texcoord + texel_size * float2(0.0, -i), 0.0) + tex.SampleLevel(smp, vs_out.texcoord + texel_size * float2(0.0, i), 0.0)) * weight;
		wsum += 2.0 * weight;
	}
	return csum / wsum;
}

//!END_PASS

//!BEGIN_PASS

Texture2D tex : register(t0);
SamplerState smp : register(s0);

struct Vs_out
{
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD;
};

#define get_weight(x) (exp(-(x) * (x) / (2.0 * sigma * sigma)))

float4 main(Vs_out vs_out) : SV_Target
{
	float2 dims;
	tex.GetDimensions(dims.x, dims.y);
	const float2 texel_size = 1.0 / dims;
	float weight;
	float4 csum = tex.SampleLevel(smp, vs_out.texcoord, 0.0);
	float wsum = 1.0;
	for (float i = 1; i <= radius; ++i) {
		weight = get_weight(i);
		csum += (tex.SampleLevel(smp, vs_out.texcoord + texel_size * float2(-i, 0.0), 0.0) + tex.SampleLevel(smp, vs_out.texcoord + texel_size * float2(i, 0.0), 0.0)) * weight;
		wsum += 2.0 * weight;
	}
	return csum / wsum;
}

//!END_PASS