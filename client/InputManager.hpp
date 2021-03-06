//
// InputManager.hpp
//

#pragma once

#include <array>

class InputManager {
    public:
        InputManager();
        static void Update();
        void operator&=(const InputManager& input);

        static int GetGamepadNum();

        int GetMouseX() const;
        int GetMouseY() const;
        std::pair<int, int> GetMousePos() const;
        int GetMouseWheel() const;

        double GetGamepadAnalogX() const;
        double GetGamepadAnalogY() const;
        double GetGamepadAnalogZ() const;
        double GetGamepadAnalogRx() const;
        double GetGamepadAnalogRy() const;
        double GetGamepadAnalogRz() const;

		static void SetGamepadType(int);
		static void SetGamepadEnable(int); // ※ ゲームパッド有効をウインドウアクティブ時のみにもできる様に追加
// ※ ここから  ゲームパッドのボタンを変更できる様に追加
		static void SetGamepadJump(int);
		static void SetGamepadSpeed(int);
		static void SetGamepadWarp(int);
		static void SetGamepadSShot(int);
// ※ ここまで

		double GetGamepadManagedAnalogRx() const;
        double GetGamepadManagedAnalogRy() const;

        double GetGamepadPOVX() const;
        double GetGamepadPOVY() const;

        double GetGamepadSlider1() const;
        double GetGamepadSlider2() const;

        int GetMouseRightCount() const;
        int GetMouseLeftCount() const;
        int GetMouseMiddleCount() const;

        bool GetMouseRight() const;
        bool GetMouseLeft() const;
        bool GetMouseMiddle() const;

        bool GetPrevMouseRight() const;
        bool GetPrevMouseLeft() const;
        bool GetPrevMouseMiddle() const;

        int GetKeyCount(int) const;
        int GetGamepadCount(int) const;

        void CancelMouseWheel();
        void CancelMouseRight();
        void CancelMouseLeft();
        void CancelMouseMiddle();
        void CancelKeyCount(int);
        void CancelKeyCountAll();

    private:
        static int mouse_x_, mouse_y_;
        static double pad_x_, pad_y_, pad_z_;
        static double pad_rx_, pad_ry_, pad_rz_;
        static double pov_x_, pov_y_;
        static double slider1_, slider2_;
        static int static_mouse_right_count_, static_mouse_left_count_, static_mouse_middle_count_;
        static bool static_mouse_right_, static_mouse_left_, static_mouse_middle_;
        static bool prev_mouse_right_, prev_mouse_left_, prev_mouse_middle_;
        static int static_mouse_wheel_;

		static int gamepad_type_;
		static int gamepad_enable_; // ※ ゲームパッド有効をウインドウアクティブ時のみにもできる様に追加
// ※ ここから  ゲームパッドのボタンを変更できる様に追加
		static int gamepad_jump_;
		static int gamepad_speed_;
		static int gamepad_warp_;
		static int gamepad_sshot_;
// ※ ここまで

        bool mouse_right_, mouse_left_, mouse_middle_;
        int mouse_right_count_, mouse_left_count_, mouse_middle_count_;
        int mouse_wheel_;
        std::array<int, 256> key_count_;
        std::array<int, 32> pad_count_;

        static std::array<int, 256> static_key_count_;
        static std::array<int, 256> static_key_count_tmp_;
        static std::array<int, 32> static_pad_count_;
        static std::array<int, 32> static_pad_count_tmp_;

    // キーバインド
    public:
        static int
        KEYBIND_FORWARD,
        KEYBIND_BACK,
        KEYBIND_RIGHT_TRUN,
        KEYBIND_LEFT_TURN,
        KEYBIND_JUMP,
        KEYBIND_CHANGE_SPEED,
        KEYBIND_CHANGE_SPEED2,
        KEYBIND_TAB,
        KEYBIND_SHIFT,
        KEYBIND_RETURN,
        KEYBIND_EXIT,
        KEYBIND_REFRESH,
        KEYBIND_SCRIPT_MODE,
		KEYBIND_SCREEN_SHOT,
		KEYBIND_ENTER,

		KEYBIND_LCTRL,
		KEYBIND_RCTRL,
		KEYBIND_LALT,
		KEYBIND_RALT,

		KEYBIND_MOTION_01,
		KEYBIND_MOTION_02,
		KEYBIND_MOTION_03,
		KEYBIND_MOTION_04,
		KEYBIND_MOTION_05,
		KEYBIND_MOTION_06,
		KEYBIND_MOTION_07,
		KEYBIND_MOTION_08,
		KEYBIND_MOTION_09,
		KEYBIND_MOTION_00,

// ※ ここから パッド操作追加の為修正
//      PADBIND_JUMP
        PADBIND_JUMP,
        PADBIND_SPEED,
        PADBIND_WARP,
        PADBIND_SCREEN_SHOT
// ※ ここまで
        ;
};
