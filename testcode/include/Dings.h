#pragma once


class Dings;


class Dings{
public:
	Dings(const char *name_);

	// We should do something
	void tu_wat(int number, int other_number=666);

	/**
	 * We should do even more!
	 *
	 * Like, start a revolution or something.
	 */
	void tu_wat(double number, double other_number=777);

private:
	const char *name;
};


namespace lol{

Dings make_dings(const char *name);
Dings make_dings(const char *name);

}

