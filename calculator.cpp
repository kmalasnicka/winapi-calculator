#include "calculator.h"
#include "Resource.h"
#include <windowsx.h>
#include <stdexcept>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>
#include <cwctype>

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
std::wstring const calculator::s_bits_class_name{ L"calculator_bits" };

struct DisplayState {
	std::wstring history;
	std::wstring result;
	HFONT history_font;
	HFONT result_font;
	std::wstring warning;
	HFONT warning_font;

	DisplayState() : history_font(nullptr), result_font(nullptr), warning_font(nullptr) {}
};

void update_display(HWND display, const std::wstring& history, const std::wstring& result, const std::wstring& warning);
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
		L"DevCalculator",
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
	state->warning = m_precision_warning;
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
	const int padding = 5;

	int display_x = padding;
	int display_y = padding;
	int display_w = width - 2 * padding;
	int display_h = height / 4;
	if (display_h < 60) display_h = 60;

	MoveWindow(m_display, display_x, display_y, display_w, display_h, TRUE);

	int top_after_display = display_y + display_h + padding;

	int buttons_top = top_after_display;
	if (m_mode == CalcMode::Programmer) {
		int bits_h = 70;
		int combo_visible_h = 28;
		int combo_total_h = 200;
		int combo_gap = 5;

		ShowWindow(m_bits_display, SW_SHOW);
		ShowWindow(m_type_combo, SW_SHOW);
		ShowWindow(m_base_combo, SW_SHOW);

		MoveWindow(m_bits_display, padding, top_after_display, width - 2 * padding, bits_h, TRUE);

		int combo_y = top_after_display + bits_h + padding;
		int combo_w = (width - 3 * padding) / 2;

		MoveWindow(m_type_combo, padding, combo_y, combo_w, combo_total_h, TRUE);
		MoveWindow(m_base_combo, padding + combo_w + padding, combo_y, combo_w, combo_total_h, TRUE);

		buttons_top = combo_y + combo_visible_h + padding;
	}
	else {
		ShowWindow(m_bits_display, SW_HIDE);
		ShowWindow(m_type_combo, SW_HIDE);
		ShowWindow(m_base_combo, SW_HIDE);
	}

	int buttons_height = height - buttons_top - padding;
	int buttons_width = width - 2 * padding;

	int rows = 5;
	int cols = 4;

	int cell_w = (buttons_width - (cols - 1) * padding) / cols;
	int cell_h = (buttons_height - (rows - 1) * padding) / rows;

	if (cell_w < 40) cell_w = 40;
	if (cell_h < 30) cell_h = 30;

	int index = 0;
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			int x = padding + col * (cell_w + padding);
			int y = buttons_top + row * (cell_h + padding);
			MoveWindow(m_buttons[index], x, y, cell_w, cell_h, TRUE);
			index++;
		}
	}

	int last_row_y = buttons_top + 4 * (cell_h + padding);
	MoveWindow(m_buttons[16], padding, last_row_y, cell_w, cell_h, TRUE);
	MoveWindow(m_buttons[17], padding + cell_w + padding, last_row_y, buttons_width - cell_w - padding, cell_h, TRUE);
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

		if (state->warning_font) {
			DeleteObject(state->warning_font);
			state->warning_font = nullptr;
		}

		int warning_font_height = height / 12;
		if (warning_font_height < 9) warning_font_height = 9;

		state->warning_font = CreateFontW(
			-warning_font_height, 0, 0, 0,
			FW_NORMAL, FALSE, FALSE, FALSE,
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

		RECT warning_rect = rect;
		warning_rect.left += 10;
		warning_rect.right -= 10;
		warning_rect.bottom -= 4;
		warning_rect.top = rect.bottom - (rect.bottom - rect.top) / 4;

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

		if (state && state->warning_font && !state->warning.empty()) {
			SelectObject(hdc, state->warning_font);
			SetTextColor(hdc, RGB(200, 0, 0));
			DrawTextW(
				hdc,
				state->warning.c_str(),
				-1,
				&warning_rect,
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
			if (state->warning_font) DeleteObject(state->warning_font);
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
		create_programmer_controls(window);
		create_buttons(window); //tworzymy przyciski
		create_bit_hint(window);
		sync_programmer_ui();
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
	case WM_CTLCOLORSTATIC:
	{
		HDC hdc = reinterpret_cast<HDC>(wparam);
		HWND ctrl = reinterpret_cast<HWND>(lparam);

		if (ctrl == m_bit_hint) {
			SetTextColor(hdc, RGB(0, 0, 0));
			SetBkColor(hdc, RGB(255, 255, 210));
			static HBRUSH hintBrush = CreateSolidBrush(RGB(255, 255, 210));
			return reinterpret_cast<LRESULT>(hintBrush);
		}
		break;
	}
	case WM_COMMAND: //obsluga komend menu
	{
		if (HIWORD(wparam) == CBN_SELCHANGE && LOWORD(wparam) == IDC_BASE_COMBO) {
			int sel = static_cast<int>(SendMessageW(m_base_combo, CB_GETCURSEL, 0, 0));
			if (sel >= 0) {
				m_number_base = static_cast<NumberBase>(sel);
				m_result_text = format_value_by_base();
				update_all_displays();
			}
			return 0;
		}

		if (HIWORD(wparam) == CBN_SELCHANGE && LOWORD(wparam) == IDC_TYPE_COMBO) {
			int sel = static_cast<int>(SendMessageW(m_type_combo, CB_GETCURSEL, 0, 0));
			if (sel >= 0) {
				m_data_type = static_cast<DataType>(sel);

				if (is_float_type()) {
					m_number_base = NumberBase::Dec;
					SendMessageW(m_base_combo, CB_SETCURSEL, static_cast<WPARAM>(0), 0);
				}

				sync_bits_from_result_text();
				m_result_text = format_value_by_base();
				update_all_displays();
			}
			return 0;
		}

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

			update_all_displays();
			SetFocus(window);
			return 0;
		}
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

			if (!is_float_type() && !is_input_char_allowed(digit)) {
				SetFocus(window);
				return 0;
			}

			append_digit_to_result(digit);
			update_all_displays();
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

			update_all_displays();
			SetFocus(window);
			return 0;
		}

		case ID_BTN_DOT:
			if (!is_float_type()) {
				SetFocus(window);
				return 0;
			}

			if (m_start_new_input) {
				m_result_text = L"0.";
				m_start_new_input = false;
			}
			else if (m_result_text.find(L'.') == std::wstring::npos) {
				m_result_text += L'.';
			}

			update_all_displays();
			SetFocus(window);
			return 0;

		case ID_BTN_C:
			m_history_text.clear();
			m_result_text = L"0";
			m_stored_value = 0.0;
			m_pending_operator = 0;
			m_start_new_input = false;
			update_all_displays();
			SetFocus(window);
			return 0;

		case ID_BTN_BS:
			if (!m_result_text.empty())
				m_result_text.pop_back();

			if (m_result_text.empty())
				m_result_text = L"0";

			update_all_displays();
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

			update_all_displays();
			SetFocus(window);
			return 0;
		}
		case IDM_MODE_BASIC:
			set_mode(CalcMode::Basic);
			SetFocus(window);
			return 0;

		case IDM_MODE_PROGRAMMER:
			set_mode(CalcMode::Programmer);
			SetFocus(window);
			return 0;
		}
		return 0;
	}
	case WM_CHAR:
	{
		wchar_t ch = (wchar_t)wparam;

		if (!is_float_type()) {
			if (!is_input_char_allowed(ch))
				return 0;
		}

		if (ch >= L'0' && ch <= L'9') {
			append_digit_to_result(ch);
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

		else if ((ch >= L'A' && ch <= L'F') || (ch >= L'a' && ch <= L'f')) {
			ch = towupper(ch);
			append_digit_to_result(ch);
		}

		update_all_displays();
		return 0;
	}
	case WM_KEYDOWN:
	{
		switch (wparam)
		{;
		case 'C':
			if (GetKeyState(VK_CONTROL) < 0) {
				copy_text_to_clipboard(m_result_text);
				return 0;
			}
			break;

		case 'V':
			if (GetKeyState(VK_CONTROL) < 0) {
				std::wstring pasted;
				if (paste_text_from_clipboard(pasted) && is_valid_pasted_text(pasted)) {
					if ((m_number_base == NumberBase::Hex) && !pasted.empty()) {
						for (wchar_t& ch : pasted) {
							if (ch >= L'a' && ch <= L'f')
								ch = towupper(ch);
						}
					}
					m_result_text = pasted;
					m_start_new_input = false;
					update_all_displays();
				}
				else {
					MessageBoxW(window, L"Clipboard does not contain a valid numeric value.", L"Paste Error", MB_OK | MB_ICONWARNING);
				}
				return 0;
			}
			break;

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

			update_all_displays();
			return 0;
		}

		case VK_BACK:
			if (!m_result_text.empty())
				m_result_text.pop_back();

			if (m_result_text.empty())
				m_result_text = L"0";

			update_all_displays();
			return 0;

		case VK_ESCAPE:
			m_history_text.clear();
			m_result_text = L"0";
			m_stored_value = 0.0;
			m_pending_operator = 0;
			m_start_new_input = false;
			update_all_displays();
			return 0;
		}
		break;
	}
	}
	return DefWindowProcW(window, message, wparam, lparam);
}

