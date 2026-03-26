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

	std::wstring get_display_text() const;

	HINSTANCE m_instance;
	HWND m_main;
	HWND m_display;
	HWND m_bits_display; //panel bitow
	HWND m_type_combo; 
	HACCEL m_accel; //uchwyt akceleratora

	HWND m_buttons[18]; //tablica uchwytow do przyciskow

	std::wstring m_history_text;
	std::wstring m_result_text;

	double m_stored_value;
	wchar_t m_pending_operator;
	bool m_start_new_input;

	CalcMode m_mode;
	DataType m_data_type; 
	uint64_t m_bits; //przechowuje aktualna wartosc jako surowe bity
public:
	calculator(HINSTANCE instance);
	int run(int show_command);
}; 
