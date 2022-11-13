#include <brutus/completion_t.h>
#include <brutus/string.h>
#include <util/ordering_e.h>

#include <algorithm>
#include <stdexcept>



std::string completion_t::get_typed_text() const{
	for(const auto &chunk : chunks){
		if(chunk.type == completion_chunk_type_e::TYPED_TEXT){
			return chunk.text;
		}
	}
	throw std::runtime_error("completion does not have a TYPED_TEXT chunk");
}


std::vector<availability_e> completion_t::rank_availability = {
	availability_e::AVAILABLE,
	availability_e::DEPRECATED,
	availability_e::NOT_ACCESSIBLE,
	availability_e::NOT_AVAILABLE,
};


std::vector<CXCursorKind> completion_t::rank_cursor = {
	CXCursor_FieldDecl,
	CXCursor_CXXMethod,
	CXCursor_OverloadCandidate,
	CXCursor_Constructor,
	CXCursor_ConversionFunction,
	CXCursor_Destructor,
	CXCursor_VarDecl,
	CXCursor_FunctionDecl,
	CXCursor_FunctionTemplate,
	CXCursor_ClassDecl,
};


bool completion_t::compare(const completion_t &a, const completion_t &b){
	{
		// let libclang sort it
		if(a.priority < b.priority){
			return true;
		}else if(a.priority > b.priority){
			return false;
		}
	}

	{
		// sort by availability
		const ordering_e ord = list_rank(rank_availability, a.availability, b.availability);
		if(ord == ordering_e::LESS){
			return true;
		}else if(ord == ordering_e::GREATER){
			return false;
		}
	}

	{
		// sort by number of fixits
		const size_t a_siz = a.fixits.size();
		const size_t b_siz = b.fixits.size();
		if(a_siz < b_siz){
			return true;
		}else if(a_siz > b_siz){
			return false;
		}
	}

	{
		// sort by cursor kind
		const ordering_e ord = list_rank(rank_cursor, a.kind, b.kind);
		if(ord == ordering_e::LESS){
			return true;
		}else if(ord == ordering_e::GREATER){
			return false;
		}
	}

	{
		// sort by name
		auto iter_a = a.chunks.begin();
		auto iter_b = b.chunks.begin();
		while(iter_a != a.chunks.end() && iter_a->type != completion_chunk_type_e::TYPED_TEXT){
			iter_a++;
		}
		while(iter_b != b.chunks.end() && iter_b->type != completion_chunk_type_e::TYPED_TEXT){
			iter_b++;
		}
		if(iter_a != a.chunks.end() && iter_b != b.chunks.end()){
			const int cmp = iter_a->text.compare(iter_b->text);
			if(cmp < 0){
				return true;
			}else if(cmp > 0){
				return false;
			}
		}
	}

	// whatever
	return false;
}


completion_t convert(const CXCompletionResult &completion_result){
	completion_t result;
	result.kind = completion_result.CursorKind;
	result.priority = clang_getCompletionPriority(completion_result.CompletionString);
	result.availability = convert(clang_getCompletionAvailability(completion_result.CompletionString));
	result.brief_comment = convert(clang_getCompletionBriefComment(completion_result.CompletionString));
	size_t num_annotations = clang_getCompletionNumAnnotations(completion_result.CompletionString);
	for(size_t i=0; i<num_annotations; i++){
		result.annotations.push_back(convert(clang_getCompletionAnnotation(completion_result.CompletionString, i)));
	}
	chunk_completion(result.chunks, completion_result.CompletionString);
	return result;
}


std::vector<completion_t> convert(CXCodeCompleteResults *&&complete_results){
	if(complete_results == NULL){
		throw std::runtime_error("Failed to get code completion results");
	}
	std::vector<completion_t> result;

	for(size_t i=0; i<complete_results->NumResults; i++){
		completion_t completion = convert(complete_results->Results[i]);

		size_t num_fixits = clang_getCompletionNumFixIts(complete_results, i);
		for(size_t k=0; k<num_fixits; k++){
			CXSourceRange range;
			std::string replace = convert(clang_getCompletionFixIt(complete_results, i, k, &range));
			completion.fixits.push_back(make_fixit(range, replace));
		}
		result.push_back(std::move(completion));
	}
	clang_disposeCodeCompleteResults(complete_results);

	std::sort(result.begin(), result.end(), completion_t::compare);
	return result;
}


