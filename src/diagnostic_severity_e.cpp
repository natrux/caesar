#include <brutus/diagnostic_severity_e.h>

#include <stdexcept>


diagnostic_severity_e convert(const CXDiagnosticSeverity &severity){
	switch(severity){
	case CXDiagnostic_Ignored:
		return diagnostic_severity_e::IGNORED;
	case CXDiagnostic_Note:
		return diagnostic_severity_e::NOTE;
	case CXDiagnostic_Warning:
		return diagnostic_severity_e::WARNING;
	case CXDiagnostic_Error:
		return diagnostic_severity_e::ERROR;
	case CXDiagnostic_Fatal:
		return diagnostic_severity_e::FATAL;
	default:
		throw std::logic_error("Unknown severity: " + std::to_string(severity));
	}
}


