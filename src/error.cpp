#include <brutus/error.h>


std::string cx_error_string(const CXErrorCode &error){
	switch(error){
	case CXError_Success:
		return "Success";
	case CXError_Failure:
		return "Generic Error";
	case CXError_Crashed:
		return "libclang crashed while performing the requested operation";
	case CXError_InvalidArguments:
		return "The function detected that the arguments violate the function contract";
	case CXError_ASTReadError:
		return "An AST deserialization error has occurred";
	}
	return "Unknown error: " + std::to_string(error);
}


std::string cx_error_string(const CXCompilationDatabase_Error &error){
	switch(error){
	case CXCompilationDatabase_NoError:
		return "No error";
	case CXCompilationDatabase_CanNotLoadDatabase:
		return "Can not load database";
	}
	return "Unknown error: " + std::to_string(error);
}


