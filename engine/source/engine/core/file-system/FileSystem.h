#pragma once
#include "../EngineCore.h"
#include "../thread/Lock.h"
#include "../utility/TypeHelper.h"
#include <json/json.h>
#include <filesystem>

namespace longmarch
{
	namespace fs = std::filesystem;

	class FileSystem : BaseAtomicClassNI
	{
	public:
		enum class FileType {
			OPEN_BINARY = 0, /// Open In Binary Mode
			OPEN_TEXT = 1, /// Open In Text Mode
		};
	public:
		NONINSTANTIABLE(FileSystem);
		//! Path protocols are in the format : $xxx:filename.txt where "$xxx:" will be replaced by "path_to_file/"
		static void RegisterProtocol(const std::string& name, const fs::path& path);
		//<! Path protocols are in the format : $xxx:filename.txt where "$xxx:" will be replaced by "path_to_file/"
		static fs::path ResolveProtocol(const fs::path& path);
		static fs::path ResolveSingleBackSlash(const std::string& path);
		static fs::path Absolute(const fs::path& path);
		static fs::path CWD();
		static bool ExistCheck(const fs::path& path, bool throwOnNotExist = true);

		static std::ifstream& OpenIfstream(const fs::path& file, FileType type = FileType::OPEN_BINARY);
		static std::ofstream& OpenOfstream(const fs::path& file, FileType type = FileType::OPEN_BINARY);
		static void CloseIfstream(const fs::path& file);
		static void CloseOfstream(const fs::path& file);
		//! Get a brand new JsonCPP value
		static Json::Value GetNewJsonCPP(const fs::path& file);
		//! Try to get a cached value of that json file. Call RemoveCachedJsonCPP() after writing to a the same file for it to be updated.
		static Json::Value& GetCachedJsonCPP(const fs::path& file);
		static void RemoveCachedJsonCPP(const fs::path& file);

	private:
		inline static fs::path Absolute(const fs::path& p, std::error_code& ec);
		inline static  fs::path CWD(std::error_code& ec);
	private:
		inline static std::atomic_flag s_IfFlag;
		inline static LongMarch_UnorderedMap<std::string, std::ifstream> s_FileIStreamMap;

		inline static std::atomic_flag s_OfFlag;
		inline static LongMarch_UnorderedMap<std::string, std::ofstream> s_FileOStreamMap;

		inline static std::atomic_flag s_jsonFlag;
		inline static LongMarch_UnorderedMap<std::string, Json::Value> s_jsonCPPParserMap;

		inline static LongMarch_UnorderedMap<std::string, fs::path> s_pathProtocol;
	};
}