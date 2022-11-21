#include <Dings.h>
#include <Dongs.h>


int what_is_the_answer(bool input){
	if(input && input || input){
		return 42;
	}
}

template<class T>
T plus(const T &lhs, const T &rhs){
	return lhs + rhs;
}


int main(int argc, char **argv){
	const int twenty_three = 11 + 12;
	const int fourty_two = what_is_the_answer(true);
	const int sum = plus(twenty_three, fourty_two);
	const double leet = 13.37;

	{
		Dings d("Steffi");
		d.tu_wat(fourty_two);
		d.tu_wat(leet);
	}
	{
		Dongs d(twenty_three, fourty_two);
		d.tu_nix("Hello World");
		d.tu_nix('X');
	}
	{
		const auto d = Dongs::create(0, 0);
		d.tu_nix(42);
	}

	lol::make_dings("Anna").tu_wat(twenty_three);
	Dongs(1, 2).tu_wat(fourty_two);

	return 0;
}