calculator::calculator(HINSTANCE instance)
	:m_instance{ instance },
	m_main{},
	m_display{},
	m_bits_display{},
	m_type_combo{},
	m_accel{},
	m_history_text{ L"" },
	m_result_text{ L"0" },
	m_stored_value{ 0.0 },
	m_pending_operator{ 0 },
	m_start_new_input{ false },
	m_mode{ CalcMode::Basic },
	m_data_type{ DataType::Int32 },
	m_bits{ 0 },
	m_hover_bit_index{ -1 },
	m_bit_hint{},
	m_base_combo{},
	m_number_base{ NumberBase::Dec },
	m_precision_warning{ L"" }
{
	for (int i = 0; i < 18; i++) {
		m_buttons[i] = nullptr;
	}
	register_class();
	register_display_class();
	register_bits_class();

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

void update_display(HWND display, const std::wstring& history, const std::wstring& result, const std::wstring& warning) {
	auto* state = reinterpret_cast<DisplayState*>(GetWindowLongPtrW(display, GWLP_USERDATA));
	if (state) {
		state->history = history;
		state->result = result;
		state->warning = warning;
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

bool calculator::register_bits_class() {
	WNDCLASSEXW desc{};
	if (GetClassInfoExW(m_instance, s_bits_class_name.c_str(), &desc) != 0) return true;

	desc.cbSize = sizeof(WNDCLASSEXW);
	desc.style = CS_HREDRAW | CS_VREDRAW;
	desc.lpfnWndProc = bits_proc_static;
	desc.hInstance = m_instance;
	desc.hCursor = LoadCursorW(nullptr, IDC_ARROW);
	desc.lpszClassName = s_bits_class_name.c_str();
	desc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

	return RegisterClassExW(&desc) != 0;
}

void calculator::create_programmer_controls(HWND parent) {
	m_bits_display = CreateWindowExW(
		WS_EX_CLIENTEDGE,
		s_bits_class_name.c_str(),
		nullptr,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0,
		parent,
		reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BITS_VIEW)),
		m_instance,
		nullptr
	);

	m_type_combo = CreateWindowExW(
		0,
		L"COMBOBOX",
		nullptr,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		0, 0, 0, 0,
		parent,
		reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_TYPE_COMBO)),
		m_instance,
		nullptr
	);

	m_base_combo = CreateWindowExW(
		0,
		L"COMBOBOX",
		nullptr,
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		0, 0, 0, 0,
		parent,
		reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDC_BASE_COMBO)),
		m_instance,
		nullptr
	);

	populate_base_combo();

	populate_type_combo();
}

