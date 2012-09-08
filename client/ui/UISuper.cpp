//
// UISuper.cpp
//

#include "UISuper.hpp"

int UISuper::max_focus_index = 0;

int Hex2Int(const std::string& hex)
{
    int dec = 0;
    int base = 1;
    for (auto it = hex.rbegin(); it != hex.rend(); ++it) {
        auto c = *it;
        int d = 0;
        if ('A' <= c && c <= 'F') {
            d = (c - 'A') + 10;
        } else
        if ('a' <= c && c <= 'f'){
            d = (c - 'a') + 10;
        } else
        if ('0' <= c && c <= '9'){
            d = (c - '0');
        }
        dec += d * base;
        base *= 16;
    }
    return dec;
}

std::string UISuper::Color::ToString() const
{
    std::stringstream stream;
    stream << "#";
    stream << std::hex << std::setw(2) << std::setfill('0') << r;
    stream << std::hex << std::setw(2) << std::setfill('0') << g;
    stream << std::hex << std::setw(2) << std::setfill('0') << b;
    stream << std::hex << std::setw(2) << std::setfill('0') << a;
    return stream.str();
}

UISuper::Color UISuper::Color::FromString(const std::string & str)
{
    Color color;
    if (str.length() >= 7) {
        color.r = (unsigned char)Hex2Int(str.substr(1, 2));
        color.b = (unsigned char)Hex2Int(str.substr(3, 2));
        color.g = (unsigned char)Hex2Int(str.substr(5, 2));
        if (str.length() >= 9) {
            color.a = (unsigned char)Hex2Int(str.substr(7, 2));
        } else {
            color.a = 255;
        }
    }
    return color;
}

UISuper::UISuper() :
                focus_index_(0),
                width_(100),
                height_(100),
                top_(12),
                left_(12),
                bottom_(12),
                right_(12),
                docking_(DOCKING_TOP | DOCKING_LEFT),
                visible_(true)
{

}

int UISuper::absolute_x() const
{
    return absolute_rect_.x + offset_rect_.x;
}

int UISuper::absolute_y() const
{
    return absolute_rect_.y + offset_rect_.y;
}
int UISuper::absolute_width() const
{
    return absolute_rect_.width + offset_rect_.width;
}

int UISuper::absolute_height() const
{
    return absolute_rect_.height + offset_rect_.height;
}
void UISuper::UpdatePosition()
{
    int parent_x_,
        parent_y_,
        parent_width_,
        parent_height_;

    // 画面のサイズを取得
	parent_x_ = 0;
	parent_y_ = 0;
	GetScreenState(&parent_width_, &parent_height_, nullptr);

    // 幅を計算
    if ((docking_ & DOCKING_LEFT) && (docking_ & DOCKING_RIGHT)) {
        int left = parent_x_ + left_;
        int right = parent_x_ + parent_width_ - right_;
        absolute_rect_.width = right - left;
    } else {
        absolute_rect_.width = width_;
    }

    // 高さを計算
    if ((docking_ & DOCKING_TOP) && (docking_ & DOCKING_BOTTOM)) {
        int top = parent_y_ + top_;
        int bottom = parent_y_ + parent_height_ - bottom_;
        absolute_rect_.height = bottom - top;
    } else {
        absolute_rect_.height = height_;
    }

    // 左上X座標を計算
    if (docking_ & DOCKING_HCENTER) {
        absolute_rect_.x = parent_x_ + parent_width_ / 2 - absolute_rect_.width / 2;
    } else if (docking_ & DOCKING_RIGHT) {
        absolute_rect_.x = parent_x_ + parent_width_ - right_ - absolute_rect_.width;
    } else {
        absolute_rect_.x = parent_x_ + left_;
    }

    // 左上Y座標を計算
    if (docking_ & DOCKING_VCENTER) {
        absolute_rect_.y = parent_y_ + parent_height_ / 2 - absolute_rect_.height / 2;
    } else if (docking_ & DOCKING_BOTTOM) {
        absolute_rect_.y = parent_y_ + parent_height_ - bottom_ - absolute_rect_.height;
    } else {
        absolute_rect_.y = parent_y_ + top_;
    }
}

void UISuper::UpdateBaseImage()
{

}

int UISuper::height() const { return height_; }
void UISuper::set_height(int height) { height_ = height; }
int UISuper::width() const { return width_; }
void UISuper::set_width(int width) { width_ = width; }

int UISuper::top() const { return top_; }
void UISuper::set_top(int top) { top_ = top; }
int UISuper::left() const { return left_; }
void UISuper::set_left(int left) { left_ = left; }
int UISuper::right() const { return right_; }
void UISuper::set_right(int right) { right_ = right; }
int UISuper::bottom() const { return bottom_; }
void UISuper::set_bottom(int bottom) { bottom_ = bottom; }

int UISuper::docking() const { return docking_; }
void UISuper::set_docking(int docking) { docking_ = docking; }

void UISuper::Focus()
{
	focus_index_ = ++max_focus_index;
}

bool UISuper::visible() const
{
    return visible_;
}

void UISuper::set_visible(bool visible)
{
    visible_ = visible;
}

int UISuper::offset_x() const
{
    return offset_rect_.x;
}

void UISuper::set_offset_x(int offset_x)
{
    offset_rect_.x = offset_x;
}

int UISuper::offset_y() const
{
    return offset_rect_.y;
}

void UISuper::set_offset_y(int offset_y)
{
    offset_rect_.y = offset_y;
}

int UISuper::offset_width() const
{
    return offset_rect_.width;
}

void UISuper::set_offset_width(int offset_width)
{
    offset_rect_.width = offset_width;
}

int UISuper::offset_height() const
{
    return offset_rect_.height;
}

void UISuper::set_offset_height(int offset_height)
{
    offset_rect_.height = offset_height;
}

int UISuper::focus_index() const
{
    return focus_index_;
}