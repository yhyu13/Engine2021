#include "engine-precompiled-header.h"
#include "FileSystem.h"
#include "../exception/EngineException.h"

void longmarch::FileSystem::RegisterProtocol(const std::string& name, const fs::path& path)
{
	s_pathProtocol[name] = path;
}

fs::path longmarch::FileSystem::ResolveSingleBackSlash(const std::string& path)
{
	auto _path = path;
	std::replace(_path.begin(), _path.end(), '\\', '/');
	return fs::path(_path).make_preferred();
}

fs::path longmarch::FileSystem::ResolveProtocol(const fs::path& _path)
{
	const auto& string_path = _path.string();
	auto pos_1 = string_path.find('$', 0);
	auto pos_2 = string_path.find(':', 1);
	if (pos_1 != 0 || pos_2 == std::string::npos)
	{
		if (!string_path.empty())
		{
			DEBUG_PRINT("[FileSystem] Path protocol cannot be resolved : " + string_path);
		}
		return _path;
	}
	pos_2 += 1; // Protocol includes the ':' char
	const auto& root = string_path.substr(0, pos_2);
	fs::path relative_path = string_path.substr(pos_2);
	if (auto it = s_pathProtocol.find(root); it != s_pathProtocol.end())
	{
		auto result = it->second;
		if (!relative_path.empty())
		{
			result = result / relative_path.make_preferred();
		}
		return result.make_preferred();
	}
	else
	{
		ENGINE_EXCEPT(L"Path protocol is not registered : " + str2wstr(string_path));
		return _path;
	}
}

fs::path longmarch::FileSystem::Absolute(const fs::path& path)
{
	std::error_code ec;
	fs::path result = Absolute(path, ec);
	ENGINE_EXCEPT_IF(ec, L"ToAbsolute failed with " + wStr(ec));
	return result.make_preferred();
}

fs::path longmarch::FileSystem::Absolute(const fs::path& p, std::error_code& ec)
{
	ec.clear();
#if defined(WIN32) || defined(WINDOWS_APP)
	if (p.empty()) {
		return Absolute(CWD(ec), ec) / "";
	}
	ULONG size = ::GetFullPathNameW(p.wstring().c_str(), 0, 0, 0);
	if (size) {
		std::vector<wchar_t> buf(size, 0);
		ULONG s2 = GetFullPathNameW(p.wstring().c_str(), size, buf.data(), nullptr);
		if (s2 && s2 < size) {
			fs::path result = fs::path(std::wstring(buf.data(), s2));
			if (p.filename() == ".") {
				result /= ".";
			}
			return result;
		}
	}
	ec = std::error_code(::GetLastError(), std::system_category());
	return fs::path();
#else
	fs::path base = current_path(ec);
	fs::path absoluteBase = base.is_absolute() ? base : absolute(base, ec);
	if (!ec) {
		if (p.empty()) {
			return absoluteBase / p;
		}
		if (p.has_root_name()) {
			if (p.has_root_directory()) {
				return p;
			}
			else {
				return p.root_name() / absoluteBase.root_directory() / absoluteBase.relative_path() / p.relative_path();
			}
		}
		else {
			if (p.has_root_directory()) {
				return absoluteBase.root_name() / p;
			}
			else {
				return absoluteBase / p;
			}
		}
	}
	ec = std::error_code(errno, std::system_category());
	return fs::path();
#endif
}

fs::path longmarch::FileSystem::CWD()
{
	std::error_code ec;
	auto result = CWD(ec);
	ENGINE_EXCEPT_IF(ec, L"Current Path failed with " + wStr(ec));
	return result;
}

bool longmarch::FileSystem::ExistCheck(const fs::path& path, bool throwOnNotExist)
{
	if (fs::exists(path))
	{
		return true;
	}
	else
	{
		if (throwOnNotExist)
		{
			ENGINE_EXCEPT(L"Path does not exist : " + path.wstring());
		}
		return false;
	}
}

