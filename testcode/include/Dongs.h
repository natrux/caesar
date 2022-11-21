#pragma once

#include <Dings.h>


class Dongs : public Dings{
public:
	static Dongs create(int a, int b);

	Dongs(int a, int b);
	void tu_nix(const char *str) const;
	void tu_nix(char chr) const;

private:
	int num_a;
	int num_b;
};



