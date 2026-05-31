# DevCalculator – WinAPI Calculator

DevCalculator is a desktop calculator application written in C++ using the WinAPI library.

The program was created as a native Windows application, without using ready-made graphical frameworks. The window, buttons, menus, display area and user interactions are handled directly with WinAPI mechanisms.

The application has two working modes: a standard calculator mode and a programmer mode. It allows the user to perform basic arithmetic operations, work with different number systems, view the bit representation of values and use the calculator with both mouse and keyboard.

## Project Description

The basic mode provides standard arithmetic operations, such as addition, subtraction, multiplication and division. The user can enter numbers, clear the current value, delete the last digit and display the result. The calculator also shows a simple operation history.

The programmer mode extends the calculator with features useful for programming-related calculations. The user can switch between decimal, hexadecimal, octal and binary representation. The application also displays the bit representation of the current value and allows the user to edit individual bits.

The calculator supports different numeric data types, which affects how values are represented in programmer mode.

## User Interface

The interface consists of a custom display area, calculator buttons, menu options and additional controls available in programmer mode.

The display area is drawn manually using GDI and shows the current value and operation history. The calculator buttons are standard WinAPI controls, and their layout is adjusted when the window size changes.

In basic mode, only standard calculator buttons are visible. In programmer mode, additional controls for number system selection, data type selection and bit view are shown.

## Main Functionalities

- basic arithmetic calculations,
- basic and programmer mode,
- decimal, hexadecimal, octal and binary number systems,
- bit representation display,
- editing individual bits in programmer mode,
- selectable numeric data type,
- keyboard input support,
- copy and paste support,
- clearing input and deleting the last digit,
- always-on-top option,
- saving selected settings between program runs.

## Keyboard Support

The calculator supports mouse and keyboard input. Numbers and operators can be typed directly from the keyboard. Enter works as the equals button, Escape clears the input and Backspace removes the last character.

## Settings

The program saves selected settings to a configuration file, so they can be restored after restarting the application.
