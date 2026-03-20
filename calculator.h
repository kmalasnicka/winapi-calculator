#pragma once
#include <windows.h>
#include <string>
class calculator
{
private:
	bool register_class();
	static std::wstring const s_class_name;
	static LRESULT CALLBACK window_proc_static(
		HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);
	LRESULT window_proc(
		HWND window, UINT message,
		WPARAM wparam, LPARAM lparam);
	HWND create_window();
	HINSTANCE m_instance;
	HWND m_main;
public:
	calculator(HINSTANCE instance);
	int run(int show_command);
}; 
