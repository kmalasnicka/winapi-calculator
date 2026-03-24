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
	desc.hIcon = LoadIconW(m_instance, MAKEINTRESOURCE(IDI_CALC));
	desc.hIconSm = LoadIconW(m_instance, MAKEINTRESOURCE(IDI_CALC));
	desc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	desc.lpszClassName = s_class_name.c_str();
	desc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); //bialy kolor
	return RegisterClassExW(&desc) != 0;
}

HWND calculator::create_window()
{
	return CreateWindowExW(
		0,
		s_class_name.c_str(),
		L"calculator",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, 0,
		400, 500,
		nullptr,
		LoadMenuW(m_instance, MAKEINTRESOURCEW(IDC_LABY32)), //podpiecie menu do okna
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
	case WM_COMMAND: //obsluga komend menu
	{
		switch (LOWORD(wparam))
		{
		case IDM_EXIT:
			DestroyWindow(window);
			return 0;

		case IDM_ABOUT:
			MessageBoxW(window, L"DevCalculator", L"About", MB_OK | MB_ICONINFORMATION);
			return 0;

		case IDM_EDIT_CLEAR:
			MessageBoxW(window, L"Clear", L"Edit", MB_OK);
			return 0;

		case IDM_MODE_BASIC:
			CheckMenuRadioItem(
				GetMenu(window),
				IDM_MODE_BASIC,
				IDM_MODE_PROGRAMMER,
				IDM_MODE_BASIC,
				MF_BYCOMMAND);
			return 0;

		case IDM_MODE_PROGRAMMER:
			MessageBoxW(window, L"Not implemented yet", L"Programmer mode", MB_OK | MB_ICONINFORMATION);
			return 0;
		}
		return 0;
	}
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

calculator::calculator(HINSTANCE instance) : m_instance{ instance }, m_main{}, m_accel{} {
	register_class();
	m_main = create_window();
	m_accel = LoadAcceleratorsW(m_instance, MAKEINTRESOURCEW(IDC_LABY32)); //ladowanie tabeli akceleratorow
}

int calculator::run(int show_command)
{
	ShowWindow(m_main, show_command);
	MSG msg{};
	BOOL result = TRUE;
	while ((result = GetMessageW(&msg, nullptr, 0, 0)) != 0)
	{
		if (result == -1) return EXIT_FAILURE;
		if (!TranslateAcceleratorW(m_main, m_accel, &msg)) {
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}
	return EXIT_SUCCESS;
}

