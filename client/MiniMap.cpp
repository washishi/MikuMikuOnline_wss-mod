//
// MiniMap.cpp
//

#include "MiniMap.hpp"
#include "Profiler.hpp"

const int MiniMap::BASE_BLOCK_SIZE = 24;
float MiniMap::ZOOM_SIZE = 1.0f;

MiniMap::MiniMap(const ManagerAccessorPtr& manager_accessor) :
                max_width_(MINIMAP_MAXSIZE),
                min_width_(MINIMAP_MINSIZE),
                max_height_(MINIMAP_MAXSIZE),
                min_height_(MINIMAP_MINSIZE),
				prev_myself_pos_on_map_(VGet(0,0,0)),
				drag_offset_rect_(-1, -1, -1, -1),
				drag_resize_offset_rect_(-1, -1, -1, -1),
				manager_accessor_(manager_accessor),
				font_handle_(ResourceManager::default_font_handle())
{
	base_image_handle_ = ResourceManager::LoadCachedDivGraph<4>(
		_T("resources/images/gui/gui_board_bg.png"), 2, 2, 24, 24);
}

void MiniMap::UpdateDrag(InputManager* input, bool resizeable)
{
    int screen_width, screen_height;
    GetScreenState(&screen_width, &screen_height, nullptr);

    bool hover = (absolute_x()<= input->GetMouseX() && input->GetMouseX() <= absolute_x()+ absolute_width()
            && absolute_y() <= input->GetMouseY() && input->GetMouseY() <= absolute_y() + absolute_height());

    bool corner_hover = (absolute_x() + absolute_width() - 18 <= input->GetMouseX()
            && input->GetMouseX() <= absolute_x() + absolute_width()
            && absolute_y() + absolute_height() - 18 <= input->GetMouseY()
            && input->GetMouseY() <= absolute_y() + absolute_height());

    // アクティブ
    if (hover && input->GetMouseLeftCount() == 1) {
        Focus();
    }

    //Logger::Log("%d, %d, %d", hover, input->GetMouseLeftCount(), drag_resize_offset_rect_.x);

    // ドラッグ処理
    if (input->GetMouseLeft()) {
        if (input->GetMouseLeftCount() == 1) {
            if (drag_offset_rect_.x < 0 && hover
                    && !corner_hover) {
                //drag_offset_rect_.x = input->GetMouseX() - x_;
                //drag_offset_rect_.y = input->GetMouseY() - y_;
                drag_offset_rect_.x = input->GetMouseX() - offset_rect_.x;
                drag_offset_rect_.y = input->GetMouseY() - offset_rect_.y;
            }
            if (drag_resize_offset_rect_.x < 0
                    && corner_hover) {
                //drag_resize_offset_rect_.x = x_ + width_ - input->GetMouseX();
                //drag_resize_offset_rect_.y = y_ + height_ - input->GetMouseY();
                drag_resize_offset_rect_.x = input->GetMouseX() - offset_rect_.width;
                drag_resize_offset_rect_.y = input->GetMouseY() - offset_rect_.height;
            }
        }
    } else {
        drag_offset_rect_.x = -1;
        drag_offset_rect_.y = -1;
        drag_resize_offset_rect_.x = -1;
        drag_resize_offset_rect_.y = -1;
    }

    if (drag_offset_rect_.x >= 0) {
        offset_rect_.x= input->GetMouseX() - drag_offset_rect_.x;
        offset_rect_.y= input->GetMouseY() - drag_offset_rect_.y;
        input->CancelMouseLeft();
    } else if (drag_resize_offset_rect_.x >= 0) {
        offset_rect_.width= input->GetMouseX() -  drag_resize_offset_rect_.x;
        offset_rect_.height= input->GetMouseY() - drag_resize_offset_rect_.y;
        input->CancelMouseLeft();

        if (width_ + offset_rect_.width < min_width_) {
            offset_rect_.width = min_width_ - width_;
        } else if (width_ + offset_rect_.width > max_width_) {
            offset_rect_.width = max_width_ - width_;
        }

        if (height_ + offset_rect_.height < min_height_) {
            offset_rect_.height = min_height_ - height_;
        } if (height_ + offset_rect_.height > max_height_) {
            offset_rect_.height = max_height_ - height_;
        }
    }
}

void MiniMap::UIPlacement(int x, int y, int height, int width)
{
	set_left(x);
	set_top(y);
	set_height(height);
	set_width(width);
}

