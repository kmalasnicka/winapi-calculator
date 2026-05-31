# DevCalculator – WinAPI Calculator

DevCalculator is a desktop calculator application written in C++ using the WinAPI library.  
The program was created as a native Windows application, without using ready-made graphical frameworks. The window, buttons, menus, display area and user interactions are handled directly with WinAPI mechanisms.

The goal of the project was to create a functional calculator with two working modes: a standard calculator mode and a programmer mode. The application allows the user to perform basic arithmetic operations, work with different number systems, inspect binary representation of values and interact with the calculator using both mouse and keyboard.

## Project Description

The application is divided into two main modes.

The first mode is the basic calculator mode. It provides the most common arithmetic operations, such as addition, subtraction, multiplication and division. The user can enter numbers, perform calculations, clear the current value, delete the last digit and display the result. The calculator also keeps a simple operation history, so the user can see the expression that is currently being calculated.

The second mode is the programmer calculator mode. This mode extends the standard calculator with features useful when working with low-level numeric values. The user can switch between decimal, hexadecimal, octal and binary representation. The same value can therefore be viewed in different number systems. This makes the calculator useful for tasks connected with programming, bit operations and numeric conversions.

In programmer mode, the application also displays a bit representation of the current value. The user can inspect individual bits and change them directly. The value shown by the calculator is then updated according to the selected numeric base and data type. This part of the application shows how binary data can be represented visually in a desktop program.

The calculator supports several data types. This allows the user to check how values are represented depending on the selected type. It is especially useful in programmer mode, where the size and interpretation of the value affect the visible bit representation.

## User Interface

The interface consists of a custom display area, a grid of calculator buttons, menu options and additional controls available in programmer mode.

The display area is drawn manually using GDI. It shows the current value and the operation history. The calculator buttons are standard WinAPI controls, but their layout is adjusted dynamically when the window size changes.

In basic mode, only the standard calculator buttons are visible. In programmer mode, additional buttons and controls appear, including number system selection, data type selection and the bit view.

The application can be resized, and the controls are rearranged to fit the available window space. This makes the program more flexible than a fixed-size calculator window.

## Main Functionalities

The calculator allows the user to:

- perform basic arithmetic calculations,
- use decimal, hexadecimal, octal and binary number systems,
- switch between basic and programmer mode,
- display the binary representation of the current value,
- edit individual bits in programmer mode,
- choose the numeric data type,
- use keyboard shortcuts for faster input,
- copy and paste numeric values,
- clear the current input or remove the last digit,
- keep the calculator window always on top,
- save selected window settings between program runs.

## Keyboard Support

The application supports both mouse and keyboard input.  
Numbers and operators can be typed directly from the keyboard. The Enter key works as the equals button, Escape clears the current input, and Backspace removes the last character.

This makes the calculator faster to use and closer to standard desktop calculator applications.

## Settings

The program saves selected settings to a configuration file. Thanks to this, some window options can be restored when the application is opened again.

This includes options such as the selected mode, window size or always-on-top setting, depending on the current state of the application.
