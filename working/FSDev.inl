#include <fstream>

#if !defined(IN_FS_H_FILE)

#if defined(CONFIG_WINDOWS)
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#ifdef __cpp_lib_filesystem
#include <filesystem>
#define FILESYSTEM std::filesystem
#elif __cpp_lib_experimental_filesystem
#include <experimental/filesystem>
#define FILESYSTEM std::experimental::filesystem
#else
#error "Must specify a filesystem."
#endif

#elif defined(CONFIG_LINUX)
#include <experimental/filesystem>
#define FILESYSTEM std::experimental::filesystem
#else
#error "Must specify a filesystem for this arcetecture."
#endif

namespace
{
	HSTL::IDTable<HSTL::String> files;

	void ParseFolder(HSTL::String &path)
	{
		HSTL::Bool hasArr = false;
#if defined(CONFIG_WINDOWS)
		for(auto &file : FILESYSTEM::directory_iterator(reinterpret_cast<const wchar_t*>(path.C_Str16())))
#else
		for(auto &file : FILESYSTEM::directory_iterator(path.C_Str()))
#endif
		{
#if defined(CONFIG_WINDOWS)
			HSTL::String filename(reinterpret_cast<const HSTL::Char16*>(file.path().c_str()), true);
#else
			HSTL::String filename(file.path().c_str(), HSTL::StringConstructType::Copy);
#endif

			if(FILESYSTEM::is_directory(file.path()))
			{
				if(filename.EndsWith(".") || filename.EndsWith(".."))
					continue;
				ParseFolder(filename);
			}
			else
			{
			HSTL::WholeNumber dotPos = filename.ReverseFind('.');
   				if(dotPos != -1)
				{
					HSTL::WholeNumber slashPos = filename.ReverseFind(SLASH_FS);
					HSTL::WholeNumber start = ((slashPos != -1) ? slashPos + 1 : 0);
					HSTL::ID hash = filename.HashSubString(start, dotPos - start);

					if(filename.EndsWith(".arr", 4))
					{
						files.Insert(hash, std::move(filename));
						hasArr = true;
					}
				}
			}
		}

		if(hasArr)
			return;

#if defined(CONFIG_WINDOWS)
		for(auto &file : FILESYSTEM::directory_iterator(reinterpret_cast<const wchar_t*>(path.C_Str16())))
#else
		for(auto &file : FILESYSTEM::directory_iterator(path.C_Str()))
#endif
		{
			if(!FILESYSTEM::is_regular_file(file.path()))
				continue;

#if defined(CONFIG_WINDOWS)
			HSTL::String filename(reinterpret_cast<const HSTL::Char16*>(file.path().c_str()), true);
#else
			HSTL::String filename(file.path().c_str(), HSTL::StringConstructType::Copy);
#endif
			HSTL::WholeNumber dotPos = filename.ReverseFind('.');
   			if(dotPos != -1)
			{
				HSTL::WholeNumber slashPos = filename.ReverseFind(SLASH_FS);
				HSTL::WholeNumber start = ((slashPos != -1) ? slashPos + 1 : 0);
				HSTL::ID hash = filename.HashSubString(start, dotPos - start);
				files.Insert(hash, std::move(filename));
			}
		}
	}
}

void BindLoadingManager(void);

void InitFS(void)
{
#if defined(CONFIG_WINDOWS)
	HSTL::Char16 dirstr[] = {'f', 's', 'd', 0};
	HSTL::String dir(dirstr, false);
#else
	HSTL::String dir("fsd", HSTL::StringConstructType::Reference, 3);
#endif
	ParseFolder(dir);
	BindLoadingManager();
}

HSTL::IDTable<HSTL::String> &GetFSFilesArray(void)
{
	return files;
}

#else

HSTL::IDTable<HSTL::String> &GetFSFilesArray(void);