void MiniMap::ProcessInput(InputManager* input)
{
	MMO_PROFILE_FUNCTION;

	UpdateDrag(input, resizable_);
}

void MiniMap::Draw()
{
	MMO_PROFILE_FUNCTION;

    if (!visible_) {
        return;
    }

	// BASEGRAPH_BEGINDRAW
    SetDrawBlendMode(DX_BLENDMODE_ADD, 255);

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

    SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
	// BASEGRAPH_ENDDRAW

	auto DrawOfOnlyEdge = [](int x, int y, int width, int height, int Color, int thickness)
	{
		DrawBox( x, y, x + width, y + thickness, Color, TRUE);
		DrawBox( x, y, x + thickness, y + height, Color, TRUE);
		DrawBox( x + width - thickness, y, x + width, y + height, Color, TRUE);
		DrawBox( x, y + height - thickness, x + width, y + height, Color, TRUE);
	};// thicknessで示した太さで縁のみの四角形を描画

	DrawOfOnlyEdge(x + 12, y + 12, width - 24, height - 24 - 16, GetColor(133,211,192),2);

	DrawPosAndCalc();
}

void MiniMap::DrawPosAndCalc()
{

	auto player_manager_ = manager_accessor_->player_manager().lock();
	auto world_manager_ = manager_accessor_->world_manager().lock();

	const auto& providers = player_manager_->char_data_providers();
	auto myself_pos = player_manager_->char_data_providers()[player_manager_->charmgr()->my_character_id()]->position();

	VECTOR direction = {0},move_direction = {0};
	move_direction = VSize(myself_pos - prev_myself_pos_on_map_) == 0 ? move_direction : myself_pos - prev_myself_pos_on_map_;
	move_direction.y = 0;
	float tmp_pos_x = 0, tmp_pos_z = 0;
	auto theta = 0.0f, mtheta = player_manager_->char_data_providers()[player_manager_->charmgr()->my_character_id()]->theta();

	// 先に自分の位置を描画【中央固定】
	DrawCircle( absolute_x() + absolute_width()/2, absolute_y() + absolute_height()/2, 2, GetColor(206,52,95));

	auto it = providers.begin();
	for(it; it != providers.end(); ++it)
	{
		if(it->first == player_manager_->charmgr()->my_character_id())continue;

		direction = VSub(it->second->position(),myself_pos);
		direction.y = 0;
		theta = atan2( -direction.x, -direction.z);
		tmp_pos_x = ( sin(mtheta + TORADIAN(180.0f) - theta) * VSize(direction) * ( 1.0f / world_manager_->stage()->map_scale()) )/ 3.0f + absolute_x() + absolute_width()/2;
		tmp_pos_z = ( cos(mtheta + TORADIAN(180.0f) - theta) * VSize(direction) * ( 1.0f / world_manager_->stage()->map_scale()) )/ 3.0f + absolute_y() + absolute_height()/2; // y座標化する
		if(tmp_pos_x < absolute_x() + 12)tmp_pos_x = absolute_x() + 12;
		if(tmp_pos_x > absolute_x() + absolute_width() - 12)tmp_pos_x = absolute_x() + absolute_width() - 12;
		if(tmp_pos_z < absolute_y() + 12)tmp_pos_z = absolute_y() + 12;
		if(tmp_pos_z > absolute_y() + absolute_height() -12)tmp_pos_z = absolute_y() + absolute_height() - 12;
		DrawCircle( tmp_pos_x, tmp_pos_z, 2, GetColor(23,162,175),TRUE);
	}
	prev_myself_pos_on_map_ = player_manager_->char_data_providers()[player_manager_->charmgr()->my_character_id()]->position();

	TCHAR tcs_tmp[256];
	_stprintf(tcs_tmp, _T("ログイン人数: %d"),player_manager_->GetAll().size());
	DrawBox(absolute_x() + 12,absolute_y() + absolute_height() - 24,absolute_x() + absolute_width() - 12,absolute_y() + absolute_height() - 6,GetColor(133,211,192),TRUE);
	DrawStringToHandle(absolute_x() + 10,absolute_y() + absolute_height() - 24 + 2, tcs_tmp,GetColor(34,34,34),font_handle_);
}

void MiniMap::Update()
{
	MMO_PROFILE_FUNCTION;

	UpdatePosition();
}

bool MiniMap::resizable() const
{
    return resizable_;
}

void MiniMap::set_resizable(bool resizable)
{
    resizable_ = resizable;
}