void calculator::populate_type_combo() {
	const wchar_t* items[] = {
		L"Int 8",
		L"UInt 8",
		L"Int 16",
		L"UInt 16",
		L"Int 32",
		L"UInt 32",
		L"Int 64",
		L"UInt 64",
		L"Float 16",
		L"Float 32",
		L"Float 64"
	};

	for (const auto* item : items) {
		SendMessageW(m_type_combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item));
	}

	SendMessageW(m_type_combo, CB_SETCURSEL, static_cast<WPARAM>(m_data_type), 0);
}

void calculator::set_mode(CalcMode mode) {
	m_mode = mode;

	HMENU menu = GetMenu(m_main);
	CheckMenuRadioItem(
		menu,
		IDM_MODE_BASIC,
		IDM_MODE_PROGRAMMER,
		mode == CalcMode::Basic ? IDM_MODE_BASIC : IDM_MODE_PROGRAMMER,
		MF_BYCOMMAND
	);

	sync_programmer_ui();

	RECT rect{};
	GetClientRect(m_main, &rect);
	resize_children(rect.right - rect.left, rect.bottom - rect.top);

	InvalidateRect(m_main, nullptr, TRUE);
}

void calculator::sync_programmer_ui() {
	BOOL show = (m_mode == CalcMode::Programmer) ? TRUE : FALSE;
	ShowWindow(m_bits_display, show ? SW_SHOW : SW_HIDE);
	ShowWindow(m_type_combo, show ? SW_SHOW : SW_HIDE);
	ShowWindow(m_base_combo, show ? SW_SHOW : SW_HIDE);
}

