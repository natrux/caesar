#include <brutus/CompilationDatabase.h>
#include <brutus/string.h>
#include <brutus/error.h>

#include <clang-c/CXCompilationDatabase.h>
#include <clang-c/Index.h>

#include <string>
#include <stdexcept>


CompilationDatabase::CompilationDatabase(){
	// We want clang_version == "X.Y.Z". The docs says clang_getClangVersion() may change its format in the future, so keep an eye on that.
	clang_version = convert(clang_getClangVersion());
	const size_t pos = clang_version.rfind(' ');
	if(pos != std::string::npos){
		clang_version = clang_version.substr(pos+1);
	}
}


void CompilationDatabase::read_from_build_directory(const std::string &path){
	CXCompilationDatabase_Error error;
	CXCompilationDatabase database = clang_CompilationDatabase_fromDirectory(path.c_str(), &error);
	if(error != CXCompilationDatabase_NoError){
		throw std::runtime_error("Error loading database: " + cx_error_string(error));
	}

	clear();
	CXCompileCommands all_compile_commands = clang_CompilationDatabase_getAllCompileCommands(database);
	for(size_t i=0; i<clang_CompileCommands_getSize(all_compile_commands); i++){
		CXCompileCommand command = clang_CompileCommands_getCommand(all_compile_commands, i);
		auto &entry = (*this)[convert(clang_CompileCommand_getFilename(command))];
		entry.emplace_back();
		auto &args = entry.back();
		for(size_t k=0; k<clang_CompileCommand_getNumArgs(command); k++){
			if(inject_include_directories && k == 1){
				// Now that's what I call a hack...
				args.push_back("-I/usr/lib/clang/" + clang_version + "/include");
			}
			args.push_back(convert(clang_CompileCommand_getArg(command, k)));
		}
	}

	clang_CompileCommands_dispose(all_compile_commands);
	clang_CompilationDatabase_dispose(database);
}


CompilationDatabase CompilationDatabase::diffplus(const CompilationDatabase &other) const{
	CompilationDatabase result;
	for(const auto &entry : other){
		const auto &file = entry.first;
		const auto find_existing_entries = find(file);
		for(const auto &args : entry.second){
			bool already_exists = false;
			if(find_existing_entries != end()){
				const auto &existing_entries = find_existing_entries->second;
				for(const auto &existing_args : existing_entries){
					bool identical = true;
					const size_t size = args.size();
					if(size != existing_args.size()){
						identical = false;
					}else{
						for(size_t i=0; i<size; i++){
							if(args[i] != existing_args[i]){
								identical = false;
								break;
							}
						}
					}
					if(identical){
						already_exists = true;
						break;
					}
				}
			}
			if(!already_exists){
				result[file].push_back(args);
			}
		}
	}
	return result;
}



