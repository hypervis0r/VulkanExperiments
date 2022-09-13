#include "files.h"

namespace Engine
{
	void Filesystem::ReadFile(const std::string& Filename, std::vector<char>& data)
	{
		std::ifstream file(Filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) 
		{
			throw std::runtime_error("Failed to open file.");
		}

		auto fileSize = static_cast<size_t>(file.tellg());

		data.resize(fileSize);

		file.seekg(0);
		file.read(data.data(), fileSize);

		file.close();
	}
}