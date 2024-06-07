// Source: https://github.com/bloc97/Anime4K/blob/master/glsl/Denoise/Anime4K_Denoise_Bilateral_Mean.glsl

Texture2D tex : register(t0);
SamplerState smp : register(s0);

cbuffer cb0 : register(b0)
{
	float2 texel_size; // x y
	int radius; // z
	
	//Spatial window size, higher is stronger denoise, must be a positive real number.
	float sigma_spatial; // w
	
	//Intensity window size, higher is stronger denoise, must be a positive real number.
	float sigma_intensity; // xx
	
}

struct Vs_out
{
    float4 position : SV_POSITION; // xyzw
    float2 texcoord : TEXCOORD; // uv
};

#define gaussian(x,s,m) (exp(-((x) - (m)) * ((x) - (m)) / (2.0 * (s) * (s))))

float4 main(Vs_out vs_out) : SV_Target
{
	float4 sum = 0.0;
	float4 n = 0.0;
	const float4 vc = tex.SampleLevel(smp, vs_out.texcoord, 0.0);
	const int kernel_size = radius * 2 + 1;
	for (int i = 0; i < kernel_size * kernel_size; ++i) {
		
		// Get offset.
		const float2 ipos = float2((i % kernel_size) - radius, (i / kernel_size) - radius);
		
		const float4 v = tex.SampleLevel(smp, vs_out.texcoord + ipos * texel_size, 0.0);
		const float4 d = gaussian(length(ipos), sigma_spatial, 0.0) * gaussian(v, sigma_intensity, vc);
		sum += d * v;
		n += d;
	}
	return sum / n;
}