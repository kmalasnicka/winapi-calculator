#pragma once
#include <windows.h>
#include <string>
class calculator
{
private:
	bool register_class();
	bool register_display_class();

	static std::wstring const s_class_name;
	static std::wstring const s_display_class_name;

	static LRESULT CALLBACK window_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK display_proc_static(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

	LRESULT window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
	
	HWND create_window();
	void create_display(HWND parent);
	void create_buttons(HWND parent);
	void resize_children(int width, int height);

	HINSTANCE m_instance;
	HWND m_main;
	HWND m_display;
	HACCEL m_accel; //uchwyt akceleratora

	HWND m_buttons[18]; //tablica uchwytow do przyciskow

	std::wstring m_history_text;
	std::wstring m_result_text;
public:
	calculator(HINSTANCE instance);
	int run(int show_command);
}; 
