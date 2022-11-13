#include <util/ls.h>

#include <string>
#include <windows.h>


std::string GetLastErrorString(){
	const DWORD error = GetLastError();
	if(error == 0) {
		return "No error";
	}

	LPSTR buffer = NULL;
	const size_t size = FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&buffer,
		0,
		NULL
	);
	if(size == 0){
		const DWORD another_errror = GetLastError();
		return "Code " + std::to_string(error) + " (additionally, FormatMessage() failed with Code " + std::to_string(another_error) + ")";
	}

	const std::string message(buffer, size);
	LocalFree(buffer);

	return message;
}


void ls(const std::string &full_path, std::vector<std::string> &files, std::vector<std::string> &directories){
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind;

	hFind = FindFirstFile((full_path + "/*").c_str(), &FindFileData);
	if (hFind == INVALID_HANDLE_VALUE){
		throw std::runtime_error("FindFirstFile() failed with: " + GetLastErrorString());
	}

	do{
		std::string name = FindFileData.cFileName;
		if(name == "." || name == ".."){
			continue;
		}

		if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0){
			directories.push_back(name);
		}else /*if((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_NORMAL) != 0)*/{
			files.push_back(name);
		}
	}while(FindNextFile(hFind, &FindFileData) != 0);

	FindClose(hFind);
}



