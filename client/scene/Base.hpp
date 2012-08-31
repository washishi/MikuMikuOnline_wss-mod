//
// Base.hpp
//

#pragma once

#include "DxLib.h"
#include <memory>
#include "../../common/unicode.hpp"

namespace scene {
class Base;
typedef std::shared_ptr<Base> BasePtr;

class Base {
    public:
        virtual ~Base();
	    virtual void Begin() = 0;
	    virtual void Update() = 0;
	    virtual void Draw() = 0;
	    virtual void End() = 0;
	    virtual BasePtr NextScene() {return BasePtr();};
};

}
