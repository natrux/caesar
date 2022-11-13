#include <brutus/availability_e.h>

#include <string>
#include <stdexcept>


availability_e convert(const CXAvailabilityKind &kind){
	switch(kind){
	case CXAvailability_Available:
		return availability_e::AVAILABLE;
	case CXAvailability_Deprecated:
		return availability_e::DEPRECATED;
	case CXAvailability_NotAvailable:
		return availability_e::NOT_AVAILABLE;
	case CXAvailability_NotAccessible:
		return availability_e::NOT_ACCESSIBLE;
	default:
		throw std::logic_error("Unknown availability kind: " + std::to_string(kind));
	}
}


