#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <stdexcept>


class ThreadPool{
public:
	ThreadPool(size_t size);

	~ThreadPool();

	// Starts all threads and makes them wait for jobs.
	void start();

	// Adds a job to the outstanding jobs. job is allowed to throw std::runtime_error which can then be retrieved with status().
	void add_job(const std::function<void()> &job);

	// Throws the next collected exception (std::runtime_error), if any.
	void status();

	// Waits until all pending jobs have been processed.
	void wait();

	// Cancels all outstanding (not started) jobs.
	void cancel();

	// Tells the threads to exit as soon as there are no more jobs. Also, the pool stops accepting new jobs. Call join() afterwards.
	void close();

	// Waits until all threads have terminated. Call close() before.
	void join();

	// Returns the current load. A value of 1.0 means there is one pending or active job for each thread.
	double get_load() const;

private:
	bool out_of_jobs = false;
	std::vector<std::thread> threads;

	mutable std::mutex mutex_jobs;
	std::condition_variable cv_jobs;
	std::queue<std::function<void()>> jobs;
	size_t jobs_in_progress = 0;

	std::mutex mutex_exceptions;
	std::queue<std::runtime_error> exceptions;

	void job_loop();
};


