// MIT License

// Copyright (c) 2024 Ivan Bjeliš
// All rights reserved.

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

//!API_V1
//!HOOK MAIN

//!BEGIN_USER_CONFIG

sigma=1.0 // Blur spread or amount, (0.0, 10+].
radius=2 // Kernel radius, (0, 10+].

//!END_USER_CONFIG

//!BEGIN_PASS
//!WIDTH SRC
//!HEIGTH SRC

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
//!WIDTH SRC
//!HEIGTH SRC

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