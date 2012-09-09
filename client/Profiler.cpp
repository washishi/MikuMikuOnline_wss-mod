#include "Profiler.hpp"
#include <DxLib.h>
#include <fstream>
#include <vector>

#ifdef _DEBUG

namespace Profiler {

	Writer writer_instance;

	ScopeTimer::ScopeTimer(const std::string name) :
		name_(name)
	{
		start_time_ = GetNowHiPerformanceCount();
	}
	
	ScopeTimer::~ScopeTimer()
	{
		unsigned int delta = static_cast<unsigned int>(GetNowHiPerformanceCount() - start_time_);

		Info& info = writer_instance.datamap[name_];
		info.time_sum += delta;
		info.time_max = std::max(info.time_max, delta);
		info.time_min = std::min(info.time_min, delta);
		info.count++;
	}

	Writer::~Writer()
	{
		std::ofstream ofs("profile.log");
		std::vector<std::pair<std::string, Info>> v(datamap.begin(), datamap.end());
		std::sort(v.begin(), v.end(),
			[](const std::pair<std::string, Info>& a, const std::pair<std::string, Info>& b)
		{
			return a.second.time_sum > b.second.time_sum;
		});

		for (auto it = v.begin(); it != v.end(); ++it) {
			ofs << " *** " << it->first << std::endl;
			ofs << " count:   " << it->second.count << std::endl;
			ofs << " total:   " << it->second.time_sum << "ms" << std::endl;
			ofs << " average: " << 1.0 * it->second.time_sum / it->second.count << "ms" << std::endl;
			ofs << " max:     " << it->second.time_max << "ms" << std::endl;
			ofs << " min:     " << it->second.time_min << "ms" << std::endl;
			ofs << std::endl << std::endl;
		}
	}
}

#endif