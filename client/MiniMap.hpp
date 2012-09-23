//
// MiniMap.hpp
//
#pragma once

#include <array>
#include <unordered_map>
#include "ManagerAccessor.hpp"
#include "ResourceManager.hpp"
#include "PlayerManager.hpp"
#include "InputManager.hpp"
#include "WorldManager.hpp"
#include "ui/UISuper.hpp"
#include "3d/Stage.hpp"

#define MINIMAP_MAXSIZE 800
#define MINIMAP_MINSIZE 132

class MiniMap : public UISuper
{
public:
	MiniMap(const ManagerAccessorPtr& manager_accessor);
	void ProcessInput(InputManager* input);
	void Update();
	void Draw();

	bool resizable() const;
	void set_resizable(bool resizable);

	void UIPlacement(int x = 12, int y = 12, int height = MINIMAP_MINSIZE + 16, int width = MINIMAP_MINSIZE);

private:
	void UpdateDrag(InputManager* input, bool resizeable = true);
	void DrawPosAndCalc();

private:
	std::array<ImageHandlePtr,4> base_image_handle_;

	ManagerAccessorPtr manager_accessor_;

	VECTOR prev_myself_pos_on_map_;

	bool resizable_;

	int max_width_, min_width_;
	int max_height_, min_height_;

	int font_handle_;

    Rect drag_offset_rect_, drag_resize_offset_rect_;
private:
	static const int BASE_BLOCK_SIZE;
	static float ZOOM_SIZE;	// î{ó¶ÅAñ¢égóp
};

typedef std::shared_ptr<MiniMap> MiniMapPtr;