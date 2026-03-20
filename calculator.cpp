#include "calculator.h"
#include "Resource.h"
#include <stdexcept>

std::wstring const calculator::s_class_name{ L"window" };

bool calculator::register_class() {
	WNDCLASSEXW desc{};
	if (GetClassInfoExW(m_instance, s_class_name.c_str(), &desc) != 0) return true;
	desc.cbSize = sizeof(WNDCLASSEXW);
	desc.lpfnWndProc = window_proc_static;
	desc.hInstance = m_instance;
	desc.hIcon = LoadIconW(m_instance, MAKEINTRESOURCE(IDI_ICON2));
	desc.hIconSm = LoadCursorW(m_instance, MAKEINTRESOURCE(IDI_ICON2));
	desc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	desc.lpszClassName = s_class_name.c_str();
	desc.hbrBackground = CreateSolidBrush(RGB(100, 100, 100));
	return RegisterClassExW(&desc) != 0;
}

HWND calculator::create_window()
{
	return CreateWindowExW(
		0,
		s_class_name.c_str(),
		L"calculator",
		WS_SYSMENU | WS_CAPTION |
		WS_BORDER | WS_MINIMIZEBOX | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0,
		320, 400,
		nullptr,
		nullptr,
		m_instance,
		this);
}

LRESULT calculator::window_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	calculator* app = nullptr;
	if (message == WM_NCCREATE)
	{
		auto p = reinterpret_cast<LPCREATESTRUCTW>(lparam);
		app = static_cast<calculator*>(p->lpCreateParams);
		SetWindowLongPtrW(window, GWLP_USERDATA,
			reinterpret_cast<LONG_PTR>(app));
	}
	else
	{
		app = reinterpret_cast<calculator*>(
			GetWindowLongPtrW(window, GWLP_USERDATA));
	}
	if (app != nullptr)
	{
		return app->window_proc(window, message,
			wparam, lparam);
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT calculator::window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_CLOSE:
		DestroyWindow(window);
		return 0;
	case WM_DESTROY:
		if (window == m_main)
			PostQuitMessage(EXIT_SUCCESS);
		return 0;
	case WM_GETMINMAXINFO:
	{
		MINMAXINFO* mmi = (MINMAXINFO*)lparam;
		mmi->ptMinTrackSize.x = 320;
		mmi->ptMinTrackSize.y = 400;
		return 0;
	}
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

calculator::calculator(HINSTANCE instance) : m_instance{ instance }, m_main{} {
	register_class();
	m_main = create_window();
}

int calculator::run(int show_command)
{
	ShowWindow(m_main, show_command);
	MSG msg{};
	BOOL result = TRUE;
	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (result == -1) return EXIT_FAILURE;
		TranslateMessage(&msg);
		DispatchMessageW(&msg);
	}
	return EXIT_SUCCESS;
}

