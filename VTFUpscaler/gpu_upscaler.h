#pragma once

#include "pch.h"

class Gpu_upscaler
{
public:
	void init();
	void upscale(const void* data, const std::filesystem::path& path);
private:
	void create_device();
	void create_vertex_shader() const;
	void create_sampler() const;
	void create_image(const void* data);
	void pass_denoise();
	void pass_resample_ortho();
	void pass_resample_cyl();
	void pass_unsharp();
	void pass_rcas();
	void create_pixel_shader(const BYTE* shader, size_t shader_size) const;
	void create_constant_buffer(UINT byte_width, const void* data, ID3D11Buffer** buffer) const;
	void update_constant_buffer(ID3D11Buffer* buffer, const void* data, size_t size) const;
	void draw_pass(UINT width, UINT height);
	void create_viewport(float width, float height) const;
	void save_image(const std::filesystem::path& path);
	Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_device_context;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv_pass;
};