std::wstring calculator::get_display_text() const {
	switch (m_data_type) {
	case DataType::Int8:
		return std::to_wstring(static_cast<int8_t>(m_bits));
	case DataType::UInt8:
		return std::to_wstring(static_cast<uint8_t>(m_bits));
	case DataType::Int16:
		return std::to_wstring(static_cast<int16_t>(m_bits));
	case DataType::UInt16:
		return std::to_wstring(static_cast<uint16_t>(m_bits));
	case DataType::Int32:
		return std::to_wstring(static_cast<int32_t>(m_bits));
	case DataType::UInt32:
		return std::to_wstring(static_cast<uint32_t>(m_bits));
	case DataType::Int64:
		return std::to_wstring(static_cast<int64_t>(m_bits));
	case DataType::UInt64:
		return std::to_wstring(static_cast<uint64_t>(m_bits));

	case DataType::Float32:
	{
		uint32_t raw = static_cast<uint32_t>(m_bits);
		float f = 0.0f;
		std::memcpy(&f, &raw, sizeof(f));
		return double_to_text(f);
	}

	case DataType::Float64:
	{
		uint64_t raw = m_bits;
		double d = 0.0;
		std::memcpy(&d, &raw, sizeof(d));
		return double_to_text(d);
	}

	case DataType::Half:
		return L"0";
	}

	return L"0";
}

void calculator::update_all_displays() {
	sync_bits_from_result_text();
	update_precision_warning();
	update_display(m_display, m_history_text, m_result_text, m_precision_warning);
	InvalidateRect(m_bits_display, nullptr, TRUE);
}

