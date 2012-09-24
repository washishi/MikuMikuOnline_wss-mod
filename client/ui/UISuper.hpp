//
// UISuper.hpp
//

#pragma once

#include <memory>
#include <string>
#include <iostream>
#include <vector>
#include "../ResourceManager.hpp"

class InputManager;

struct Rect {
    Rect(int x_ = 0, int y_ = 0, int width_ = 0, int height_ = 0) : x(x_), y(y_), width(width_), height(height_) {}
    int x, y, width, height;
};

struct Point {
    Point(int x_ = 0, int y_ = 0) : x(x_), y(y_) {}
    int x, y;
};

class UISuper : public std::enable_shared_from_this<UISuper> {
    public:
        struct Color{
                Color(unsigned char _r = 255, unsigned char _g = 255,
                        unsigned char _b = 255, unsigned char _a = 255) :
                                r(_r),
                                g(_g),
                                b(_b),
                                a(_a)
                {
                }
            unsigned char r, g, b, a;
            std::string ToString() const;
            static Color FromString(const std::string & str);
        };

    public:
        UISuper();
		virtual ~UISuper(){};

        int absolute_x() const;
        int absolute_y() const;
        int absolute_width() const;
        int absolute_height() const;

		virtual void Draw() {}
		virtual void Update() {}
		virtual void ProcessInput(InputManager* input) {}

        int height() const;
        void set_height(int height);
        int width() const;
        void set_width(int width);

        int top() const;
        void set_top(int top);
        int left() const;
        void set_left(int left);
        int right() const;
        void set_right(int right);
        int bottom() const;
        void set_bottom(int bottom);

        int docking() const;
        void set_docking(int docking);
        bool visible() const;
        void set_visible(bool visible);

        int offset_x() const;
        void set_offset_x(int offset_x);
        int offset_y() const;
        void set_offset_y(int offset_y);
        int offset_width() const;
        void set_offset_width(int offset_width);
        int offset_height() const;
        void set_offset_height(int offset_height);

		int focus_index() const;

		ImageHandlePtr icon_image_handle() const;
		void set_icon_image_handle(const ImageHandlePtr& handle);

        enum {
            DOCKING_NONE = 0,
            DOCKING_TOP = 1,
            DOCKING_LEFT = 2,
            DOCKING_RIGHT = 4,
            DOCKING_BOTTOM = 8,
            DOCKING_HCENTER = 16,
            DOCKING_VCENTER = 32,
            DOCKING_ALL = 1 | 2 | 4 | 8
        };

    protected:

        virtual void UpdatePosition();

        virtual void UpdateBaseImage();

        void Focus();

	protected:
        static int max_focus_index;

    protected:

        int focus_index_;  
        int width_, height_;
        int top_, left_, bottom_, right_;
        Rect absolute_rect_, offset_rect_;
        int docking_;
        bool visible_;
        std::string base_image_;

		ImageHandlePtr icon_image_handle_;
};

typedef std::shared_ptr<UISuper> UISuperPtr;
typedef std::weak_ptr<UISuper> UISuperWeakPtr;