//
// UIGroup.cpp
//

/**
* @module global
* @submodule UI
 */

/**
 * @class Group
 *
 * @namespace UI
 * @extends UI.Base
 */

#include "UIGroup.hpp"
#include "../ScriptEnvironment.hpp"
#include "../ResourceManager.hpp"

UIGroup::UIGroup()
{
}

UIGroup::~UIGroup()
{
}

void UIGroup::DefineInstanceTemplate(Handle<ObjectTemplate>* object)
{
    UIBase::DefineInstanceTemplate(object);

}

void UIGroup::ProcessInput(InputManager* input)
{
    ProcessInputChildren(input);
}

void UIGroup::Update()
{
    UpdatePosition();

    //int max_y = 0;
    for(auto it = children_.begin(); it != children_.end(); ++it) {
        auto child = *it;
//      UIBasePtr child_ptr = *static_cast<UIBasePtr*>(child->GetPointerFromInternalField(0));
		UIBasePtr child_ptr = *static_cast<UIBasePtr*>(Local<External>::Cast(child->GetInternalField(0))->Value());
        child_ptr->Update();
        //max_y = max(max_y, child_ptr->)
    }
}

void UIGroup::Draw()
{
    if (!visible_) {
        return;
    }
    DrawChildren();
}
