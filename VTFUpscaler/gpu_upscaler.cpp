#include "pch.h"
#include "gpu_upscaler.h"
#include "global.h"

// Shader byte code.
#include "vs_quad_hlsl.h"
#include "ps_sample_hlsl.h"
#include "ps_resample_ortho_hlsl.h"
#include "ps_resample_cyl_hlsl.h"
#include "ps_unsharp_hlsl.h"

union Cb_types
{
	float f;
	int32_t i;
};

// Used for constant buffer data.
struct Cb4
{
	Cb_types x;
	Cb_types y;
	Cb_types z;
	Cb_types w;
};

void Gpu_upscaler::init()
{
	create_device();
	create_vertex_shader();
	create_sampler();
}

void Gpu_upscaler::upscale(const void* data, const std::filesystem::path& path)
{
	create_image(data);
	if (g_config.m_use_jinc.val)
		pass_resample_cyl();
	else
		pass_resample_ortho();
	if (g_config.m_unsharp_enabeled.val)
		pass_unsharp();
	save_image(path);
}

void Gpu_upscaler::create_device()
{
	UINT flags = D3D11_CREATE_DEVICE_SINGLETHREADED;

#ifndef NDEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	static constinit const std::array feature_levels = {
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};
	if (FAILED(D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, flags, feature_levels.data(), feature_levels.size(), D3D11_SDK_VERSION, m_device.ReleaseAndGetAddressOf(), nullptr, m_device_context.ReleaseAndGetAddressOf())))
		throw std::runtime_error("ERROR: D3D11 failed to create device.");
}

void Gpu_upscaler::create_vertex_shader() const
{
	m_device_context->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertex_shader;
	vtfu_assert(m_device->CreateVertexShader(VS_QUAD, sizeof(VS_QUAD), nullptr, vertex_shader.ReleaseAndGetAddressOf()), == S_OK);
	m_device_context->VSSetShader(vertex_shader.Get(), nullptr, 0);
}

void Gpu_upscaler::create_sampler() const
{
	const D3D11_SAMPLER_DESC sampler_desc = {
		.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
		.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP,
		.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP,
		.MaxAnisotropy = 1,
		.ComparisonFunc = D3D11_COMPARISON_NEVER
	};
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler_state;
	vtfu_assert(m_device->CreateSamplerState(&sampler_desc, sampler_state.ReleaseAndGetAddressOf()), == S_OK);
	m_device_context->PSSetSamplers(0, 1, sampler_state.GetAddressOf());
}

void Gpu_upscaler::create_image(const void* data)
{
	const D3D11_TEXTURE2D_DESC texture2d_desc = {
		.Width = static_cast<UINT>(g_src_width),
		.Height = static_cast<UINT>(g_src_height),
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.SampleDesc = {
			.Count = 1
		},
		.Usage = D3D11_USAGE_IMMUTABLE,
		.BindFlags = D3D11_BIND_SHADER_RESOURCE
	};
	const D3D11_SUBRESOURCE_DATA subresource_data = {
		.pSysMem = data,
		.SysMemPitch = static_cast<UINT>(g_src_width) * 4
	};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
	vtfu_assert(m_device->CreateTexture2D(&texture2d_desc, &subresource_data, texture2d.ReleaseAndGetAddressOf()), == S_OK);
	vtfu_assert(m_device->CreateShaderResourceView(texture2d.Get(), nullptr, m_srv_image.ReleaseAndGetAddressOf()), == S_OK);
}