fs::path longmarch::FileSystem::CWD(std::error_code& ec)
{
	ec.clear();
#if defined(WIN32) || defined(WINDOWS_APP)
	DWORD pathlen = ::GetCurrentDirectoryW(0, 0);
	std::unique_ptr<wchar_t[]> buffer(new wchar_t[pathlen + 1]);
	if (::GetCurrentDirectoryW(pathlen, buffer.get()) == 0) {
		ec = std::error_code(::GetLastError(), std::system_category());
		return fs::path();
	}
	return fs::path(std::wstring(buffer.get()), fs::path::native_format);
#else
	size_t pathlen = static_cast<size_t>(std::max(int(::pathconf(".", _PC_PATH_MAX)), int(PATH_MAX)));
	std::unique_ptr<char[]> buffer(new char[pathlen + 1]);
	if (::getcwd(buffer.get(), pathlen) == NULL) {
		ec = std::error_code(errno, std::system_category());
		return fs::path();
	}
	return fs::path(buffer.get());
#endif
}

std::ifstream& longmarch::FileSystem::OpenIfstream(const fs::path& _file, FileType type)
{
	auto file = FileSystem::ResolveProtocol(_file).string();
	atomic_flag_guard lock(s_IfFlag);
	if (auto it = s_FileIStreamMap.find(file); it != s_FileIStreamMap.end())
	{
		// Rewind
		it->second.clear();
		it->second.seekg(0);
		return it->second;
	}
	else
	{
		switch (type)
		{
		case longmarch::FileSystem::FileType::OPEN_BINARY:
			s_FileIStreamMap[file] = std::ifstream(file, std::ifstream::in | std::ifstream::binary);
			break;
		case longmarch::FileSystem::FileType::OPEN_TEXT:
			s_FileIStreamMap[file] = std::ifstream(file, std::ifstream::in);
			break;
		}
		return s_FileIStreamMap[file];
	}
}

void longmarch::FileSystem::CloseIfstream(const fs::path& _file)
{
	auto file = FileSystem::ResolveProtocol(_file).string();
	atomic_flag_guard lock(s_IfFlag);
	if (auto it = s_FileIStreamMap.find(file); it != s_FileIStreamMap.end())
	{
		it->second.close();
		s_FileIStreamMap.erase(it);
	}
}

std::ofstream& longmarch::FileSystem::OpenOfstream(const fs::path& _file, FileType type)
{
	auto file = FileSystem::ResolveProtocol(_file).string();
	atomic_flag_guard lock(s_OfFlag);
	if (auto it = s_FileOStreamMap.find(file); it != s_FileOStreamMap.end())
	{
		// Rewind
		it->second.clear();
		it->second.seekp(0);
		return it->second;
	}
	else
	{
		switch (type)
		{
		case longmarch::FileSystem::FileType::OPEN_BINARY:
			s_FileOStreamMap[file] = std::ofstream(file, std::ofstream::out | std::ofstream::binary);
			break;
		case longmarch::FileSystem::FileType::OPEN_TEXT:
			s_FileOStreamMap[file] = std::ofstream(file, std::ofstream::out);
			break;
		}
		return s_FileOStreamMap[file];
	}
}

void longmarch::FileSystem::CloseOfstream(const fs::path& _file)
{
	auto file = FileSystem::ResolveProtocol(_file).string();
	atomic_flag_guard lock(s_OfFlag);
	if (auto it = s_FileOStreamMap.find(file); it != s_FileOStreamMap.end())
	{
		it->second.close();
		s_FileOStreamMap.erase(it);
	}
}

Json::Value longmarch::FileSystem::GetNewJsonCPP(const fs::path& _file)
{
	auto file = FileSystem::ResolveProtocol(_file).string();
	Json::Value doc;
	auto& stream = OpenIfstream(file, longmarch::FileSystem::FileType::OPEN_BINARY);
	ENGINE_TRY_CATCH({ stream >> doc; });
	CloseIfstream(file);
	return doc;
}

Json::Value& longmarch::FileSystem::GetCachedJsonCPP(const fs::path& _file)
{
	auto file = FileSystem::ResolveProtocol(_file).string();
	atomic_flag_guard lock(s_jsonFlag);
	if (auto it = s_jsonCPPParserMap.find(file); it == s_jsonCPPParserMap.end())
	{
		Json::Value doc;
		auto& stream = OpenIfstream(file, longmarch::FileSystem::FileType::OPEN_BINARY);
		ENGINE_TRY_CATCH({ stream >> doc; });
		CloseIfstream(file);
		s_jsonCPPParserMap[file] = std::move(doc);
	}
	return s_jsonCPPParserMap[file];
}

void longmarch::FileSystem::RemoveCachedJsonCPP(const fs::path& _file)
{
	auto file = FileSystem::ResolveProtocol(_file).string();
	atomic_flag_guard lock(s_jsonFlag);
	s_jsonCPPParserMap.erase(file);
}