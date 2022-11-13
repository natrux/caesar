#pragma once

#include <stdexcept>


template<class T>
struct option_t{
	T option;
	bool is_set = false;

	explicit operator bool() const{
		return is_set;
	}

	T operator()() const{
		if(!is_set){
			throw std::logic_error("option not set");
		}
		return option;
	}

	option_t &operator=(const T &value){
		option = value;
		is_set = true;
		return *this;
	}

	option_t &operator=(T &&value){
		option = std::move(value);
		is_set = true;
		return *this;
	}

	option_t &operator=(const option_t &other){
		if(&other == this){
			return *this;
		}

		if(other){
			*this = other();
		}else{
			clear();
		}
		return *this;
	}

	option_t &operator=(option_t &&other){
		if(&other == this){
			return *this;
		}

		if(other){
			*this = std::move(other.option);
			other.clear();
		}else{
			clear();
		}
		return *this;
	}

	option_t &operator|=(const T &value){
		if(!*this){
			*this = value;
		}
		return *this;
	}

	option_t &operator|=(T &&value){
		if(!*this){
			*this = std::move(value);
		}
		return *this;
	}

	option_t &operator|=(const option_t &other){
		if(&other == this){
			return *this;
		}
		if(!*this){
			*this = other;
		}
		return *this;
	}

	option_t &operator|=(option_t &&other){
		if(&other == this){
			return *this;
		}
		if(!*this){
			*this = std::move(other);
		}
		return *this;
	}

	friend option_t operator|(option_t lhs, const T &rhs){
		lhs |= rhs;
		return lhs;
	}

	friend option_t operator|(option_t lhs, const option_t &rhs){
		lhs |= rhs;
		return lhs;
	}

	void clear(){
		is_set = false;
	}
};


