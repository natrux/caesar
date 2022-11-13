#pragma once

#include <memory>

namespace std{

// pre-C++14
#if __cplusplus < 201402L
template<class T, class... Args>
unique_ptr<T> make_unique(Args &&...args){
	return unique_ptr<T>(new T(std::forward<Args>(args)...));
}
#endif

}

