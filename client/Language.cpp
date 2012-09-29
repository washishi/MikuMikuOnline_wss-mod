//
// Language.cpp
//

#include "Language.hpp"
#include <unordered_map>
#include <boost/property_tree/json_parser.hpp>

using namespace boost::property_tree;

namespace Language {

	namespace {
		ptree text_tree;
		std::unordered_map<std::string, tstring> text_map;
	};

	void Initialize(const std::string& lang_name)
	{
		read_json("./system/languages/" + lang_name + ".json", text_tree);
		text_map.clear();
	}

	const tstring& Get(const std::string& name)
	{
		auto it = text_map.find(name);
		if (it != text_map.end()) {
			return it->second;
		} else {
			tstring text = unicode::ToTString(text_tree.get<std::string>(name, "[" + name + "]"));
			text_map[name] = text;
			return text_map[name];
		}
	}

};