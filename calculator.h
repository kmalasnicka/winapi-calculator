#pragma once
#include <windows.h>
#include <string>
#include <cstdint>

enum class CalcMode {
	Basic = 0,
	Programmer = 1
};

enum class DataType {
	Int8 = 0,
	UInt8,
	Int16,
	UInt16,
	Int32,
	UInt32,
	Int64,
	UInt64,
	Half,
	Float32,
	Float64
};

enum class NumberBase {
	Dec,
	Hex,
	Oct,
	Bin
};

class calculator
{
private:
	bool register_class();
	bool register_display_class();
	bool register_bits_class();

	static std::wstring const s_class_name;
	static std::wstring const s_display_class_name;
	static std::wstring const s_bits_class_name;

	static LRESULT CALLBACK window_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK display_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK bits_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

	LRESULT window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

	HWND create_window();
	void create_display(HWND parent);
	void create_buttons(HWND parent);
	void create_programmer_controls(HWND parent); //tworzy nowe elementy ui: panel bitow i dropdown
	void resize_children(int width, int height);

	void populate_type_combo(); //dodaje typy danych i ustawia domyslna wartosc int32
	void set_mode(CalcMode mode);
	void sync_programmer_ui(); //odpowiada za widocznosc w programmer pokazuje panel bitow + combo w basic je ukrywa
	void update_all_displays();
	int get_visible_bit_count() const;
	bool is_float_type() const;
	int get_exponent_bits() const;
	int get_mantissa_bits() const;
	void update_tooltip_for_bit(int bit_index, POINT screen_pt);
	void toggle_bit(int bit_index);
	void sync_bits_from_result_text();

	std::wstring get_display_text() const;

	HINSTANCE m_instance;
	HWND m_main;
	HWND m_display;
	HWND m_bits_display; //panel bitow
	HWND m_type_combo; 
	HACCEL m_accel; //uchwyt akceleratora
	int m_hover_bit_index;

	HWND m_buttons[18]; //tablica uchwytow do przyciskow

	std::wstring m_history_text;
	std::wstring m_result_text;

	double m_stored_value;
	wchar_t m_pending_operator;
	bool m_start_new_input;

	CalcMode m_mode;
	DataType m_data_type; 
	uint64_t m_bits; //przechowuje aktualna wartosc jako surowe bity
	HWND m_bit_hint;
	void create_bit_hint(HWND parent);
	void show_bit_hint(const std::wstring& text, POINT screen_pt);
	void hide_bit_hint();

	HWND m_base_combo;
	NumberBase m_number_base;
	std::wstring m_precision_warning;

	void populate_base_combo();
	std::wstring format_value_by_base() const;
	bool is_input_char_allowed(wchar_t ch) const;
	std::wstring to_binary_string(uint64_t value, int bits) const;
	std::wstring to_octal_string(uint64_t value) const;
	std::wstring to_hex_string(uint64_t value) const;

	bool should_show_precision_warning() const;
	void update_precision_warning();
	bool copy_text_to_clipboard(const std::wstring& text);
	bool paste_text_from_clipboard(std::wstring& out);
	bool is_valid_pasted_text(const std::wstring& text) const;
	bool is_zero_text_for_current_base() const;
	void append_digit_to_result(wchar_t digit);
public:
	calculator(HINSTANCE instance);
	int run(int show_command);
}; 