LRESULT CALLBACK calculator::bits_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
	calculator* app = reinterpret_cast<calculator*>(GetWindowLongPtrW(GetParent(window), GWLP_USERDATA));
	if (!app) return DefWindowProcW(window, message, wparam, lparam);

	switch (message) {
	case WM_ERASEBKGND:
		return 1;

	case WM_MOUSEMOVE:
	{
		RECT rect{};
		GetClientRect(window, &rect);

		int visible_bits = app->get_visible_bit_count();
		int total_w = rect.right - rect.left;
		int box_w =total_w / visible_bits;
		if (box_w < 6) box_w = 6;

		int x = GET_X_LPARAM(lparam);
		int bit_index = x / box_w;
		if (bit_index < 0 || bit_index >= visible_bits)
			bit_index = -1;

		if (app->m_hover_bit_index != bit_index) {
			app->m_hover_bit_index = bit_index;
			InvalidateRect(window, nullptr, TRUE);
		}

		TRACKMOUSEEVENT tme{};
		tme.cbSize = sizeof(tme);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = window;
		TrackMouseEvent(&tme);

		if (bit_index >= 0) {
			POINT pt{ GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
			ClientToScreen(window, &pt);
			app->update_tooltip_for_bit(bit_index, pt);
		}

		return 0;
	}

	case WM_MOUSELEAVE:
	{
		app->m_hover_bit_index = -1;
		InvalidateRect(window, nullptr, TRUE);
		app->hide_bit_hint();
		return 0;
	}

	case WM_LBUTTONDOWN:
	{
		RECT rect{};
		GetClientRect(window, &rect);

		int visible_bits = app->get_visible_bit_count();
		int total_w = rect.right - rect.left;
		int box_w = total_w / visible_bits;
		if (box_w < 6) box_w = 6;

		int x = GET_X_LPARAM(lparam);
		int bit_index = x / box_w;

		if (bit_index >= 0 && bit_index < visible_bits) {
			app->toggle_bit(bit_index);
		}
		app->hide_bit_hint();
		return 0;
	}

	case WM_PAINT:
	{
		PAINTSTRUCT ps{};
		HDC hdc = BeginPaint(window, &ps);

		RECT rect{};
		GetClientRect(window, &rect);

		int width = rect.right - rect.left;
		int height = rect.bottom - rect.top;

		HDC memdc = CreateCompatibleDC(hdc);
		HBITMAP membmp = CreateCompatibleBitmap(hdc, width, height);
		HBITMAP oldbmp = (HBITMAP)SelectObject(memdc, membmp);

		FillRect(memdc, &rect, (HBRUSH)(COLOR_WINDOW + 1));
		SetBkMode(memdc, TRANSPARENT);

		int visible_bits = app->get_visible_bit_count();
		int box_w = width / visible_bits;
		if (box_w < 6) box_w = 6;

		int exp_bits = app->get_exponent_bits();
		int mant_bits = app->get_mantissa_bits();

		int font_h = box_w - 3;
		if (font_h < 6) font_h = 6;
		if (font_h > 18) font_h = 18;
		HFONT font = CreateFontW(
			-font_h, 0, 0, 0,
			FW_NORMAL, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_SWISS, L"Segoe UI"
		);
		HFONT oldfont = (HFONT)SelectObject(memdc, font);

		for (int i = 0; i < visible_bits; i++) {
			int shift = visible_bits - 1 - i;
			bool bit = ((app->m_bits >> shift) & 1ULL) != 0;

			RECT box{
				rect.left + i * box_w,
				rect.top,
				min(rect.left + (i + 1) * box_w, rect.right),
				rect.bottom
			};

			COLORREF bg = RGB(255, 255, 255);
			COLORREF fg = RGB(0, 0, 0);

			if (app->is_float_type()) {
				if (i == 0) {
					bg = RGB(255, 230, 230);
					fg = RGB(180, 0, 0);
				}
				else if (i <= exp_bits) {
					bg = RGB(230, 255, 230);
					fg = RGB(0, 140, 0);
				}
				else {
					bg = RGB(230, 240, 255);
					fg = RGB(0, 70, 180);
				}
			}

			if (i == app->m_hover_bit_index) {
				bg = RGB(
					min(255, GetRValue(bg) - 20),
					min(255, GetGValue(bg) - 20),
					min(255, GetBValue(bg) - 20)
				);
			}

			HBRUSH boxBrush = CreateSolidBrush(bg);
			FillRect(memdc, &box, boxBrush);
			DeleteObject(boxBrush);

			DrawEdge(memdc, &box, BDR_SUNKENOUTER, BF_RECT);

			SetTextColor(memdc, fg);
			const wchar_t* txt = bit ? L"1" : L"0";
			DrawTextW(memdc, txt, -1, &box, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
		}

		BitBlt(hdc, 0, 0, width, height, memdc, 0, 0, SRCCOPY);

		SelectObject(memdc, oldfont);
		DeleteObject(font);

		SelectObject(memdc, oldbmp);
		DeleteObject(membmp);
		DeleteDC(memdc);

		EndPaint(window, &ps);
		return 0;
	}
	}

	return DefWindowProcW(window, message, wparam, lparam);
}

bool calculator::is_float_type() const {
	return m_data_type == DataType::Half || m_data_type == DataType::Float32 || m_data_type == DataType::Float64;
}

int calculator::get_visible_bit_count() const {
	switch (m_data_type) {
	case DataType::Int8:
	case DataType::UInt8:
		return 8;
	case DataType::Int16:
	case DataType::UInt16:
	case DataType::Half:
		return 16;
	case DataType::Int32:
	case DataType::UInt32:
	case DataType::Float32:
		return 32;
	case DataType::Int64:
	case DataType::UInt64:
	case DataType::Float64:
		return 64;
	}
	return 32;
}

int calculator::get_exponent_bits() const {
	switch (m_data_type) {
	case DataType::Half: return 5;
	case DataType::Float32: return 8;
	case DataType::Float64: return 11;
	default: return 0;
	}
}

int calculator::get_mantissa_bits() const {
	switch (m_data_type) {
	case DataType::Half: return 10;
	case DataType::Float32: return 23;
	case DataType::Float64: return 52;
	default: return 0;
	}
}

void calculator::toggle_bit(int bit_index) { //klikanie bitow
	int visible_bits = get_visible_bit_count();
	int shift = visible_bits - 1 - bit_index;
	if (shift < 0 || shift >= 64) return;

	m_bits ^= (1ULL << shift);

	if (m_mode == CalcMode::Programmer) {
		m_result_text = format_value_by_base();
		update_all_displays();
	}
}

//tooltip dla konkretnego bitu
void calculator::update_tooltip_for_bit(int bit_index, POINT screen_pt) {
	if (bit_index < 0) return;

	wchar_t tip[256];

	int visible_bits = get_visible_bit_count();
	int logical_bit = visible_bits - 1 - bit_index;

	if (!is_float_type()) {
		swprintf_s(tip, L"Bit %d, weight: 2^%d", logical_bit, logical_bit);
	}
	else {
		int exp_bits = get_exponent_bits();

		if (bit_index == 0) {
			swprintf_s(tip, L"Sign bit");
		}
		else if (bit_index <= exp_bits) {
			int exp_pos = exp_bits - bit_index;
			int bias = (1 << (exp_bits - 1)) - 1;
			swprintf_s(tip, L"Exponent bit, weight: 2^%d (Bias: %d)", exp_pos, bias);
		}
		else {
			int mant_index = bit_index - 1 - exp_bits;
			swprintf_s(tip, L"Mantissa bit, weight: 2^-%d", mant_index + 1);
		}
	}

	show_bit_hint(tip, screen_pt);
}

void calculator::sync_bits_from_result_text() {
	if (m_result_text.empty() || m_result_text == L"-" || m_result_text == L"." || m_result_text == L"-.")
		return;

	std::wstring text = m_result_text;
	int base = 10;

	if (!is_float_type()) {
		switch (m_number_base) {
		case NumberBase::Dec:
			base = 10;
			break;
		case NumberBase::Hex:
			base = 16;
			if (text.rfind(L"0x", 0) == 0 || text.rfind(L"0X", 0) == 0)
				text = text.substr(2);
			break;
		case NumberBase::Oct:
			base = 8;
			if (!text.empty() && (text[0] == L'o' || text[0] == L'O'))
				text = text.substr(1);
			break;
		case NumberBase::Bin:
			base = 2;
			if (!text.empty() && (text[0] == L'b' || text[0] == L'B'))
				text = text.substr(1);
			break;
		}
	}

	switch (m_data_type) {
	case DataType::Int8:
		m_bits = static_cast<uint8_t>(static_cast<int8_t>(std::wcstoll(text.c_str(), nullptr, base)));
		break;
	case DataType::UInt8:
		m_bits = static_cast<uint8_t>(std::wcstoull(text.c_str(), nullptr, base));
		break;
	case DataType::Int16:
		m_bits = static_cast<uint16_t>(static_cast<int16_t>(std::wcstoll(text.c_str(), nullptr, base)));
		break;
	case DataType::UInt16:
		m_bits = static_cast<uint16_t>(std::wcstoull(text.c_str(), nullptr, base));
		break;
	case DataType::Int32:
		m_bits = static_cast<uint32_t>(static_cast<int32_t>(std::wcstoll(text.c_str(), nullptr, base)));
		break;
	case DataType::UInt32:
		m_bits = static_cast<uint32_t>(std::wcstoull(text.c_str(), nullptr, base));
		break;
	case DataType::Int64:
		m_bits = static_cast<uint64_t>(static_cast<int64_t>(std::wcstoll(text.c_str(), nullptr, base)));
		break;
	case DataType::UInt64:
		m_bits = static_cast<uint64_t>(std::wcstoull(text.c_str(), nullptr, base));
		break;

	case DataType::Float32:
	{
		float f = static_cast<float>(std::wcstod(m_result_text.c_str(), nullptr));
		uint32_t raw = 0;
		std::memcpy(&raw, &f, sizeof(f));
		m_bits = raw;
		break;
	}

	case DataType::Float64:
	{
		double d = std::wcstod(m_result_text.c_str(), nullptr);
		uint64_t raw = 0;
		std::memcpy(&raw, &d, sizeof(d));
		m_bits = raw;
		break;
	}

	case DataType::Half:
		m_bits = 0;
		break;
	}
}

void calculator::create_bit_hint(HWND parent) {
	m_bit_hint = CreateWindowExW(
		WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
		L"STATIC",
		L"",
		WS_POPUP | SS_LEFT,
		0, 0, 0, 0,
		parent,
		nullptr,
		m_instance,
		nullptr
	);

	SendMessageW(m_bit_hint, WM_SETFONT,
		reinterpret_cast<WPARAM>(GetStockObject(DEFAULT_GUI_FONT)), TRUE);
}
void calculator::show_bit_hint(const std::wstring& text, POINT screen_pt) {
	if (!m_bit_hint) return;

	SetWindowTextW(m_bit_hint, text.c_str());

	HDC hdc = GetDC(m_bit_hint);
	RECT rc{ 0, 0, 0, 0 };
	DrawTextW(hdc, text.c_str(), -1, &rc, DT_CALCRECT | DT_SINGLELINE);
	ReleaseDC(m_bit_hint, hdc);

	int width = (rc.right - rc.left) + 12;
	int height = (rc.bottom - rc.top) + 8;

	SetWindowPos(
		m_bit_hint,
		HWND_TOPMOST,
		screen_pt.x + 16,
		screen_pt.y + 20,
		width,
		height,
		SWP_SHOWWINDOW | SWP_NOACTIVATE
	);
}
void calculator::hide_bit_hint() {
	if (m_bit_hint) {
		ShowWindow(m_bit_hint, SW_HIDE);
	}
}

void calculator::populate_base_combo() {
	const wchar_t* items[] = {
		L"Dec",
		L"Hex",
		L"Oct",
		L"Bin"
	};

	for (const auto* item : items) {
		SendMessageW(m_base_combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item));
	}

	SendMessageW(m_base_combo, CB_SETCURSEL, static_cast<WPARAM>(0), 0);
}

