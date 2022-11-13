#include <util/ls.h>

#include <stdexcept>
#include <cstring>
#include <dirent.h>
#include <sys/stat.h>


void ls(const std::string &full_path, std::vector<std::string> &files, std::vector<std::string> &directories){
	DIR *dir = opendir(full_path.c_str());
	if(dir == NULL){
		throw std::runtime_error("opendir(\"" + full_path + "\") failed with: " + std::string(strerror(errno)));
	}

	dirent *dp;
	while((dp = readdir(dir)) != NULL){
		const std::string name = dp->d_name;
		if(name == "." || name == ".."){
			continue;
		}

		bool is_file = false;
		bool is_directory = false;
#ifdef _DIRENT_HAVE_D_TYPE
		if(dp->d_type == DT_DIR){
			is_file = false;
			is_directory = true;
		}else if(dp->d_type == DT_REG){
			is_file = true;
			is_directory = false;
		}else if(dp->d_type == DT_UNKNOWN || dp->d_type == DT_LNK)
#endif
		{
			const std::string new_path = full_path + "/" + name;
			struct stat info;
			const auto ret = stat(new_path.c_str(), &info);
			if(ret == -1){
				closedir(dir);
				throw std::runtime_error("stat(\"" + new_path + "\") failed with: " + std::string(strerror(errno)));
			}
			is_file = S_ISREG(info.st_mode);
			is_directory = S_ISDIR(info.st_mode);
		}

		if(is_file){
			files.push_back(name);
		}
		if(is_directory){
			directories.push_back(name);
		}
	}
	closedir(dir);
}


