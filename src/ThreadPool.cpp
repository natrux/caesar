#include <util/ThreadPool.h>

#include <stdexcept>
#include <limits>


ThreadPool::ThreadPool(size_t size):
	threads(size)
{
}


ThreadPool::~ThreadPool(){
	close();
	cancel();
	join();
}


void ThreadPool::start(){
	for(auto &thread : threads){
		thread = std::thread(std::bind(&ThreadPool::job_loop, this));
	}
}


void ThreadPool::add_job(const std::function<void()> &job){
	if(out_of_jobs){
		throw std::logic_error("No more jobs accepted");
	}
	{
		std::lock_guard<std::mutex> lock(mutex_jobs);
		jobs.push(job);
	}
	cv_jobs.notify_all();
}


void ThreadPool::status(){
	std::lock_guard<std::mutex> lock(mutex_exceptions);
	if(!exceptions.empty()){
		auto ex = exceptions.front();
		exceptions.pop();
		std::rethrow_exception(ex);
	}
}


void ThreadPool::wait(){
	std::unique_lock<std::mutex> lock(mutex_jobs);
	while(!jobs.empty() || jobs_in_progress > 0){
		cv_jobs.wait(lock);
	}
}


void ThreadPool::cancel(){
	std::lock_guard<std::mutex> lock(mutex_jobs);
	jobs = decltype(jobs)();
}


void ThreadPool::close(){
	out_of_jobs = true;
	cv_jobs.notify_all();
}


void ThreadPool::join(){
	size_t to_join = threads.size();
	for(auto &thread : threads){
		if(thread.joinable()){
			thread.join();
			to_join--;
		}
	}
	if(to_join > 0){
		throw std::runtime_error(std::to_string(to_join) + " threads not joinable");
	}
}


double ThreadPool::get_load() const{
	size_t num_jobs;
	{
		std::lock_guard<std::mutex> lock(mutex_jobs);
		num_jobs = jobs.size() + jobs_in_progress;
	}
	if(threads.empty()){
		if(num_jobs == 0){
			return 1.0;
		}else{
			return std::numeric_limits<double>::max();
		}
	}
	return static_cast<double>(num_jobs) / threads.size();
}


void ThreadPool::job_loop(){
	while(true){
		std::function<void()> job;
		{
			std::unique_lock<std::mutex> lock(mutex_jobs);
			while(jobs.empty() && !out_of_jobs){
				cv_jobs.wait(lock);
			}
			if(!jobs.empty()){
				job = jobs.front();
				jobs.pop();
				jobs_in_progress++;
				cv_jobs.notify_all();
			}else{
				// out of jobs
				break;
			}
		}
		if(job){
			try{
				job();
			}catch(...){
				std::lock_guard<std::mutex> lock(mutex_exceptions);
				exceptions.push(std::current_exception());
			}
			{
				std::lock_guard<std::mutex> lock(mutex_jobs);
				jobs_in_progress--;
				cv_jobs.notify_all();
			}
		}
	}
}


