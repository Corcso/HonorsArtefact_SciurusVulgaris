#include "PCH.h"
#include "Input.h"
#include "Graphics.h"

Input Input::instance;

void Input::Initialize()
{
    instance.quitCalled = false;
    for (uint8_t i = 0; i < 0xFF; ++i) {
        instance.keys[i] = InputState::UP;
    }
    for (uint8_t i = 0; i < static_cast<uint8_t>(MouseButton::TOTAL_MOUSE_BUTTONS); ++i) {
        instance.mouseButtons[i] = InputState::UP;
    }
}

LRESULT Input::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT paintStruct;
    HDC hDC;

    // ImGui input
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
        return true;

    // Switch case over different message types we care about
    switch (message)
    {
        // Process key up and down events
    case WM_KEYUP:
    {
        Input::LogKeyRelease(wParam);
    }
    break;
    case WM_KEYDOWN:
    {
        // Windows sends repeat messages (Microsoft, 2025) so use the lParam to check the last state and break early if it was down
        if (lParam & 0x40000000) break;
        Input::LogKeyPress(wParam);
    }
    break;
    // Process mouse button events
    case WM_LBUTTONDOWN:
    {
        Input::LogMousePress(Input::MouseButton::LEFT);
    }
    break;
    case WM_LBUTTONUP:
    {
        Input::LogMouseRelease(Input::MouseButton::LEFT);
    }
    break;
    case WM_RBUTTONDOWN:
    {
        Input::LogMousePress(Input::MouseButton::RIGHT);
    }
    break;
    case WM_RBUTTONUP:
    {
        Input::LogMouseRelease(Input::MouseButton::RIGHT);
    }
    break;

    // Use the mouse move event if the mouse isnt locked otherwise use the WM_INPUT raw input event (Microsoft, 2023)
    // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/taking-advantage-of-high-dpi-mouse-movement
    case WM_MOUSEMOVE:
    {
        if (Input::IsMouseLocked()) break;
        int xCoord = 0x0000FFFF & lParam;
        int yCoord = (0xFFFF0000 & lParam) >> 16;
        Input::SetMousePosition(HMM_V2(xCoord, yCoord));
        Input::SetMousePositionDifference(HMM_V2(xCoord, yCoord) - Input::GetMousePositionLastFrame());

    }
    break;
    case WM_INPUT:
    {
        if (!Input::IsMouseLocked()) break;
        UINT dwSize = sizeof(RAWINPUT);
        static BYTE lpb[sizeof(RAWINPUT)];

        GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));

        RAWINPUT* raw = (RAWINPUT*)lpb;

        if (raw->header.dwType == RIM_TYPEMOUSE)
        {
            int xPosRelative = raw->data.mouse.lLastX;
            int yPosRelative = raw->data.mouse.lLastY;
            Input::SetMousePositionDifference(HMM_V2(xPosRelative, yPosRelative));
        }
        break;
    }
    break;
    case WM_SIZE:
    {
        HMM_Vec2 newSize;
        newSize.Width = LOWORD(lParam);
        newSize.Height = HIWORD(lParam);
        Graphics::RegisterWindowSizeChange(newSize);
    }
    break;
    case WM_PAINT:
    {
        hDC = BeginPaint(hwnd, &paintStruct);
        EndPaint(hwnd, &paintStruct);
    }
    break;
    case WM_DESTROY:
    {
        PostQuitMessage(0);
        Input::QuitMainLoop();
    }
    break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

bool Input::IsKeyDown(uint8_t keyCode)
{
    return static_cast<bool>(instance.keys[keyCode] & InputState::DOWN);
}

bool Input::IsKeyUp(uint8_t keyCode)
{
    return static_cast<bool>(instance.keys[keyCode] & InputState::UP);
}

bool Input::IsKeyReleased(uint8_t keyCode)
{
    return static_cast<bool>((instance.keys[keyCode] & InputState::RELEASED) == InputState::RELEASED);
}

bool Input::IsKeyPressed(uint8_t keyCode)
{
    return static_cast<bool>((instance.keys[keyCode] & InputState::PRESSED) == InputState::PRESSED);
}

void Input::LogKeyPress(uint8_t keyCode)
{
    if (instance.keys[keyCode] == InputState::PRESSED_AND_RELEASED_SAME_FRAME) instance.keys[keyCode] = InputState::RELEASED_AND_PRESSED_SAME_FRAME;
    else if (instance.keys[keyCode] == InputState::RELEASED) instance.keys[keyCode] = InputState::RELEASED_AND_PRESSED_SAME_FRAME;
    else instance.keys[keyCode] = InputState::PRESSED;
}

void Input::LogKeyRelease(uint8_t keyCode)
{
    if (instance.keys[keyCode] == InputState::RELEASED_AND_PRESSED_SAME_FRAME) instance.keys[keyCode] = InputState::PRESSED_AND_RELEASED_SAME_FRAME;
    else if (instance.keys[keyCode] == InputState::PRESSED) instance.keys[keyCode] = InputState::PRESSED_AND_RELEASED_SAME_FRAME;
    else instance.keys[keyCode] = InputState::RELEASED;
}

bool Input::IsMouseDown(MouseButton mouseCode)
{
    return static_cast<bool>(instance.mouseButtons[static_cast<int>(mouseCode)] & InputState::DOWN);
}

