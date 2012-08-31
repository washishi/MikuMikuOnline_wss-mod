//
// UIButton.cpp
//

/**
* @module global
* @submodule UI
 */

/**
 * @class Button
 * @namespace UI
 * @extends UI.Base
 */

#include "UIButton.hpp"
#include "../ScriptEnvironment.hpp"

const int UIButton::BASE_BLOCK_SIZE = 12;

UIButton::UIButton()
{
    base_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
            _T("resources/images/gui/gui_button_base.png"), 2, 2, 12, 12);
}

UIButton::~UIButton()
{
}

void UIButton::DefineInstanceTemplate(Handle<ObjectTemplate>* object)
{
    UIBase::DefineInstanceTemplate(object);
}

void UIButton::ProcessInput(InputManager* input)
{
    ProcessInputChildren(input);

    bool hover = (absolute_x()<= input->GetMouseX() && input->GetMouseX() <= absolute_x()+ absolute_width()
            && absolute_y() <= input->GetMouseY() && input->GetMouseY() <= absolute_y() + absolute_height());
    if (hover && input->GetMouseLeftCount() == 1) {
        if (on_click_) {
            on_click_();
        }
    }
}

void UIButton::Update()
{
    UpdatePosition();
}

void UIButton::Draw()
{
    if (!visible_) {
        return;
    }

    DrawBase();
    DrawChildren();
}

void UIButton::DrawBase()
{
    int x = absolute_x();
    int y = absolute_y();
    int width = absolute_width();
    int height = absolute_height();

    DrawGraph(x, y, *base_image_handle_[0], TRUE);
    DrawGraph(x + width - BASE_BLOCK_SIZE, y, *base_image_handle_[1], TRUE);
    DrawGraph(x, y + height - BASE_BLOCK_SIZE, *base_image_handle_[2], TRUE);
    DrawGraph(x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y,
                         x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         0, 0, 1, BASE_BLOCK_SIZE, *base_image_handle_[1], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         x + width - BASE_BLOCK_SIZE, y + height,
                         0, 0, 1, BASE_BLOCK_SIZE, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x, y + BASE_BLOCK_SIZE,
                         x + BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         0, 0, BASE_BLOCK_SIZE, 1, *base_image_handle_[2], TRUE);

    DrawRectExtendGraphF(x + width - BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         x + width, y + height - BASE_BLOCK_SIZE,
                         0, 0, BASE_BLOCK_SIZE, 1, *base_image_handle_[3], TRUE);

    DrawRectExtendGraphF(x + BASE_BLOCK_SIZE, y + BASE_BLOCK_SIZE,
                         x + width - BASE_BLOCK_SIZE, y + height - BASE_BLOCK_SIZE,
                         0, 0, 1, 1, *base_image_handle_[3], TRUE);

}

void UIButton::set_on_click(const CallbackFunc& func)
{
    on_click_ = func;
}
