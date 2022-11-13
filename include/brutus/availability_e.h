#pragma once

#include <clang-c/Index.h>


enum class availability_e{
	AVAILABLE,
	DEPRECATED,
	NOT_ACCESSIBLE,
	NOT_AVAILABLE,
};


availability_e convert(const CXAvailabilityKind &kind);