std::wstring calculator::to_binary_string(uint64_t value, int bits) const {
	if (value == 0) return L"b0";

	std::wstring result = L"b";
	bool started = false;

	for (int i = bits - 1; i >= 0; --i) {
		bool bit = ((value >> i) & 1ULL) != 0;

		if (bit) {
			started = true;
		}

		if (started) {
			result += bit ? L'1' : L'0';
		}
	}

	return result;
}

std::wstring calculator::to_octal_string(uint64_t value) const {
	if (value == 0) return L"o0";

	std::wstring digits;
	while (value > 0) {
		digits.insert(digits.begin(), wchar_t(L'0' + (value % 8)));
		value /= 8;
	}
	return L"o" + digits;
}

std::wstring calculator::to_hex_string(uint64_t value) const {
	if (value == 0) return L"0x0";

	const wchar_t* hex = L"0123456789ABCDEF";
	std::wstring digits;
	while (value > 0) {
		digits.insert(digits.begin(), hex[value & 0xF]);
		value >>= 4;
	}
	return L"0x" + digits;
}

std::wstring calculator::format_value_by_base() const {
	if (is_float_type()) {
		return get_display_text();
	}

	uint64_t raw = m_bits;
	int bits = get_visible_bit_count();

	switch (m_number_base) {
	case NumberBase::Dec:
		return get_display_text();
	case NumberBase::Hex:
		return to_hex_string(raw);
	case NumberBase::Oct:
		return to_octal_string(raw);
	case NumberBase::Bin:
		return to_binary_string(raw, bits);
	}

	return get_display_text();
}

