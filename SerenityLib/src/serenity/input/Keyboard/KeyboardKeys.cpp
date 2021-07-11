#include <serenity/input/Keyboard/KeyboardKeys.h>
#include <array>

constexpr std::array<const char*, KeyboardKey::_TOTAL> KEYBOARD_KEY_NAMES { {
    "Unknown",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "Esc",
    "Ctrl",
    "Shift",
    "Alt",
    "System",
    "Ctrl",
    "Shift",
    "Alt",
    "System",
    "Menu",
    "[",
    "]",
    ";",
    ",",
    ".",
    "'",
    "/",
    "\\",
    "~",
    "=",
    "-",
    "Spacebar",
    "Enter",
    "Backspace",
    "Tab",
    "Page Up",
    "Page Down",
    "End",
    "Home",
    "Insert",
    "Delete",
    "+",
    "-",
    "*",
    "/",
    "Left Arrow",
    "Right Arrow",
    "Up Arrow",
    "Down Arrow",
    "Numpad 0",
    "Numpad 1",
    "Numpad 2",
    "Numpad 3",
    "Numpad 4",
    "Numpad 5",
    "Numpad 6",
    "Numpad 7",
    "Numpad 8",
    "Numpad 9",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "Pause",
} };

const char* KeyboardKey::toCharArray(KeyboardKey::Key keyboard_key) {
    return KEYBOARD_KEY_NAMES[keyboard_key];
}
std::string KeyboardKey::toString(KeyboardKey::Key keyboard_key) {
    return KEYBOARD_KEY_NAMES[keyboard_key];
}
bool KeyboardKey::isModifierKey(KeyboardKey::Key key) {
    return (
        key == KeyboardKey::LeftShift || 
        key == KeyboardKey::RightShift || 
        key == KeyboardKey::LeftControl || 
        key == KeyboardKey::RightControl || 
        key == KeyboardKey::LeftAlt || 
        key == KeyboardKey::RightAlt
    );
}
const char* KeyboardKey::toCharArray(uint32_t key) {
    return KeyboardKey::toCharArray((KeyboardKey::Key)key);
}
std::string KeyboardKey::toString(uint32_t key) {
    return KeyboardKey::toString((KeyboardKey::Key)key);
}
bool KeyboardKey::isModifierKey(uint32_t key) {
    return KeyboardKey::isModifierKey((KeyboardKey::Key)key);
}