template <typename T>
void FileFetcher<T>::Iterate(void)
{
	if(!done)
	{
		softcheck(callback == nullptr, "File: callback is null", errorReported = true; done = true; return; );
		auto f = GetFSFilesArray().Get(fileID);
		softcheck(f == nullptr, "File: could not be found", errorReported = true; done = true; return; );
		if(f->EndsWith(".arr"))
		{
#if defined(CONFIG_WINDOWS)
			std::ifstream master(reinterpret_cast<const wchar_t*>(f->C_Str16()));
#else
			std::ifstream master(f->C_Str());
#endif
			softcheck(!master.is_open(), "File: master could not be opened", errorReported = true; done = true; return; );

			HSTL::String prePath;
			size_t slashPos = f->ReverseFind(SLASH_FS);
			if(slashPos != std::string::npos)
				prePath = std::move(f->SubString(0, slashPos+1));

			while(!master.eof())
			{
				char readLine[1025];
				for(int i=0 ; i<1025 ; ++i)
					readLine[i] = 0;
				HSTL::WholeNumber read = 0;
				while(read < 1024)
				{
					int c = master.get();
					if(c == EOF || c == 0x0A || c == 0x0D || c < 0)
						break;
					readLine[read] = char(c);
					++read;
				}
				HSTL::String fileName(readLine, HSTL::StringConstructType::Reference, read);

				if(0 < fileName.Length())
				{
					FileBuffer fb;
					if(fileName.Length() < MAX_FILENAMELEN)
						HSTL::Internal::Strcpy(fileName.C_Str(), fb.name);
					HSTL::WholeNumber dotPos = fileName.ReverseFind('.');
					fb.type = fileName.HashSubString(dotPos, fileName.Length() - dotPos);

					HSTL::StringArray fullPath;
					fullPath.Append(prePath.MakeReference());
					fullPath.Append(std::move(fileName));
#if defined(CONFIG_WINDOWS)
					std::ifstream file(reinterpret_cast<const wchar_t*>(fullPath.Combine().C_Str16()), std::ifstream::binary | std::ifstream::ate);
#else
					std::ifstream file(fullPath.Combine().C_Str(), std::ifstream::binary | std::ifstream::ate);
#endif
					softcheck(!file.is_open(), "File: could not be opened", callback(fb, userData); errorReported = true; continue; );
					HSTL::WholeNumber size = file.tellg();
					file.seekg(0);
					unsigned char *buffer = reinterpret_cast<unsigned char*>(HSTL::Internal::GetMem(size + 4));
					softcheck(!buffer, "File: buffer not allocated", callback(fb, userData); errorReported = true; continue; );
					HSTL::WholeNumber readLen = 0;
					while(readLen < size)
					{
						file.read(reinterpret_cast<char*>(buffer + readLen), size - readLen);
						readLen += file.gcount();
						softcheck(file.bad(), "File: buffer was not filled", HSTL::Internal::FreeMem(buffer); buffer = nullptr; errorReported = true; break; );
					}

					if(buffer != nullptr)
					{
						buffer[size] = buffer[size+1] = buffer[size+2] = buffer[size+3] = 0;

						fb.buffer = buffer;
						fb.size = size;
						HSTL::Bool ret = callback(fb, userData);

						if(fb.buffer != nullptr)
							HSTL::Internal::FreeMem(fb.buffer);
						softcheck(!ret, "File: callback failed", errorReported = true; );
					}
					else
						callback(fb, userData);
				}
			}
			done = true;
		}
		else
		{
#if defined(CONFIG_WINDOWS)
			std::ifstream file(reinterpret_cast<const wchar_t*>(f->C_Str16()), std::ifstream::binary | std::ifstream::ate);
#else
			std::ifstream file(f->C_Str(), std::ifstream::binary | std::ifstream::ate);
#endif
			softcheck(!file.is_open(), "File: could not be opened", errorReported = true; done = true; return; );
			HSTL::WholeNumber size = file.tellg();
			file.seekg(0);
			unsigned char *buffer = reinterpret_cast<unsigned char*>(HSTL::Internal::GetMem(size + 4));
			softcheck(!buffer, "File: buffer not allocated", errorReported = true; done = true; return; );
			HSTL::WholeNumber readLen = 0;
			while(readLen < size)
			{
				file.read(reinterpret_cast<char*>(buffer + readLen), size - readLen);
				readLen += file.gcount();
				softcheck(file.bad(), "File: buffer was not filled", HSTL::Internal::FreeMem(buffer); errorReported = true; done = true; return; );
			}

			buffer[size] = buffer[size+1] = buffer[size+2] = buffer[size+3] = 0;

			FileBuffer fb;
			fb.buffer = buffer;
			fb.size = size;
			HSTL::WholeNumber dotPos = f->ReverseFind('.');
			fb.type = f->HashSubString(dotPos, f->Length() - dotPos);
			if(f->Length() < MAX_FILENAMELEN)
				HSTL::Internal::Strcpy(f->C_Str(), fb.name);
			HSTL::Bool ret = callback(fb, userData);

			if(fb.buffer != nullptr)
				HSTL::Internal::FreeMem(fb.buffer);
			softcheck(!ret, "File: callback failed", errorReported = true; );
			done = true;
		}
	}
}

#endif
