// Reference: https://github.com/tntmeijs/GLSL-Shader-Includes

#pragma once

#include <string>
#include <fstream>
#include <iostream>
#include "engine/core/file-system/FileSystem.h"

namespace AAAAgames
{
	class A4GAMES_ShaderInclude
	{
	public:
		inline static std::string includeIndentifier{ "#include " };

		// Return the source code of the complete shader
		static std::string load(const fs::path& path, unsigned int level)
		{
			std::string fullSourceCode = "";
			std::ifstream file(path, std::ifstream::in);

			if (!file.is_open())
			{
				ENGINE_EXCEPT(L"ERROR: could not open the shader at: " + path.wstring());
			}

			std::string lineBuffer;
			while (std::getline(file, lineBuffer))
			{
				// Look for the new shader include identifier
				if (lineBuffer.find(includeIndentifier) != lineBuffer.npos)
				{
					// Remove the include identifier, this will cause the path to remain
					lineBuffer.erase(0, includeIndentifier.size());

					// The include path is relative to the current shader file path
					fs::path  dir = path;
					dir.remove_filename();
					fs::path relativePath(dir / fs::path(lineBuffer));
					auto absPath = FileSystem::Absolute(relativePath);
					// Include guard by absolute path
					if (std::find(fullPathIncludeGuardLUT.begin(), fullPathIncludeGuardLUT.end(), absPath) != fullPathIncludeGuardLUT.end())
					{
						continue;
					}
					else
					{
						fullPathIncludeGuardLUT.emplace_back(absPath);
					}

					// By using recursion, the new include file can be extracted
					// and inserted at this location in the shader source code
					fullSourceCode += load(relativePath, level + 1);

					// Do not add this line to the shader source code, as the include
					// path would generate a compilation issue in the final source code
					continue;
				}

				fullSourceCode += lineBuffer + '\n';
			}

			// Only add the null terminator at the end of the complete file,
			// essentially skipping recursive function calls this way
			if (level == 0)
			{
				fullSourceCode += '\0';
				fullPathIncludeGuardLUT.clear();
			}

			file.close();

			return fullSourceCode;
		}

	private:
		inline static std::vector<fs::path> fullPathIncludeGuardLUT;
	};
}