#include "calculator.h"
#include "Resource.h"
#include <stdexcept>

enum ButtonIds { //identyfikatory przyciskow
	ID_BTN_7 = 2001,
	ID_BTN_8,
	ID_BTN_9,
	ID_BTN_DIV,
	ID_BTN_4,
	ID_BTN_5,
	ID_BTN_6,
	ID_BTN_MUL,
	ID_BTN_1,
	ID_BTN_2,
	ID_BTN_3,
	ID_BTN_SUB,
	ID_BTN_C,
	ID_BTN_0,
	ID_BTN_DOT,
	ID_BTN_ADD,
	ID_BTN_BS,
	ID_BTN_EQ
};

std::wstring const calculator::s_class_name{ L"window" };
std::wstring const calculator::s_display_class_name{ L"calculator_display" };

struct DisplayState {
	std::wstring history;
	std::wstring result;
	HFONT history_font;
	HFONT result_font;

	DisplayState() : history_font(nullptr), result_font(nullptr) {}
};

void update_display(HWND display, const std::wstring& history, const std::wstring& result);
double text_to_double(const std::wstring& text);
std::wstring double_to_text(double value);
double calculate(double left, double right, wchar_t op);

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

void calculator::create_buttons(HWND parent) {
	const wchar_t* labels[18] = { //tekst na przyciskach
		L"7", L"8", L"9", L"/",
		L"4", L"5", L"6", L"*",
		L"1", L"2", L"3", L"-",
		L"C", L"0", L".", L"+",
		L"BS", L"="
	};

	const int ids[18] = { //id przypisane do kazdego przycisku
		ID_BTN_7, ID_BTN_8, ID_BTN_9, ID_BTN_DIV,
		ID_BTN_4, ID_BTN_5, ID_BTN_6, ID_BTN_MUL,
		ID_BTN_1, ID_BTN_2, ID_BTN_3, ID_BTN_SUB,
		ID_BTN_C, ID_BTN_0, ID_BTN_DOT, ID_BTN_ADD,
		ID_BTN_BS, ID_BTN_EQ
	};

	for (int i = 0; i < 18; i++) { //tworzymy 18 przyciskow
		m_buttons[i] = CreateWindowW(
			L"BUTTON", //typ kontrolki
			labels[i], //tekst
			WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			0, 0, 0, 0,
			parent,
			reinterpret_cast<HMENU>(static_cast<INT_PTR>(ids[i])), //id
			m_instance,
			nullptr
		);
	}
}

