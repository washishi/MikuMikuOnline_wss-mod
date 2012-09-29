//
// Language.hpp
//

#pragma once

#define _LT(name) (Language::Get(name).c_str())

namespace Language {
	void Initialize(const std::string& lang_name);
	const tstring& Get(const std::string& name);
};