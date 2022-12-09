#include <brutus/source_location_t.h>
#include <brutus/string.h>

#include <stdexcept>


bool source_location_t::equals(const source_location_t &other) const{
	return
		file == other.file &&
		row == other.row &&
		column == other.column &&
		(offset == -1 || other.offset == -1 || offset == other.offset) &&
	true;
}


source_location_t convert(const CXSourceLocation &location, const location_type_e &type){
	CXFile file;
	unsigned int row;
	unsigned int column;
	unsigned int offset;

	if(type == location_type_e::FILE_LOCATION){
		clang_getFileLocation(location, &file, &row, &column, &offset);
	}else if(type == location_type_e::SPELLING_LOCATION){
		clang_getSpellingLocation(location, &file, &row, &column, &offset);
	}else if(type == location_type_e::EXPANSION_LOCATION){
		clang_getExpansionLocation(location, &file, &row, &column, &offset);
	}else{
		throw std::logic_error("Unknown location type");
	}

	source_location_t result;
	result.file = convert(clang_getFileName(file));
	result.row = row;
	result.column = column;
	result.offset = offset;
	return result;
}

