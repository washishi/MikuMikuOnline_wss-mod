//
// Base.cpp
//

#include "Base.hpp"

namespace scene {
Base::~Base()
{

}

BasePtr Base::NextScene() {
	BasePtr ptr;
	std::swap(ptr, next_scene_);
	return ptr;
}

}
