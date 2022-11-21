#include <Dings.h>


Dings::Dings(const char *name_):
	name(name_)
{
}


void Dings::tu_wat(int number, int other_number){
	// lol
}


void Dings::tu_wat(double number, double other_number){
	// lol
}


namespace lol{

Dings make_dings(const char *name){
	Dings ddd(name);
	return ddd;
}

}