void calculator::resize_children(int width, int height) {
	const int padding = 5; //odstep miedzy elementami
	//display zajmuje 1/4 wysokosci
	int display_x = padding;
	int display_y = padding;
	int display_w = width - 2 * padding;
	int display_h = height / 4;

	if (display_h < 60) display_h = 60;

	MoveWindow(m_display, display_x, display_y, display_w, display_h, TRUE);
	//obszar przyciskow
	int buttons_top = display_y + display_h + padding; //zaczynaja sie pod displayem
	int buttons_height = height - buttons_top - padding; //ile miejsca maja przyciski
	int buttons_width = width - 2 * padding;

	int rows = 5;
	int cols = 4;
	//rozmiary jednej komorki
	int cell_w = (buttons_width - (cols - 1) * padding) / cols;
	int cell_h = (buttons_height - (rows - 1) * padding) / rows;

	if (cell_w < 40) cell_w = 40;
	if (cell_h < 30) cell_h = 30;

	int index = 0;

	for (int row = 0; row < 4; row++) { //tworzymy 4 rzedy - 16 przyciskow
		for (int col = 0; col < 4; col++) {
			//pozycja przycisku
			int x = padding + col * (cell_w + padding);
			int y = buttons_top + row * (cell_h + padding);
			MoveWindow(m_buttons[index], x, y, cell_w, cell_h, TRUE); //ustawiamy pozycje i rozmiar
			index++;
		}
	}
	//ostatni rzad
	int last_row_y = buttons_top + 4 * (cell_h + padding);
	int bs_x = padding;
	int bs_y = last_row_y;
	int bs_w = cell_w;
	int bs_h = cell_h;

	MoveWindow(m_buttons[16], bs_x, bs_y, bs_w, bs_h, TRUE);

	int eq_x = padding + cell_w + padding;
	int eq_y = last_row_y;
	int eq_w = buttons_width - cell_w - padding;
	int eq_h = cell_h;

	MoveWindow(m_buttons[17], eq_x, eq_y, eq_w, eq_h, TRUE);
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
		create_buttons(window); //tworzymy przyciski
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
			m_stored_value = 0.0;
			m_pending_operator = 0;
			m_start_new_input = false;

			update_display(m_display, m_history_text, m_result_text);
			SetFocus(window);
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
			SetFocus(window);
			return 0;
		
		case ID_BTN_0:
		case ID_BTN_1:
		case ID_BTN_2:
		case ID_BTN_3:
		case ID_BTN_4:
		case ID_BTN_5:
		case ID_BTN_6:
		case ID_BTN_7:
		case ID_BTN_8:
		case ID_BTN_9:
		{
			wchar_t digit;
			switch (LOWORD(wparam)) {
			case ID_BTN_0: digit = L'0'; break;
			case ID_BTN_1: digit = L'1'; break;
			case ID_BTN_2: digit = L'2'; break;
			case ID_BTN_3: digit = L'3'; break;
			case ID_BTN_4: digit = L'4'; break;
			case ID_BTN_5: digit = L'5'; break;
			case ID_BTN_6: digit = L'6'; break;
			case ID_BTN_7: digit = L'7'; break;
			case ID_BTN_8: digit = L'8'; break;
			default:       digit = L'9'; break;
			}

			if (m_start_new_input || m_result_text == L"0") {
				m_result_text = std::wstring(1, digit);
				m_start_new_input = false;
			}
			else {
				m_result_text += digit;
			}

			update_display(m_display, m_history_text, m_result_text);
			SetFocus(window);
			return 0;
		}

		case ID_BTN_ADD:
		case ID_BTN_SUB:
		case ID_BTN_MUL:
		case ID_BTN_DIV:
		{
			wchar_t op;
			switch (LOWORD(wparam)) {
			case ID_BTN_ADD: op = L'+'; break;
			case ID_BTN_SUB: op = L'-'; break;
			case ID_BTN_MUL: op = L'*'; break;
			default:         op = L'/'; break;
			}

			if (m_pending_operator != 0 && !m_start_new_input) {
				double right = text_to_double(m_result_text);

				if (m_pending_operator == L'/' && right == 0.0) {
					MessageBoxW(window, L"Cannot divide by zero.", L"Error", MB_OK | MB_ICONERROR);
					SetFocus(window);
					return 0;
				}

				m_stored_value = calculate(m_stored_value, right, m_pending_operator);
				m_result_text = double_to_text(m_stored_value);
			}
			else {
				m_stored_value = text_to_double(m_result_text);
			}

			m_pending_operator = op;
			m_history_text = double_to_text(m_stored_value) + L" " + op;
			m_start_new_input = true;

			update_display(m_display, m_history_text, m_result_text);
			SetFocus(window);
			return 0;
		}

		case ID_BTN_DOT:
			if (m_start_new_input) {
				m_result_text = L"0.";
				m_start_new_input = false;
			}
			else if (m_result_text.find(L'.') == std::wstring::npos) {
				m_result_text += L'.';
			}

			update_display(m_display, m_history_text, m_result_text);
			SetFocus(window);
			return 0;

		case ID_BTN_C:
			m_history_text.clear();
			m_result_text = L"0";
			m_stored_value = 0.0;
			m_pending_operator = 0;
			m_start_new_input = false;
			update_display(m_display, m_history_text, m_result_text);
			SetFocus(window);
			return 0;

		case ID_BTN_BS:
			if (!m_result_text.empty())
				m_result_text.pop_back();

			if (m_result_text.empty())
				m_result_text = L"0";

			update_display(m_display, m_history_text, m_result_text);
			SetFocus(window);
			return 0;

		case ID_BTN_EQ:
		{
			if (m_pending_operator != 0) {
				double right = text_to_double(m_result_text);

				if (m_pending_operator == L'/' && right == 0.0) {
					MessageBoxW(window, L"Cannot divide by zero.", L"Error", MB_OK | MB_ICONERROR);
					SetFocus(window);
					return 0;
				}

				double result = calculate(m_stored_value, right, m_pending_operator);

				m_history_text = double_to_text(m_stored_value) + L" " + m_pending_operator + L" " + m_result_text + L" =";
				m_result_text = double_to_text(result);
				m_stored_value = result;
				m_pending_operator = 0;
				m_start_new_input = true;
			}

			update_display(m_display, m_history_text, m_result_text);
			SetFocus(window);
			return 0;
		}
		}
		return 0;
	}
	case WM_CHAR:
	{
		wchar_t ch = (wchar_t)wparam;

		if (ch >= L'0' && ch <= L'9') {
			if (m_start_new_input || m_result_text == L"0") {
				m_result_text = std::wstring(1, ch);
				m_start_new_input = false;
			}
			else {
				m_result_text += ch;
			}
		}

		else if (ch == L'.') {
			if (m_start_new_input) {
				m_result_text = L"0.";
				m_start_new_input = false;
			}
			else if (m_result_text.find(L'.') == std::wstring::npos) {
				m_result_text += L'.';
			}
		}

		else if (ch == L'+' || ch == L'-' || ch == L'*' || ch == L'/') {
			if (m_pending_operator != 0 && !m_start_new_input) {
				double right = text_to_double(m_result_text);

				if (m_pending_operator == L'/' && right == 0.0) {
					MessageBoxW(window, L"Cannot divide by zero.", L"Error", MB_OK | MB_ICONERROR);
					return 0;
				}

				m_stored_value = calculate(m_stored_value, right, m_pending_operator);
				m_result_text = double_to_text(m_stored_value);
			}
			else {
				m_stored_value = text_to_double(m_result_text);
			}

			m_pending_operator = ch;
			m_history_text = double_to_text(m_stored_value) + L" " + ch;
			m_start_new_input = true;
		}

		update_display(m_display, m_history_text, m_result_text);
		return 0;
	}
	case WM_KEYDOWN:
	{
		switch (wparam)
		{
		case VK_RETURN:
		{
			if (m_pending_operator != 0) {
				double right = text_to_double(m_result_text);

				if (m_pending_operator == L'/' && right == 0.0) {
					MessageBoxW(window, L"Cannot divide by zero.", L"Error", MB_OK | MB_ICONERROR);
					return 0;
				}

				double result = calculate(m_stored_value, right, m_pending_operator);

				m_history_text = double_to_text(m_stored_value) + L" " + m_pending_operator + L" " + m_result_text + L" =";
				m_result_text = double_to_text(result);
				m_stored_value = result;
				m_pending_operator = 0;
				m_start_new_input = true;
			}

			update_display(m_display, m_history_text, m_result_text);
			return 0;
		}

		case VK_BACK:
			if (!m_result_text.empty())
				m_result_text.pop_back();

			if (m_result_text.empty())
				m_result_text = L"0";

			update_display(m_display, m_history_text, m_result_text);
			return 0;

		case VK_ESCAPE:
			m_history_text.clear();
			m_result_text = L"0";
			m_stored_value = 0.0;
			m_pending_operator = 0;
			m_start_new_input = false;
			update_display(m_display, m_history_text, m_result_text);
			return 0;
		}
		break;
	}
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