void Gpu_upscaler::pass_resample_ortho()
{
	create_pixel_shader(PS_RESAMPLE_ORTHO, sizeof(PS_RESAMPLE_ORTHO));

	// Pass y axis.
	alignas(16) std::array cb_data = {
		Cb4{
			.x = { .f = static_cast<float>(g_src_width) }, // dims.x
			.y = { .f = static_cast<float>(g_src_height) }, // dims.y
			.z = { .f = 0.0f }, // texel_size.x
			.w = { .f = 1.0f / static_cast<float>(g_src_height) } // texel_size.y
		},
		Cb4{
			.x = { .f = 0.0f }, // axis.x
			.y = { .f = 1.0f } , // axis.y
			.z = { .i = g_config.m_kernel_function.val }, // index
			.w = { .f = g_config.m_kernel_radius.val } // radius
		},
		Cb4{
			.x = { .f = std::ceil(g_config.m_kernel_radius.val) }, // blur
			.y = { .f = g_config.m_kernel_blur.val }, // blur
			.z = { .f = g_config.m_kernel_param1.val }, // p1
			.w = { .f = g_config.m_kernel_param2.val } // p2
		},
		Cb4{
			.x = { .i = g_config.m_antiringing_amount.val > 0.0f }, // use_antiringing
			.y = { .f = g_config.m_antiringing_amount.val } // antiringing_amount
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
	create_constant_buffer(sizeof(cb_data), &cb_data, cb0.ReleaseAndGetAddressOf());
	m_device_context->PSSetShaderResources(0, 1, m_srv_image.GetAddressOf());
	draw_pass(g_src_width, g_dst_height);

	// Pass x axis.
	cb_data[0].z.f = 1.0f / static_cast<float>(g_src_width); // texel_size.x
	cb_data[0].w.f = 0.0f; // texel_size.y
	cb_data[1].x.f = 1.0f; // axis.x
	cb_data[1].y.f = 0.0f; // axis.y
	update_constant_buffer(cb0.Get(), cb_data.data(), sizeof(cb_data));
	m_device_context->PSSetShaderResources(0, 1, m_srv_pass.GetAddressOf());
	draw_pass(g_dst_width, g_dst_height);
}

void Gpu_upscaler::pass_resample_cyl()
{
	create_pixel_shader(PS_RESAMPLE_CYL, sizeof(PS_RESAMPLE_CYL));
	const alignas(16) std::array cb_data = {
		Cb4{
			.x = { .f = static_cast<float>(g_src_width) }, // dims.x
			.y = { .f = static_cast<float>(g_src_height) }, // dims.y
			.z = { .f = 1.0f / static_cast<float>(g_src_width) }, // texel_size.x
			.w = { .f = 1.0f / static_cast<float>(g_src_height) } // texel_size.y
		},
		Cb4{
			.x = { .i = g_config.m_kernel_function.val }, // index
			.y = { .f = g_config.m_kernel_radius.val }, // radius
			.z = { .f = std::ceil(g_config.m_kernel_radius.val) }, // ceil_radius
			.w = { .f = g_config.m_kernel_blur.val } // blur
		},
		Cb4{
			.x = { .f = g_config.m_kernel_param1.val }, // p1
			.y = { .f = g_config.m_kernel_param2.val }, // p2
			.z = { .i = g_config.m_antiringing_amount.val > 0.0f }, // use_antiringing
			.w = { .f = g_config.m_antiringing_amount.val } // antiringing_amount
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
	create_constant_buffer(sizeof(cb_data), &cb_data, cb0.ReleaseAndGetAddressOf());
	m_device_context->PSSetShaderResources(0, 1, m_srv_image.GetAddressOf());
	draw_pass(g_dst_width, g_dst_height);
}

void Gpu_upscaler::pass_unsharp()
{
	create_pixel_shader(PS_UNSHARP, sizeof(PS_UNSHARP));

	// Pass y axis.
	alignas(16) std::array cb_data = {
		Cb4{
			.x = { .f = 0.0f }, // texel_size.x
			.y = { .f =  1.0f / static_cast<float>(g_dst_height) }, // texel_size.y
			.z = { .i = g_config.m_unsharp_radius.val }, // radius
			.w = { .f = g_config.m_unsharp_sigma.val } // sigma
		},
		Cb4{
			.x = { .f = -1.0f }, // amount // It has to be <=0 for the first pass!
		}
	};
	Microsoft::WRL::ComPtr<ID3D11Buffer> cb0;
	create_constant_buffer(sizeof(cb_data), &cb_data, cb0.ReleaseAndGetAddressOf());
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv_original = m_srv_pass;
	m_device_context->PSSetShaderResources(0, 1, m_srv_pass.GetAddressOf());
	draw_pass(g_dst_width, g_dst_height);

	// Pass x axis.
	cb_data[0].x.f = 1.0f / static_cast<float>(g_dst_width); // texel_size.x
	cb_data[0].y.f = 0.0f; // texel_size.y
	cb_data[1].x.f = g_config.m_unsharp_amount.val; // amount // Should be > 0.
	update_constant_buffer(cb0.Get(), cb_data.data(), sizeof(cb_data));
	const std::array srvs{ m_srv_pass.Get(), srv_original.Get() };
	m_device_context->PSSetShaderResources(0, srvs.size(), srvs.data());
	draw_pass(g_dst_width, g_dst_height);
}

void Gpu_upscaler::create_pixel_shader(const BYTE* shader, size_t shader_size) const
{
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixel_shader;
	vtfu_assert(m_device->CreatePixelShader(shader, shader_size, nullptr, pixel_shader.ReleaseAndGetAddressOf()), == S_OK);
	m_device_context->PSSetShader(pixel_shader.Get(), nullptr, 0);
}

void Gpu_upscaler::create_constant_buffer(UINT byte_width, const void* data, ID3D11Buffer** buffer) const
{
	const D3D11_BUFFER_DESC buffer_desc = {
		.ByteWidth = byte_width,
		.Usage = D3D11_USAGE_DYNAMIC,
		.BindFlags = D3D11_BIND_CONSTANT_BUFFER,
		.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE
	};
	const D3D11_SUBRESOURCE_DATA subresource_data = {
		.pSysMem = data
	};
	vtfu_assert(m_device->CreateBuffer(&buffer_desc, &subresource_data, buffer), == S_OK);
	m_device_context->PSSetConstantBuffers(0, 1, buffer);
}

void Gpu_upscaler::update_constant_buffer(ID3D11Buffer* buffer, const void* data, size_t size) const
{
	D3D11_MAPPED_SUBRESOURCE mapped_subresource;
	vtfu_assert(m_device_context->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource), == S_OK);
	std::memcpy(mapped_subresource.pData, data, size);
	m_device_context->Unmap(buffer, 0);
}

void Gpu_upscaler::draw_pass(UINT width, UINT height)
{
	// Create render target.
	const D3D11_TEXTURE2D_DESC texture2d_desc = {
		.Width = width,
		.Height = height,
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_R32G32B32A32_FLOAT,
		.SampleDesc = {
			.Count = 1
		},
		.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET
	};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
	vtfu_assert(m_device->CreateTexture2D(&texture2d_desc, nullptr, texture2d.ReleaseAndGetAddressOf()), == S_OK);
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
	vtfu_assert(m_device->CreateRenderTargetView(texture2d.Get(), nullptr, rtv.ReleaseAndGetAddressOf()), == S_OK);

	// Draw to the render target.
	m_device_context->OMSetRenderTargets(1, rtv.GetAddressOf(), nullptr);
	create_viewport(width, height);
	m_device_context->Draw(3, 0);

	// Unbind render target.
	m_device_context->OMSetRenderTargets(1, &static_cast<ID3D11RenderTargetView* const&>(0), nullptr);

	vtfu_assert(m_device->CreateShaderResourceView(texture2d.Get(), nullptr, m_srv_pass.ReleaseAndGetAddressOf()), == S_OK);
}

void Gpu_upscaler::create_viewport(float width, float height) const
{
	const D3D11_VIEWPORT viewport = {
		.Width = width,
		.Height = height
	};
	m_device_context->RSSetViewports(1, &viewport);
}

void Gpu_upscaler::save_image(const std::filesystem::path& path)
{
	// Create render target.
	D3D11_TEXTURE2D_DESC texture2d_desc = {
		.Width = static_cast<UINT>(g_dst_width),
		.Height = static_cast<UINT>(g_dst_height),
		.MipLevels = 1,
		.ArraySize = 1,
		.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
		.SampleDesc = {
			.Count = 1
		},
		.BindFlags = D3D11_BIND_RENDER_TARGET
	};
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d_render_target;
	vtfu_assert(m_device->CreateTexture2D(&texture2d_desc, nullptr, texture2d_render_target.ReleaseAndGetAddressOf()), == S_OK);
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> rtv;
	vtfu_assert(m_device->CreateRenderTargetView(texture2d_render_target.Get(), nullptr, rtv.ReleaseAndGetAddressOf()), == S_OK);

	// Draw to the render target.
	create_pixel_shader(PS_SAMPLE, sizeof(PS_SAMPLE));
	m_device_context->PSSetShaderResources(0, 1, m_srv_pass.GetAddressOf());
	m_device_context->OMSetRenderTargets(1, rtv.GetAddressOf(), nullptr);
	create_viewport(g_dst_width, g_dst_height);
	m_device_context->Draw(3, 0);

	// Unbind render target.
	m_device_context->OMSetRenderTargets(1, &static_cast<ID3D11RenderTargetView* const&>(0), nullptr);

	// Create staging texture.
	texture2d_desc.BindFlags = 0;
	texture2d_desc.Usage = D3D11_USAGE_STAGING;
	texture2d_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d_staging;
	vtfu_assert(m_device->CreateTexture2D(&texture2d_desc, nullptr, texture2d_staging.ReleaseAndGetAddressOf()), == S_OK);

	// Get data.
	m_device_context->CopyResource(texture2d_staging.Get(), texture2d_render_target.Get());
	D3D11_MAPPED_SUBRESOURCE mapped_resource;
	vtfu_assert(m_device_context->Map(texture2d_staging.Get(), 0, D3D11_MAP_READ, 0, &mapped_resource), == S_OK);

	// Its convinient to save VTF file here.
	VTFLib::CVTFFile vtf_dst;
	if (g_config.m_save_uncompressed.val) {
		if (g_vtf_create_options.ImageFormat == IMAGE_FORMAT_DXT1)
			g_vtf_create_options.ImageFormat = IMAGE_FORMAT_RGB888;
		else if (g_vtf_create_options.ImageFormat == IMAGE_FORMAT_DXT5)
			g_vtf_create_options.ImageFormat = IMAGE_FORMAT_RGBA8888;
	}
	if (vtf_dst.Create(g_dst_width, g_dst_height, reinterpret_cast<vlByte*>(mapped_resource.pData), g_vtf_create_options) == vlFalse)
		throw std::runtime_error("FAILED: " + path.string());
	if (vtf_dst.Save(path.string().c_str()) == vlFalse)
		throw std::runtime_error("FAILED: " + path.string());

	m_device_context->Unmap(texture2d_staging.Get(), 0);

	// Finish with flush.
	m_device_context->Flush();
}
