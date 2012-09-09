
#pragma once
#include <string>
#include <unordered_map>

#ifdef _DEBUG
#define MMO_PROFILE_FUNCTION Profiler::ScopeTimer(__FUNCTION__)
#else 
#define MMO_PROFILE_FUNCTION
#endif

#ifdef _DEBUG

namespace Profiler {

	struct Info {
		Info() : time_sum(0), count(0), time_min(UINT_MAX), time_max(0) {}
		long long time_sum;
		unsigned int count;
		unsigned int time_min, time_max;
	};

	struct Writer {
		~Writer();
		std::unordered_map<std::string, Info> datamap;
	};

	class ScopeTimer {
	public:
		ScopeTimer(const std::string name);
		~ScopeTimer();

	private:
		std::string name_;
		long long start_time_;
	};
}

#endif