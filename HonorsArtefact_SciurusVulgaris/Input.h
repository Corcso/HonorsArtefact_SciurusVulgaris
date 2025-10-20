#pragma once
#include "PCH.h"
class Input
{
public:

	enum class InputState : uint8_t {
		INVALID = 0b000000,
		UP = 0b0000001,
		DOWN = 0b0000010,
		RELEASED = 0b00000100 | UP,
		PRESSED = 0b00001000 | DOWN,
		PRESSED_AND_RELEASED_SAME_FRAME = 0b00010000 | PRESSED | RELEASED,
		RELEASED_AND_PRESSED_SAME_FRAME = 0b00100000 | PRESSED | RELEASED
	};

	InputState operator&(const InputState& r) {
		return static_cast<InputState>(static_cast<uint8_t>(this) & static_cast<uint8_t>(r));
	}


	static LRESULT WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

	bool IsKeyDown(uint8_t keyCode);
	bool IsKeyUp(uint8_t keyCode);
	bool IsKeyReleased(uint8_t keyCode);
	bool IsKeyPressed(uint8_t keyCode);

	bool IsMouseDown(uint8_t mouseCode);
	bool IsMouseUp(uint8_t mouseCode);
	bool IsMouseReleased(uint8_t mouseCode);
	bool IsMousePressed(uint8_t mouseCode);

private:
	InputState keys[256];
	InputState mouse[1];

};

