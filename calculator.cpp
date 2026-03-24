#include "calculator.h"
#include "Resource.h"
#include <stdexcept>

std::wstring const calculator::s_class_name{ L"window" };
std::wstring const calculator::s_display_class_name{ L"calculator_display" };

struct DisplayState {
	std::wstring history;
	std::wstring result;
	HFONT history_font;
	HFONT result_font;

	DisplayState() : history_font(nullptr), result_font(nullptr) {}
};

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
	desc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
	return RegisterClassExW(&desc) != 0;
}

bool calculator::register_display_class() {
	WNDCLASSEXW desc{};
	if (GetClassInfoExW(m_instance, s_display_class_name.c_str(), &desc) != 0) return true;
	desc.cbSize = sizeof(WNDCLASSEXW);
	desc.style = CS_HREDRAW | CS_VREDRAW;
	desc.lpfnWndProc = display_proc_static;
	desc.hInstance = m_instance;
	desc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	desc.lpszClassName = s_display_class_name.c_str();
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

void calculator::create_display(HWND parent) {
	m_display = CreateWindowExW(
		WS_EX_CLIENTEDGE,
		s_display_class_name.c_str(),
		nullptr,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		parent,
		nullptr,
		m_instance,
		nullptr
	);
	auto* state = new DisplayState();
	state->history = m_history_text;
	state->result = m_result_text;
	SetWindowLongPtrW(m_display, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(state));
}

void calculator::resize_children(int width, int height) {
	const int padding = 5;
	int display_x = padding;
	int display_y = padding;
	int display_w = width - 2 * padding;
	int display_h = height / 4 - padding * 2;
	if (display_h < 60) display_h = 60;
	MoveWindow(m_display, display_x, display_y, display_w, display_h, TRUE);
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

LRESULT CALLBACK calculator::display_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	auto* state = reinterpret_cast<DisplayState*>(GetWindowLongPtrW(window, GWLP_USERDATA));

	switch (message) {
	case WM_SIZE:
	{
		if (!state) return 0;

		int width = LOWORD(lparam);
		int height = HIWORD(lparam);

		if (state->history_font) {
			DeleteObject(state->history_font);
			state->history_font = nullptr;
		}

		if (state->result_font) {
			DeleteObject(state->result_font);
			state->result_font = nullptr;
		}

		int history_font_height = height / 6;
		int result_font_height = height / 3;

		if (history_font_height < 14) history_font_height = 14;
		if (result_font_height < 28) result_font_height = 28;

		state->history_font = CreateFontW(
			-history_font_height, 0, 0, 0,
			FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

		state->result_font = CreateFontW(
			-result_font_height, 0, 0, 0,
			FW_BOLD, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

		InvalidateRect(window, nullptr, TRUE);
		return 0;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps{};
		HDC hdc = BeginPaint(window, &ps);

		RECT rect{};
		GetClientRect(window, &rect);

		HBRUSH brush = (HBRUSH)(COLOR_WINDOW + 1);
		FillRect(hdc, &rect, brush);

		SetBkMode(hdc, TRANSPARENT);

		RECT history_rect = rect;
		history_rect.left += 10;
		history_rect.right -= 10;
		history_rect.top += 8;
		history_rect.bottom = rect.top + (rect.bottom - rect.top) / 2;

		RECT result_rect = rect;
		result_rect.left += 10;
		result_rect.right -= 10;
		result_rect.top = history_rect.bottom;
		result_rect.bottom -= 8;

		if (state && state->history_font) {
			SelectObject(hdc, state->history_font);
			SetTextColor(hdc, RGB(120, 120, 120));
			DrawTextW(
				hdc,
				state->history.c_str(),
				-1,
				&history_rect,
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}

		if (state && state->result_font) {
			SelectObject(hdc, state->result_font);
			SetTextColor(hdc, RGB(0, 0, 0));
			DrawTextW(
				hdc,
				state->result.c_str(),
				-1,
				&result_rect,
				DT_RIGHT | DT_VCENTER | DT_SINGLELINE);
		}

		EndPaint(window, &ps);
		return 0;
	}

	case WM_DESTROY:
	{
		if (state) {
			if (state->history_font) DeleteObject(state->history_font);
			if (state->result_font) DeleteObject(state->result_font);
			delete state;
			SetWindowLongPtrW(window, GWLP_USERDATA, 0);
		}
		return 0;
	}
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT calculator::window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	switch (message) {
	case WM_CREATE:
		m_main = window;
		create_display(window);
		return 0;
	case WM_SIZE:
	{
		int width = LOWORD(lparam);
		int height = HIWORD(lparam);
		resize_children(width, height);
		return 0;
	}
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
		{
			m_history_text.clear();
			m_result_text = L"0";

			auto* state = reinterpret_cast<DisplayState*>(GetWindowLongPtrW(m_display, GWLP_USERDATA));
			if (state) {
				state->history = m_history_text;
				state->result = m_result_text;
			}

			InvalidateRect(m_display, nullptr, TRUE);
			return 0;
		}

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

calculator::calculator(HINSTANCE instance)
	: m_instance{ instance }, 
	m_main{}, 
	m_display{},
	m_accel{},
	m_history_text{ L"12 + 7 =" },
	m_result_text{ L"19" }
{
	register_class();
	register_display_class();
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

