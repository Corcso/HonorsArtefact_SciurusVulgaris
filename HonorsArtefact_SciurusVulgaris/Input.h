#pragma once
#include "PCH.h"
/// <summary>
/// Input singleton which stores the state of every key and mouse button. 
/// Contains functions for Down, Up, Pressed and Released. Where Pressed is also Down and Released is also Up
/// Also contains functions for locking the mouse. 
/// Singleton adapted from earlier work by myself, any references used within those works have been copied over. 
/// </summary>
class Input
{
public:

	Input() = default;
	Input(Input& copy) = delete;

	enum class InputState : uint8_t {
		INVALID = 0b000000,
		UP = 0b0000001,
		DOWN = 0b0000010,
		RELEASED = 0b00000100 | UP,
		PRESSED = 0b00001000 | DOWN,
		PRESSED_AND_RELEASED_SAME_FRAME = 0b00010000 | PRESSED | RELEASED,
		RELEASED_AND_PRESSED_SAME_FRAME = 0b00100000 | PRESSED | RELEASED
	};

	enum class MouseButton : uint8_t {
		INVALID,
		LEFT,
		RIGHT,
		TOTAL_MOUSE_BUTTONS
	};

	static void Initialize();

	static LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	static bool IsKeyDown(uint8_t keyCode);
	static bool IsKeyUp(uint8_t keyCode);
	static bool IsKeyReleased(uint8_t keyCode);
	static bool IsKeyPressed(uint8_t keyCode);

	static void LogKeyPress(uint8_t keyCode);
	static void LogKeyRelease(uint8_t keyCode);

	static bool IsMouseDown(MouseButton mouseCode);
	static bool IsMouseUp(MouseButton mouseCode);
	static bool IsMouseReleased(MouseButton mouseCode);
	static bool IsMousePressed(MouseButton mouseCode);

	static void LogMousePress(MouseButton mouseCode);
	static void LogMouseRelease(MouseButton mouseCode);

	static HMM_Vec2 GetMousePosition();
	static HMM_Vec2 GetMousePositionLastFrame();
	static HMM_Vec2 GetMousePositionDifference();

	static void SetMousePosition(HMM_Vec2 mousePosition);
	static void SetMousePositionDifference(HMM_Vec2 mousePositionDifference);

	static void Update();
	static bool ProcessEvents();
	static void QuitMainLoop();

	static void SetMouseLock(bool locked);
	static bool IsMouseLocked() { return instance.isMouseLocked; }

private:
	static Input instance;
	
	bool quitCalled;

	InputState keys[256];
	InputState mouseButtons[(unsigned long long)MouseButton::TOTAL_MOUSE_BUTTONS];

	HMM_Vec2 mousePositionLastFrame;
	HMM_Vec2 mousePosition;
	HMM_Vec2 mousePositionDifference;

	bool isMouseLocked;
};

Input::InputState operator&(const Input::InputState& l, const Input::InputState& r);