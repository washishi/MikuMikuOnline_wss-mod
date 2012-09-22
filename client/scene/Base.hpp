//
// Base.hpp
//

#pragma once

#include "DxLib.h"
#include <memory>
#include "../../common/unicode.hpp"
#include "../InputManager.hpp"

namespace scene {
class Base;
typedef std::shared_ptr<Base> BasePtr;

class Base : public std::enable_shared_from_this<Base> {
    public:
        virtual ~Base();
	    virtual void Begin() = 0;
	    virtual void Update() = 0;
		virtual void ProcessInput(InputManager*) = 0;
	    virtual void Draw() = 0;
	    virtual void End() = 0;
	    virtual BasePtr NextScene() {return BasePtr();};
};

}
