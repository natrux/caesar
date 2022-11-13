#pragma once

#include <vector>


enum class ordering_e{
	LESS,
	EQUAL,
	GREATER,
};


template<class T>
ordering_e list_rank(const std::vector<T> &rank, const T &a, const T &b){
	if(a == b){
		return ordering_e::EQUAL;
	}

	for(const T &level : rank){
		const bool is_a = (a == level);
		const bool is_b = (b == level);
		if(is_a && is_b){
			return ordering_e::EQUAL;
		}else if(is_a){
			return ordering_e::LESS;
		}else if(is_b){
			return ordering_e::GREATER;
		}
	}
	return ordering_e::EQUAL;
}



