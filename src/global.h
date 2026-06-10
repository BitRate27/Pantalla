#pragma once
#include "ndi-loader.h"
#include <chrono>
extern const NDIlib_v6* ndiLib;
extern std::atomic<bool> g_Running;
extern std::atomic<bool> g_SessionRunning;
extern void log_file(const char *fmt, ...);
extern void close_log();
class PerfTimer {
public:
	PerfTimer() : name_(""), frame_counter_(0), report_interval_(100), min_ns_(INT64_MAX), max_ns_(0), sum_ns_(0) {}
	void init(const std::string &name, uint64_t report_interval = 100)
	{
		name_ = name;
		report_interval_ = report_interval;
	}
	void start() { start_time_ = std::chrono::high_resolution_clock::now(); }

	void end()
	{
		auto t1 = std::chrono::high_resolution_clock::now();
		int64_t elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - start_time_).count();
		++frame_counter_;
		sum_ns_ += (uint64_t)elapsed;
		if (elapsed < min_ns_)
			min_ns_ = elapsed;
		if (elapsed > max_ns_)
			max_ns_ = elapsed;

		if (frame_counter_ >= report_interval_) {
			double avg = static_cast<double>(sum_ns_) / static_cast<double>(frame_counter_);
			log_file("%s: Performance (last %llu frames): min=%.2f ms, max=%.2f ms, avg=%.2f ms\n",
				 name_.c_str(), frame_counter_, min_ns_ / 1e6, max_ns_ / 1e6, avg / 1e6);
			// reset
			frame_counter_ = 0;
			sum_ns_ = 0;
			min_ns_ = INT64_MAX;
			max_ns_ = 0;
		}
	}

private:
	std::string name_;
	uint64_t frame_counter_;
	uint64_t report_interval_;
	int64_t min_ns_;
	int64_t max_ns_;
	uint64_t sum_ns_;
	std::chrono::high_resolution_clock::time_point start_time_;
};
