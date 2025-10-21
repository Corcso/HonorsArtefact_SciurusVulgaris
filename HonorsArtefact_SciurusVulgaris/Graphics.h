#pragma once
#include "PCH.h"
class Graphics
{
public:
	Graphics() = default;
	Graphics(Graphics& copy) = delete;

	static void Initialize(int width, int height, std::wstring title);

private:
	static Graphics instance;

	const LPCWSTR WINDOW_CLASS_NAME = L"2200592-SciurusVulgaris";

	HWND window;
};