bool Input::IsMouseUp(MouseButton mouseCode)
{
    return static_cast<bool>(instance.mouseButtons[static_cast<int>(mouseCode)] & InputState::UP);
}

bool Input::IsMouseReleased(MouseButton mouseCode)
{
    return static_cast<bool>(instance.mouseButtons[static_cast<int>(mouseCode)] & InputState::RELEASED);
}

bool Input::IsMousePressed(MouseButton mouseCode)
{
    return static_cast<bool>(instance.mouseButtons[static_cast<int>(mouseCode)] & InputState::PRESSED);
}

void Input::LogMousePress(MouseButton mouseCode)
{
    if (instance.mouseButtons[static_cast<int>(mouseCode)] == InputState::PRESSED_AND_RELEASED_SAME_FRAME) instance.mouseButtons[static_cast<int>(mouseCode)] = InputState::RELEASED_AND_PRESSED_SAME_FRAME;
    else if (instance.mouseButtons[static_cast<int>(mouseCode)] == InputState::RELEASED) instance.mouseButtons[static_cast<int>(mouseCode)] = InputState::RELEASED_AND_PRESSED_SAME_FRAME;
    else instance.mouseButtons[static_cast<int>(mouseCode)] = InputState::PRESSED;
}

void Input::LogMouseRelease(MouseButton mouseCode)
{
    if (instance.mouseButtons[static_cast<int>(mouseCode)] == InputState::RELEASED_AND_PRESSED_SAME_FRAME) instance.mouseButtons[static_cast<int>(mouseCode)] = InputState::PRESSED_AND_RELEASED_SAME_FRAME;
    else if (instance.mouseButtons[static_cast<int>(mouseCode)] == InputState::PRESSED) instance.mouseButtons[static_cast<int>(mouseCode)] = InputState::PRESSED_AND_RELEASED_SAME_FRAME;
    else instance.mouseButtons[static_cast<int>(mouseCode)] = InputState::RELEASED;
}

HMM_Vec2 Input::GetMousePosition()
{
    return instance.mousePosition;
}

HMM_Vec2 Input::GetMousePositionLastFrame()
{
    return instance.mousePositionLastFrame;
}

HMM_Vec2 Input::GetMousePositionDifference()
{
    return instance.mousePositionDifference;
}

void Input::SetMousePosition(HMM_Vec2 mousePosition)
{
    instance.mousePosition = mousePosition;
}

void Input::SetMousePositionDifference(HMM_Vec2 mousePositionDifference)
{
    instance.mousePositionDifference = mousePositionDifference;
}

void Input::Update()
{
    for (uint8_t i = 0; i < 0xFF; ++i) {
        if (instance.keys[i] == InputState::PRESSED_AND_RELEASED_SAME_FRAME) instance.keys[i] = InputState::UP;
        else if (instance.keys[i] == InputState::RELEASED_AND_PRESSED_SAME_FRAME) instance.keys[i] = InputState::DOWN;
        else if (instance.keys[i] == InputState::RELEASED) instance.keys[i] = InputState::UP;
        else if (instance.keys[i] == InputState::PRESSED) instance.keys[i] = InputState::DOWN;
        else if (instance.keys[i] == InputState::INVALID) instance.keys[i] = InputState::UP; // Shouldn't be invalid, put up.
    }
    for (uint8_t i = 0; i < static_cast<uint8_t>(MouseButton::TOTAL_MOUSE_BUTTONS); ++i) {
        if (instance.mouseButtons[i] == InputState::PRESSED_AND_RELEASED_SAME_FRAME) instance.mouseButtons[i] = InputState::UP;
        else if (instance.mouseButtons[i] == InputState::RELEASED_AND_PRESSED_SAME_FRAME) instance.mouseButtons[i] = InputState::DOWN;
        else if (instance.mouseButtons[i] == InputState::RELEASED) instance.mouseButtons[i] = InputState::UP;
        else if (instance.mouseButtons[i] == InputState::PRESSED) instance.mouseButtons[i] = InputState::DOWN;
        else if (instance.mouseButtons[i] == InputState::INVALID) instance.mouseButtons[i] = InputState::UP; // Shouldn't be invalid, put up.
    }

    instance.mousePositionLastFrame = instance.mousePosition;

    // If we are locked, set mouse movement to 0 at end of the frame
    if (instance.isMouseLocked) instance.mousePositionDifference = HMM_V2(0, 0);
}

bool Input::ProcessEvents()
{
    // Windows event process loop, calls WndProc
    MSG msg = { 0 };
    // Loop over all messages which are pending, remove them from the windows message stack
    while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // If the mouse is locked, set it to the center of the window.
    if (instance.isMouseLocked) {
        
        SetCursorPos(Graphics::GetWindowLocation().X + (Graphics::GetSwapChainExtent().width / 2.0f), Graphics::GetWindowLocation().Y + (Graphics::GetSwapChainExtent().height / 2.0f));
    }

    return instance.quitCalled;
}

void Input::QuitMainLoop()
{
    instance.quitCalled = true;
}

void Input::SetMouseLock(bool locked)
{
    // Show cursor and set mouse locked to false
    if (instance.isMouseLocked != locked) ShowCursor(!locked);// Only do this on a toggle, it works like a counter. https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-showcursor
    instance.isMouseLocked = locked;
}

Input::InputState operator&(const Input::InputState& l, const Input::InputState& r)
{
    return static_cast<Input::InputState>(static_cast<uint8_t>(l) & static_cast<uint8_t>(r));
}