calculator::calculator(HINSTANCE instance)
	: m_instance{ instance }, 
	m_main{}, 
	m_display{},
	m_accel{},
	m_history_text{ L"" },
	m_result_text{ L"0" },
	m_stored_value{ 0.0 },
	m_pending_operator{ 0 },
	m_start_new_input{ false }
{
	for (int i = 0; i < 18; i++) {
		m_buttons[i] = nullptr;
	}
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

void update_display(HWND display, const std::wstring& history, const std::wstring& result) {
	auto* state = reinterpret_cast<DisplayState*>(GetWindowLongPtrW(display, GWLP_USERDATA));
	if (state) {
		state->history = history;
		state->result = result;
	}
	InvalidateRect(display, nullptr, TRUE);
}

double text_to_double(const std::wstring& text) {
	return std::wcstod(text.c_str(), nullptr);
}

std::wstring double_to_text(double value) {
	std::wstring text = std::to_wstring(value);

	while (!text.empty() && text.back() == L'0')
		text.pop_back();

	if (!text.empty() && text.back() == L'.')
		text.pop_back();

	if (text.empty())
		text = L"0";

	return text;
}

double calculate(double left, double right, wchar_t op) {
	switch (op) {
	case L'+': return left + right;
	case L'-': return left - right;
	case L'*': return left * right;
	case L'/': return left / right;
	default:   return right;
	}
}