#pragma once

#include <vector>
#include <string>
#include <fstream>

namespace Engine
{
	static class Filesystem
	{
	public:
		static void ReadFile(const std::string& Filename, std::vector<char>& data);
	};
}