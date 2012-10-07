//
// Channel.hpp
//

#include "Channel.hpp"
#include <boost/filesystem.hpp>

using namespace boost::filesystem;

Channel::Channel()
{
	Load();
}
		
void Channel::Load()
{
	pt_.clear();
	path p("./channels");

	if (exists(p) && is_directory(p)) {
		for (auto it_dir = directory_iterator(p); it_dir != directory_iterator(); ++it_dir) {
			if (is_directory(*it_dir)) {
				path json_path = it_dir->path() / "config.json";
				if (exists(json_path)) {
					auto channel_str = it_dir->path().leaf().string();
					try {
						boost::property_tree::ptree config_pt;
						read_json(json_path.string(), config_pt);
						pt_.put_child(channel_str, config_pt);

					} catch (const std::exception& e) {
						Logger::Error("%d", e.what());
					}
				}
			}
        }
    }
}

const boost::property_tree::ptree& Channel::pt() const
{
	return pt_;
}