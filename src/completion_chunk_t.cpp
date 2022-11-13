#include <brutus/completion_chunk_t.h>
#include <brutus/string.h>

#include <stdexcept>


void chunk_completion(std::vector<completion_chunk_t> &result, const CXCompletionString &completion_string){
	for(size_t k=0; k<clang_getNumCompletionChunks(completion_string); k++){
		CXCompletionChunkKind kind = clang_getCompletionChunkKind(completion_string, k);

		if(kind == CXCompletionChunk_Optional){
			chunk_completion(result, clang_getCompletionChunkCompletionString(completion_string, k));
		}else{
			completion_chunk_type_e chunk_type = completion_chunk_type_e::TEXT;
			switch(kind){
			case CXCompletionChunk_Optional:
				// already handled
				break;
			case CXCompletionChunk_ResultType:
				chunk_type = completion_chunk_type_e::RETURN_TYPE;
				break;
			case CXCompletionChunk_TypedText:
				chunk_type = completion_chunk_type_e::TYPED_TEXT;
				break;
			case CXCompletionChunk_LeftParen:
				chunk_type = completion_chunk_type_e::LEFT_PARENTHESIS;
				break;
			case CXCompletionChunk_RightParen:
				chunk_type = completion_chunk_type_e::RIGHT_PARENTHESIS;
				break;
			case CXCompletionChunk_LeftBracket:
				chunk_type = completion_chunk_type_e::LEFT_BRACKET;
				break;
			case CXCompletionChunk_RightBracket:
				chunk_type = completion_chunk_type_e::RIGHT_BRACKET;
				break;
			case CXCompletionChunk_LeftBrace:
				chunk_type = completion_chunk_type_e::LEFT_BRACE;
				break;
			case CXCompletionChunk_RightBrace:
				chunk_type = completion_chunk_type_e::RIGHT_BRACE;
				break;
			case CXCompletionChunk_LeftAngle:
				chunk_type = completion_chunk_type_e::LEFT_ANGLE;
				break;
			case CXCompletionChunk_RightAngle:
				chunk_type = completion_chunk_type_e::RIGHT_ANGLE;
				break;
			case CXCompletionChunk_Colon:
				chunk_type = completion_chunk_type_e::COLON;
				break;
			case CXCompletionChunk_Equal:
				chunk_type = completion_chunk_type_e::EQUAL;
				break;
			case CXCompletionChunk_HorizontalSpace:
				chunk_type = completion_chunk_type_e::HORIZONTAL_SPACE;
				break;
			case CXCompletionChunk_VerticalSpace:
				chunk_type = completion_chunk_type_e::VERTICAL_SPACE;
				break;
			case CXCompletionChunk_Comma:
				chunk_type = completion_chunk_type_e::COMMA;
				break;
			case CXCompletionChunk_SemiColon:
				chunk_type = completion_chunk_type_e::SEMICOLON;
				break;
			case CXCompletionChunk_Text:
				chunk_type = completion_chunk_type_e::TEXT;
				break;
			case CXCompletionChunk_Placeholder:
				chunk_type = completion_chunk_type_e::PLACEHOLDER;
				break;
			case CXCompletionChunk_Informative:
				chunk_type = completion_chunk_type_e::INFO;
				break;
			case CXCompletionChunk_CurrentParameter:
				chunk_type = completion_chunk_type_e::CURRENT_PARAMETER;
				break;
			default:
				throw std::logic_error("Unknown completion chunk kind");
			}

			completion_chunk_t chunk;
			chunk.type = chunk_type;
			chunk.text = convert(clang_getCompletionChunkText(completion_string, k));
			result.push_back(std::move(chunk));
		}
	}
}

