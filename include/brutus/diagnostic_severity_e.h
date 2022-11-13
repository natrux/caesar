#pragma once

#include <clang-c/Index.h>


enum class diagnostic_severity_e{
	IGNORED,
	NOTE,
	WARNING,
	ERROR,
	FATAL,
};

diagnostic_severity_e convert(const CXDiagnosticSeverity &severity);

