# DevCalculator – WinAPI Calculator

DevCalculator is a desktop calculator application written in C++ using the WinAPI library.

The program was created as a native Windows application, without using ready-made graphical frameworks. The window, buttons, menus, display area and user interactions are handled directly with WinAPI mechanisms.

The calculator has two modes: basic and programmer. Basic mode provides standard arithmetic operations, while programmer mode adds support for different number systems, numeric data types and bit representation.

## Project Description

In basic mode, the user can perform calculations, clear the current value, delete the last digit and view a simple operation history.

In programmer mode, the user can switch between decimal, hexadecimal, octal and binary representation. The application also displays the bit representation of the current value and allows editing individual bits.

The display area is drawn manually using GDI. The rest of the interface is built with standard WinAPI controls and adjusts to the selected mode and window size.

## Main Functionalities

- basic arithmetic calculations,
- basic and programmer mode,
- decimal, hexadecimal, octal and binary number systems,
- selectable numeric data type,
- bit representation display and editing,
- keyboard input support,
- copy and paste support,
- clearing input and deleting the last digit,
- always-on-top option,
- saving selected settings between program runs.

## Keyboard Support

Numbers and operators can be typed directly from the keyboard. Enter works as the equals button, Escape clears the input and Backspace removes the last character.

## Settings

The program saves selected settings to a configuration file, so they can be restored after restarting the application.
