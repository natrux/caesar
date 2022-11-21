#include <brutus/string.h>
#include <brutus/completion_t.h>
#include <brutus/cursor_t.h>

#include <pompeius/TranslationUnit.h>
#include <pompeius/Project.h>

#include <iostream>



void print_cursor(const cursor_t &cursor, bool print_ref){
	std::cout
		<< (cursor.is_declaration ? "[DECL] " : "")
		<< (cursor.is_definition ? "[DEF] " : "")
		<< "(" << cursor.hash << ") "
		<< "\"" << cursor.usr << "\" "
		<< cursor.kind_spelling() << " "
		<< "of type <" << cursor.type_spelling << ">"
		<< " in " << cursor.spelling_range.begin.file << " [" << cursor.spelling_range.begin.offset << ":" << cursor.spelling_range.end.offset << "]" << "(L" << cursor.spelling_range.begin.row << "C" << cursor.spelling_range.begin.column << ") - (L" << cursor.spelling_range.end.row << "C" << cursor.spelling_range.end.column << ")"
		<< std::endl;
	if(print_ref && cursor.reference.exists){
		std::cout << "--> ";
		print_cursor(*cursor.reference.get(), print_ref);
	}
}


void print_completion(const completion_t &completion){
	switch(completion.availability){
	case availability_e::AVAILABLE:
		std::cout << "[ ]";
		break;
	case availability_e::DEPRECATED:
		std::cout << "[~]";
		break;
	case availability_e::NOT_AVAILABLE:
		std::cout << "[X]";
		break;
	case availability_e::NOT_ACCESSIBLE:
		std::cout << "[P]";
		break;
	}
	std::cout << "[" << convert(clang_getCursorKindSpelling(completion.kind)) << "]";
	std::cout << "\t";

	for(const auto &fixit : completion.fixits){
		std::cout << "[(" << fixit.range.begin.row << ", " << fixit.range.begin.column << ") '" << fixit.replace << "' (" << fixit.range.end.row << ", " << fixit.range.end.column << ")] ";
	}

	for(const auto &chunk : completion.chunks){
		std::cout << chunk.text;
		if(chunk.type == completion_chunk_type_e::RETURN_TYPE){
			std::cout << " ";
		}
	}
	std::cout << "\t";
	std::cout << completion.brief_comment;
	std::cout << "\t";
	for(const auto &an : completion.annotations){
		std::cout << an << " ";
	}

	std::cout << std::endl;
}



void print_translation_unit(std::shared_ptr<const TranslationUnit> unit){
	std::cout << unit << "{" << std::endl;
	std::cout << "main file: " << unit->get_main_file() << std::endl;
	std::cout << "included files: [";
	for(const auto &inc : unit->get_included_files()){
		std::cout << inc << ", ";
	}
	std::cout << "]" << std::endl;
	std::cout << "compile commands: [";
	for(const auto &cmd : unit->get_compile_commands()){
		std::cout << cmd << ", ";
	}
	std::cout << "]" << std::endl;
	std::cout << "diagnostics: [" << std::endl;
	for(const auto &diag : unit->get_diagnostics()){
		std::cout << "{" << std::endl;
		std::cout << "severity: ";
		switch(diag.severity){
		case diagnostic_severity_e::IGNORED:
			std::cout << "ignored";
			break;
		case diagnostic_severity_e::NOTE:
			std::cout << "note";
			break;
		case diagnostic_severity_e::WARNING:
			std::cout << "warning";
			break;
		case diagnostic_severity_e::ERROR:
			std::cout << "error";
			break;
		case diagnostic_severity_e::FATAL:
			std::cout << "fatal";
			break;
		}
		std::cout << std::endl;
		std::cout << "category: " << diag.category_name << std::endl;
		std::cout << "message: " << diag.message << std::endl;
		std::cout << "location: " << diag.location.file << "+" << diag.location.offset << " (L" << diag.location.row << "C" << diag.location.column << ")" << std::endl;
		std::cout << "Ranges: [" << std::endl;
		for(const auto &range : diag.ranges){
			std::cout << "from " << range.begin.file << "+" << range.begin.offset << " (L" << range.begin.row << "C" << range.begin.column << ") to " << range.end.file << "+" << range.end.offset << " (L" << range.end.row << "C" << range.end.column << ")" << std::endl;
		}
		std::cout << "]" << std::endl;
		std::cout << diag.fixits.size() << " fixit" << (diag.fixits.size() == 1 ? "" : "s") << " available" << std::endl;

		std::cout << "}," << std::endl;
	}
	std::cout << "]" << std::endl;
	std::cout << "}," << std::endl;
}


