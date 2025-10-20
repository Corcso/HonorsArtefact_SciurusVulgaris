#include "PCH.h"
#include "Input.h"

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
        if (Input::IsKeyPressed(virtualKeyCodeLookupTable[wParam])) Input::SetKeyState(virtualKeyCodeLookupTable[wParam], InputState::SAME_FRAME_PRESS_RELEASE);
        else Input::SetKeyState(virtualKeyCodeLookupTable[wParam], InputState::RELEASED);
    }
    break;
    case WM_KEYDOWN:
    {
        // Windows sends repeat messages (Microsoft, 2019) so use the lParam to check the last state and break early if it was down
        if (lParam & 0x40000000) break;
        if (Input::IsKeyReleased(virtualKeyCodeLookupTable[wParam])) Input::SetKeyState(virtualKeyCodeLookupTable[wParam], InputState::SAME_FRAME_PRESS_RELEASE);
        else Input::SetKeyState(virtualKeyCodeLookupTable[wParam], InputState::PRESSED);
    }
    break;
    // Process mouse button events
    case WM_LBUTTONDOWN:
    {
        Input::SetMouseState(Input::MouseButton::LEFT, InputState::PRESSED);
    }
    break;
    case WM_LBUTTONUP:
    {
        Input::SetMouseState(Input::MouseButton::LEFT, InputState::RELEASED);
    }
    case WM_RBUTTONDOWN:
    {
        Input::SetMouseState(Input::MouseButton::RIGHT, InputState::PRESSED);
    }
    break;
    case WM_RBUTTONUP:
    {
        Input::SetMouseState(Input::MouseButton::RIGHT, InputState::RELEASED);
    }
    break;

    // Use the mouse move event if the mouse isnt locked otherwise use the WM_INPUT raw input event (Microsoft, 2023)
    // https://learn.microsoft.com/en-us/windows/win32/dxtecharts/taking-advantage-of-high-dpi-mouse-movement
    case WM_MOUSEMOVE:
    {
        if (Input::IsMouseLocked()) break;
        int xCoord = 0x0000FFFF & lParam;
        int yCoord = (0xFFFF0000 & lParam) >> 16;
        Input::SetMousePosition(V2(xCoord, yCoord));
        Input::SetMouseMovement(V2(xCoord, yCoord) - Input::GetMousePositionLastFrame());

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
            Input::SetMouseMovement(V2(xPosRelative, yPosRelative));
        }
        break;
    }
    case WM_SIZE:
    {
        Vec2 newSize;
        newSize.Width = LOWORD(lParam);
        newSize.Height = HIWORD(lParam);
        Services::GetGraphics()->RegisterWindowSizeChange(newSize);
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
        Services::GetTree()->Quit();
    }
    break;
    default:
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

bool Input::IsKeyDown(uint8_t keyCode)
{
    return keys[keyCode] & InputState::DOWN;
}
