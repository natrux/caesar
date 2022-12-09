#include <brutus/source_range_t.h>

#include <stdexcept>


bool source_range_t::equals(const source_range_t &other) const{
	return begin.equals(other.begin) && end.equals(other.end);
}


bool source_range_t::contains(const source_location_t &location) const{
	if(location.file != begin.file || location.file != end.file){
		return false;
	}
	const bool contains_rowcol = (location.row >= begin.row && location.row <= end.row && (location.row > begin.row || location.column >= begin.column) && (location.row < end.row || location.column <= end.column));
	bool contains_offset = contains_rowcol;
	if(location.offset != -1 && begin.offset != -1 && end.offset != -1){
		contains_offset = (location.offset >= begin.offset && location.offset <= end.offset);
	}

	if(contains_rowcol != contains_offset){
		throw std::logic_error("row/column is inconsistent with offset");
	}
	return contains_rowcol && contains_offset;
}


bool source_range_t::contains(const source_range_t &range) const{
	return contains(range.begin) && contains(range.end);
}


bool source_range_t::overlaps(const source_range_t &range) const{
	return contains(range.begin) || range.contains(begin);
}


source_range_t convert(const CXSourceRange &range, const location_type_e &type){
	source_range_t result;
	result.begin = convert(clang_getRangeStart(range), type);
	result.end = convert(clang_getRangeEnd(range), type);
	return result;
}