void print_ast_references(const ASTReferences &refs){
	std::cout << "declarations: [" << std::endl;
	for(const auto &cursor : refs.get_declarations()){
		print_cursor(cursor, false);
	}
	std::cout << "]" << std::endl;

	std::cout << "definitions: [" << std::endl;
	for(const auto &cursor : refs.get_definitions()){
		print_cursor(cursor, false);
	}
	std::cout << "]" << std::endl;

	std::cout << "uses: [" << std::endl;
	for(const auto &cursor : refs.get_uses()){
		print_cursor(cursor, false);
	}
	std::cout << "]" << std::endl;
}


CXChildVisitResult visit_print(CXCursor cursor, CXCursor /*parent*/, CXClientData /*client_data*/){
	print_cursor(convert(cursor), true);
	return CXChildVisit_Recurse;
}



int test(){
	const std::string path_directory = "/?/?/?/";
	const std::string path_build_directory = path_directory + "/build";
	const std::string test_filename = path_directory + "/src/main.cpp";
	//const bool print_compilation_database = true;
	//const bool print_tu_map = true;
	//const bool print_reference_map = false;
	const bool print_included_files = true;
	const bool print_ast = true;
	const bool full_ast = false;
	const bool do_completion = false;
	const bool inspect_location = false;
	const bool do_anything = print_included_files || print_ast || do_completion || inspect_location;

	Project project(path_build_directory);


	std::shared_ptr<TranslationUnit> test_unit;
	if(do_anything){
		auto units = project.get_translation_units(test_filename);
		const size_t num_units = units.size();
		if(num_units == 0){
			std::cout << "No translation units for file " << test_filename << std::endl;
		}else{
			test_unit = *(units.begin());
			if(num_units > 1){
				std::cout << "Found " << num_units << " translation units for file " << test_filename << ", using the one with main file " << test_unit->get_main_file() << std::endl;
			}
			test_unit->parse();
		}
	}

	if(print_included_files && test_unit){
		std::cout << "---------- Included Files ----------" << std::endl;
		const auto included_files = test_unit->get_included_files();
		for(const std::string &file : included_files){
			std::cout << file << std::endl;
		}
	}

	if(print_ast && test_unit){
		std::cout << "---------- AST ----------" << std::endl;
		std::vector<cursor_t> cursors;
		if(full_ast){
			cursors = test_unit->get_all_cursors();
		}else{
			cursors = test_unit->get_cursors(test_unit->get_main_file());
		}
		for(const auto &cursor : cursors){
			print_cursor(cursor, true);
		}
	}

	if(do_completion && test_unit){
		std::cout << "---------- Completion results ----------" << std::endl;
		std::vector<completion_t> completions = test_unit->code_complete(test_filename, 18, 5);
		for(const auto &completion : completions){
			print_completion(completion);
		}
	}

	if(inspect_location && test_unit){
		std::cout << "---------- Inspecting location ----------" << std::endl;
		source_location_t location{
			test_filename,
			12,
			5,
			0,
		};
		cursor_t cursor = test_unit->get_location(location);
		print_cursor(cursor, true);
	}

	return 0;
}


int main(int /*argc*/, char ** /*argv*/){
	return test();
}
