#pragma once


#include <queue>
#include <mutex>


template<class T>
class SafeQueue{
public:
	void push(const T &value){
		std::lock_guard<std::mutex> lock(mutex);
		queue.push(value);
	}

	bool pop(T &value){
		std::lock_guard<std::mutex> lock(mutex);
		if(queue.empty()){
			return false;
		}
		value = queue.front();
		queue.pop();
		return true;
	}

	bool empty() const{
		std::lock_guard<std::mutex> lock(mutex);
		return queue.empty();
	}

private:
	std::queue<T> queue;
	mutable std::mutex mutex;
};

