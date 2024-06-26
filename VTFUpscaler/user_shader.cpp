#include "pch.h"
#include "user_shader.h"
#include "global.h"

User_shader::User_shader(const std::filesystem::path& path) : m_passes(), m_hooked()
{
	// For the convinience.
	// It should get optimized out by the compiler.
	const auto& ln = std::char_traits<char>::length;

	std::ifstream file(path);
	if (!file.is_open())
		throw std::runtime_error("ERROR: Couldn't open user shader " + path.string() + '.');
	std::string line;
	std::string buffer;
	std::vector<D3D_SHADER_MACRO> shader_macros;
	std::vector<std::pair<std::string, std::string>> shader_macros_data;
	int index_pass = -1;
	while (std::getline(file, line)) {
		if (line[0] == '/' && line[1] == '/') {
			if (line[2] == '!') {
				if (line.substr(ln("//!"), ln("API_V")) == "API_V") {
					if (line.substr(ln("//!API_V")) == std::to_string(m_api_v))
						continue;
					else
						throw std::runtime_error("ERROR: Unsupported user shader API_V" + line.substr(ln("//!API_V")) + ". In user shader " + path.string() + '.');
				}
				else if (line.substr(ln("//!"), ln("HOOK")) == "HOOK") {
					if (line.substr(ln("//!HOOK ")) == "MAIN") {
						m_hooked = HOOK_MAIN;
						continue;
					}
					else if (line.substr(ln("//!HOOK ")) == "DENOISE") {
						m_hooked = HOOK_DENOISE;
						continue;
					}
					else if (line.substr(ln("//!HOOK ")) == "SCALE") {
						m_hooked = HOOK_SCALE;
						continue;
					}
					else if (line.substr(ln("//!HOOK ")) == "SHARPEN") {
						m_hooked = HOOK_SHARPEN;
						continue;
					}
					else if (line.substr(ln("//!HOOK ")) == "GRAIN") {
						m_hooked = HOOK_GRAIN;
						continue;
					}
					else
						throw std::runtime_error("ERROR: Unknown HOOK: " + line.substr(ln("//!HOOK ")) + ". In user shader " + path.string() + '.');
				}
				else if (line.substr(ln("//!")) == "BEGIN_USER_CONFIG")
					continue;
				else if (line.substr(ln("//!")) == "END_USER_CONFIG") {
					std::stringstream ss(buffer);
					while (std::getline(ss, line)) {
						
						// Remove comments.
						auto pos = line.find("//");
						if (pos != std::string::npos)
							line.erase(pos);

						// Remove all white spaces.
						line.erase(std::remove_if(line.begin(), line.end(), [](unsigned char c) { return std::isspace(c); }), line.end());
						
						pos = line.find('=');
						if (pos != std::string::npos)
							shader_macros_data.push_back(std::pair(line.substr(0, pos), line.substr(pos + 1)));
					}

					// The last structure in the array serves as a terminator and must have all members set to NULL.
					shader_macros.resize(shader_macros_data.size() + 1);

					for (int i = 0; i < shader_macros_data.size(); ++i)
						shader_macros[i] = D3D_SHADER_MACRO(shader_macros_data[i].first.c_str(), shader_macros_data[i].second.c_str());
					buffer.clear();
					continue;
				}
				else if (line.substr(ln("//!")) == "BEGIN_PASS") {
					m_passes.push_back(User_shader_pass());
					++index_pass;
					continue;
				}
				else if (line.substr(ln("//!"), ln("WIDTH")) == "WIDTH") {
					if (line.substr(ln("//!WIDTH ")) == "SRC") {
						m_passes[index_pass].m_width = &g_src_width;
						continue;
					}
					else if (line.substr(ln("//!WIDTH ")) == "DST") {
						m_passes[index_pass].m_width = &g_dst_width;
						continue;
					}
					else
						throw std::runtime_error("ERROR: Unknown WIDTH: " + line.substr(ln("//!WIDTH ")) + ". In user shader " + path.string() + '.');
				}
				else if (line.substr(ln("//!"), ln("HEIGTH")) == "HEIGTH") {
					if (line.substr(ln("//!HEIGTH ")) == "SRC") {
						m_passes[index_pass].m_heigth = &g_src_height;
						continue;
					}
					else if (line.substr(ln("//!HEIGTH ")) == "DST") {
						m_passes[index_pass].m_heigth = &g_dst_height;
						continue;
					}
					else
						throw std::runtime_error("ERROR: Unknown HEIGTH: " + line.substr(ln("//!HEIGTH ")) + ". In user shader " + path.string() + '.');
				}
				else if (line.substr(ln("//!"), ln("SAVE")) == "SAVE") {
					strtoval(line.substr(ln("//!SAVE ")), m_passes[index_pass].m_save);
					continue;
				}
				else if (line.substr(ln("//!"), ln("BIND")) == "BIND") {
					int bind;
					strtoval(line.substr(ln("//!BIND ")), bind);
					m_passes[index_pass].m_binds.push_back(bind);
					continue;
				}

				// Compile pass.
				else if (line.substr(ln("//!")) == "END_PASS") {

					#ifndef NDEBUG
					const UINT flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
					#else
					const UINT flags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
					#endif

					Microsoft::WRL::ComPtr<ID3DBlob> error;
					D3DCompile(buffer.c_str(), buffer.size(), nullptr, shader_macros.data(), nullptr, "main", "ps_5_0", flags, 0, m_passes[index_pass].m_data.ReleaseAndGetAddressOf(), error.ReleaseAndGetAddressOf());
					if (error)
						throw std::runtime_error("ERROR: Faild to compile user shader pass " + std::to_string(index_pass) + " with error: " + reinterpret_cast<const char*>(error->GetBufferPointer()) + ". In user shader " + path.string() + '.');
					buffer.clear();
					continue;
				}

			}
			else
				continue;
		}
		buffer += line + '\n';
	}
}