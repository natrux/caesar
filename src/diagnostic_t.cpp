#include <brutus/diagnostic_t.h>
#include <brutus/string.h>


// have to hide this function because CXDiagnostic and CXDiagnosticSet are both typedef'ed void*
static diagnostic_t convert_single_diagnostic(CXDiagnostic &&diagnostic){
	diagnostic_t result;
	result.severity = convert(clang_getDiagnosticSeverity(diagnostic));
	result.location = convert(clang_getDiagnosticLocation(diagnostic));
	result.message = convert(clang_getDiagnosticSpelling(diagnostic));
	result.category_name = convert(clang_getDiagnosticCategoryText(diagnostic));
	result.command_line_option = convert(clang_getDiagnosticOption(diagnostic, NULL));
	for(size_t i=0; i<clang_getDiagnosticNumRanges(diagnostic); i++){
		result.ranges.push_back(convert(clang_getDiagnosticRange(diagnostic, i)));
	}
	for(size_t i=0; i<clang_getDiagnosticNumFixIts(diagnostic); i++){
		CXSourceRange range;
		std::string replace = convert(clang_getDiagnosticFixIt(diagnostic, i, &range));
		result.fixits.push_back(make_fixit(range, replace));
	}

	// Do not dispose this set.
	const CXDiagnosticSet children_set = clang_getChildDiagnostics(diagnostic);
	result.children = convert(children_set);

	clang_disposeDiagnostic(diagnostic);
	return result;
}


std::vector<diagnostic_t> convert(CXDiagnosticSet &&diagnostic_set){
	const auto result = convert(diagnostic_set);
	clang_disposeDiagnosticSet(diagnostic_set);
	return result;
}


std::vector<diagnostic_t> convert(const CXDiagnosticSet &diagnostic_set){
	std::vector<diagnostic_t> result;
	for(size_t i=0; i<clang_getNumDiagnosticsInSet(diagnostic_set); i++){
		const diagnostic_t diagnostic = convert_single_diagnostic(clang_getDiagnosticInSet(diagnostic_set, i));
		result.push_back(diagnostic);
	}
	return result;
}

