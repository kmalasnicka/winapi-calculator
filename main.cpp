#include <windows.h>
#include "calculator.h"

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prevInstance, LPWSTR command_line, int show_command) {
	calculator app{ instance };
	return app.run(show_command);
}