bool calculator::is_input_char_allowed(wchar_t ch) const {
	switch (m_number_base) {
	case NumberBase::Dec:
		if (is_float_type()) {
			return (ch >= L'0' && ch <= L'9') || ch == L'.' || ch == L'-';
		}
		return (ch >= L'0' && ch <= L'9') || ch == L'-';

	case NumberBase::Hex:
		return (ch >= L'0' && ch <= L'9') ||
			(ch >= L'a' && ch <= L'f') ||
			(ch >= L'A' && ch <= L'F');

	case NumberBase::Oct:
		return (ch >= L'0' && ch <= L'7');

	case NumberBase::Bin:
		return ch == L'0' || ch == L'1';
	}
	return false;
}

	bool calculator::should_show_precision_warning() const {
		if (!(m_data_type == DataType::Float32 || m_data_type == DataType::Float64))
			return false;

		if (m_number_base != NumberBase::Dec)
			return false;

		if (m_result_text.empty())
			return false;

		try {
			double entered = std::wcstod(m_result_text.c_str(), nullptr);

			if (m_data_type == DataType::Float32) {
				float f = static_cast<float>(entered);
				return static_cast<double>(f) != entered;
			}

			if (m_data_type == DataType::Float64) {
				double d = entered;
				long double ld = std::wcstold(m_result_text.c_str(), nullptr);
				return static_cast<long double>(d) != ld;
			}
		}
		catch (...) {
			return false;
		}

		return false;
	}

	void calculator::update_precision_warning() {
		if (should_show_precision_warning()) {
			if (m_data_type == DataType::Float32) {
				float f = static_cast<float>(std::wcstod(m_result_text.c_str(), nullptr));
				m_precision_warning = L"Precision Warning: stored as " + double_to_text(f);
			}
			else if (m_data_type == DataType::Float64) {
				double d = std::wcstod(m_result_text.c_str(), nullptr);
				m_precision_warning = L"Precision Warning: stored as " + double_to_text(d);
			}
		}
		else {
			m_precision_warning.clear();
		}
	}

	bool calculator::copy_text_to_clipboard(const std::wstring& text) {
		if (!OpenClipboard(m_main)) return false;
		EmptyClipboard();

		size_t bytes = (text.size() + 1) * sizeof(wchar_t);
		HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
		if (!hMem) {
			CloseClipboard();
			return false;
		}

		void* ptr = GlobalLock(hMem);
		std::memcpy(ptr, text.c_str(), bytes);
		GlobalUnlock(hMem);

		SetClipboardData(CF_UNICODETEXT, hMem);
		CloseClipboard();
		return true;
	}

	bool calculator::paste_text_from_clipboard(std::wstring& out) {
		if (!OpenClipboard(m_main)) return false;

		HANDLE hData = GetClipboardData(CF_UNICODETEXT);
		if (!hData) {
			CloseClipboard();
			return false;
		}

		wchar_t* text = static_cast<wchar_t*>(GlobalLock(hData));
		if (!text) {
			CloseClipboard();
			return false;
		}

		out = text;
		GlobalUnlock(hData);
		CloseClipboard();
		return true;
	}

	bool calculator::is_valid_pasted_text(const std::wstring& text) const {
		if (text.empty())
			return false;

		std::wstring s = text;

		if (is_float_type()) {
			wchar_t* end = nullptr;
			std::wcstod(s.c_str(), &end);
			return end != s.c_str() && *end == L'\0';
		}

		if (m_number_base == NumberBase::Hex) {
			if (s.rfind(L"0x", 0) == 0 || s.rfind(L"0X", 0) == 0)
				s = s.substr(2);
			if (s.empty()) return false;
			for (wchar_t ch : s) {
				if (!((ch >= L'0' && ch <= L'9') || (ch >= L'a' && ch <= L'f') || (ch >= L'A' && ch <= L'F')))
					return false;
			}
			return true;
		}

		if (m_number_base == NumberBase::Oct) {
			if (!s.empty() && (s[0] == L'o' || s[0] == L'O'))
				s = s.substr(1);
			if (s.empty()) return false;
			for (wchar_t ch : s) {
				if (!(ch >= L'0' && ch <= L'7'))
					return false;
			}
			return true;
		}

		if (m_number_base == NumberBase::Bin) {
			if (!s.empty() && (s[0] == L'b' || s[0] == L'B'))
				s = s.substr(1);
			if (s.empty()) return false;
			for (wchar_t ch : s) {
				if (!(ch == L'0' || ch == L'1'))
					return false;
			}
			return true;
		}

		wchar_t* end = nullptr;
		std::wcstoll(s.c_str(), &end, 10);
		return end != s.c_str() && *end == L'\0';
	}

	bool calculator::is_zero_text_for_current_base() const {
		switch (m_number_base) {
		case NumberBase::Dec:
			return m_result_text == L"0";
		case NumberBase::Hex:
			return m_result_text == L"0x0" || m_result_text == L"0X0";
		case NumberBase::Oct:
			return m_result_text == L"o0" || m_result_text == L"O0";
		case NumberBase::Bin:
			return m_result_text == L"b0" || m_result_text == L"B0";
		}
		return m_result_text == L"0";
	}

	void calculator::append_digit_to_result(wchar_t digit) {
		if (m_start_new_input || is_zero_text_for_current_base()) {
			switch (m_number_base) {
			case NumberBase::Dec:
				m_result_text = std::wstring(1, digit);
				break;
			case NumberBase::Hex:
				m_result_text = L"0x";
				m_result_text += digit;
				break;
			case NumberBase::Oct:
				m_result_text = L"o";
				m_result_text += digit;
				break;
			case NumberBase::Bin:
				m_result_text = L"b";
				m_result_text += digit;
				break;
			}
			m_start_new_input = false;
		}
		else {
			m_result_text += digit;
		}
	}