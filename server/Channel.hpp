//
// Channel.cpp
//

#pragma once
#include <boost/property_tree/json_parser.hpp>

class Channel {
	public:
		Channel();

		const boost::property_tree::ptree& pt() const;
		std::string GetDefaultStage() const;
		int GetDefaultCapacity() const;

	private:
		void Load();

	private:
		boost::property_tree::ptree pt_;
};