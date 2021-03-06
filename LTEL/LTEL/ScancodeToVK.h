#pragma once
#include <map>
#include <GlobalConstants.hpp>

// Essentially ripped from the Godot engine
const std::map<int, int> scancode_to_vk = {
    { godot::GlobalConstants::KEY_BACKSPACE, VK_BACK }, // (0x08) // backspace
    { godot::GlobalConstants::KEY_TAB, VK_TAB }, //(0x09)

    //VK_CLEAR (0x0C)

    { godot::GlobalConstants::KEY_ENTER, VK_RETURN }, //(0x0D)

    { godot::GlobalConstants::KEY_SHIFT, VK_SHIFT }, //(0x10)

    { godot::GlobalConstants::KEY_CONTROL, VK_CONTROL }, //(0x11)

    { godot::GlobalConstants::KEY_ALT, VK_MENU }, //(0x12)

    { godot::GlobalConstants::KEY_PAUSE, VK_PAUSE }, //(0x13)

    { godot::GlobalConstants::KEY_CAPSLOCK, VK_CAPITAL }, //(0x14)

    { godot::GlobalConstants::KEY_ESCAPE, VK_ESCAPE }, //(0x1B)

    { godot::GlobalConstants::KEY_SPACE, VK_SPACE }, //(0x20)

    { godot::GlobalConstants::KEY_PAGEUP, VK_PRIOR }, //(0x21)

    { godot::GlobalConstants::KEY_PAGEDOWN, VK_NEXT }, //(0x22)

    { godot::GlobalConstants::KEY_END, VK_END }, //(0x23)

    { godot::GlobalConstants::KEY_HOME, VK_HOME }, //(0x24)

    { godot::GlobalConstants::KEY_LEFT, VK_LEFT }, //(0x25)

    { godot::GlobalConstants::KEY_UP, VK_UP }, //(0x26)

    { godot::GlobalConstants::KEY_RIGHT, VK_RIGHT }, //(0x27)

    { godot::GlobalConstants::KEY_DOWN, VK_DOWN }, // (0x28)

    //VK_SELECT (0x29)

    { godot::GlobalConstants::KEY_PRINT, VK_PRINT }, // (0x2A)

    //VK_EXECUTE (0x2B)

    { godot::GlobalConstants::KEY_PRINT, VK_SNAPSHOT }, // (0x2C)

    { godot::GlobalConstants::KEY_INSERT, VK_INSERT }, // (0x2D)

    { godot::GlobalConstants::KEY_DELETE, VK_DELETE }, // (0x2E)

    { godot::GlobalConstants::KEY_HELP, VK_HELP }, // (0x2F)

    { godot::GlobalConstants::KEY_0, (0x30) }, ////0 key
    { godot::GlobalConstants::KEY_1, (0x31) }, ////1 key
    { godot::GlobalConstants::KEY_2, (0x32) }, ////2 key
    { godot::GlobalConstants::KEY_3, (0x33) }, ////3 key
    { godot::GlobalConstants::KEY_4, (0x34) }, ////4 key
    { godot::GlobalConstants::KEY_5, (0x35) }, ////5 key
    { godot::GlobalConstants::KEY_6, (0x36) }, ////6 key
    { godot::GlobalConstants::KEY_7, (0x37) }, ////7 key
    { godot::GlobalConstants::KEY_8, (0x38) }, ////8 key
    { godot::GlobalConstants::KEY_9, (0x39) }, ////9 key
    { godot::GlobalConstants::KEY_A, (0x41) }, ////A key
    { godot::GlobalConstants::KEY_B, (0x42) }, ////B key
    { godot::GlobalConstants::KEY_C, (0x43) }, ////C key
    { godot::GlobalConstants::KEY_D, (0x44) }, ////D key
    { godot::GlobalConstants::KEY_E, (0x45) }, ////E key
    { godot::GlobalConstants::KEY_F, (0x46) }, ////F key
    { godot::GlobalConstants::KEY_G, (0x47) }, ////G key
    { godot::GlobalConstants::KEY_H, (0x48) }, ////H key
    { godot::GlobalConstants::KEY_I, (0x49) }, ////I key
    { godot::GlobalConstants::KEY_J, (0x4A) }, ////J key
    { godot::GlobalConstants::KEY_K, (0x4B) }, ////K key
    { godot::GlobalConstants::KEY_L, (0x4C) }, ////L key
    { godot::GlobalConstants::KEY_M, (0x4D) }, ////M key
    { godot::GlobalConstants::KEY_N, (0x4E) }, ////N key
    { godot::GlobalConstants::KEY_O, (0x4F) }, ////O key
    { godot::GlobalConstants::KEY_P, (0x50) }, ////P key
    { godot::GlobalConstants::KEY_Q, (0x51) }, ////Q key
    { godot::GlobalConstants::KEY_R, (0x52) }, ////R key
    { godot::GlobalConstants::KEY_S, (0x53) }, ////S key
    { godot::GlobalConstants::KEY_T, (0x54) }, ////T key
    { godot::GlobalConstants::KEY_U, (0x55) }, ////U key
    { godot::GlobalConstants::KEY_V, (0x56) }, ////V key
    { godot::GlobalConstants::KEY_W, (0x57) }, ////W key
    { godot::GlobalConstants::KEY_X, (0x58) }, ////X key
    { godot::GlobalConstants::KEY_Y, (0x59) }, ////Y key
    { godot::GlobalConstants::KEY_Z, (0x5A) }, ////Z key

    { godot::GlobalConstants::KEY_MASK_META, VK_LWIN }, //(0x5B)
    { godot::GlobalConstants::KEY_MASK_META, VK_RWIN }, //(0x5C)
    { godot::GlobalConstants::KEY_MENU, VK_APPS }, //(0x5D)
    { godot::GlobalConstants::KEY_STANDBY, VK_SLEEP }, //(0x5F)
    { godot::GlobalConstants::KEY_KP_0, VK_NUMPAD0 }, //(0x60)
    { godot::GlobalConstants::KEY_KP_1, VK_NUMPAD1 }, //(0x61)
    { godot::GlobalConstants::KEY_KP_2, VK_NUMPAD2 }, //(0x62)
    { godot::GlobalConstants::KEY_KP_3, VK_NUMPAD3 }, //(0x63)
    { godot::GlobalConstants::KEY_KP_4, VK_NUMPAD4 }, //(0x64)
    { godot::GlobalConstants::KEY_KP_5, VK_NUMPAD5 }, //(0x65)
    { godot::GlobalConstants::KEY_KP_6, VK_NUMPAD6 }, //(0x66)
    { godot::GlobalConstants::KEY_KP_7, VK_NUMPAD7 }, //(0x67)
    { godot::GlobalConstants::KEY_KP_8, VK_NUMPAD8 }, //(0x68)
    { godot::GlobalConstants::KEY_KP_9, VK_NUMPAD9 }, //(0x69)
    { godot::GlobalConstants::KEY_KP_MULTIPLY, VK_MULTIPLY }, // (0x6A)
    { godot::GlobalConstants::KEY_KP_ADD, VK_ADD }, // (0x6B)
    //VK_SEPARATOR (0x6C)
    { godot::GlobalConstants::KEY_KP_SUBTRACT, VK_SUBTRACT }, // (0x6D)
    { godot::GlobalConstants::KEY_KP_PERIOD, VK_DECIMAL }, // (0x6E)
    { godot::GlobalConstants::KEY_KP_DIVIDE, VK_DIVIDE }, // (0x6F)
    { godot::GlobalConstants::KEY_F1, VK_F1 }, // (0x70)
    { godot::GlobalConstants::KEY_F2, VK_F2 }, // (0x71)
    { godot::GlobalConstants::KEY_F3, VK_F3 }, // (0x72)
    { godot::GlobalConstants::KEY_F4, VK_F4 }, // (0x73)
    { godot::GlobalConstants::KEY_F5, VK_F5 }, // (0x74)
    { godot::GlobalConstants::KEY_F6, VK_F6 }, // (0x75)
    { godot::GlobalConstants::KEY_F7, VK_F7 }, // (0x76)
    { godot::GlobalConstants::KEY_F8, VK_F8 }, // (0x77)
    { godot::GlobalConstants::KEY_F9, VK_F9 }, // (0x78)
    { godot::GlobalConstants::KEY_F10, VK_F10 }, // (0x79)
    { godot::GlobalConstants::KEY_F11, VK_F11 }, // (0x7A)
    { godot::GlobalConstants::KEY_F12, VK_F12 }, // (0x7B)
    { godot::GlobalConstants::KEY_F13, VK_F13 }, // (0x7C)
    { godot::GlobalConstants::KEY_F14, VK_F14 }, // (0x7D)
    { godot::GlobalConstants::KEY_F15, VK_F15 }, // (0x7E)
    { godot::GlobalConstants::KEY_F16, VK_F16 }, // (0x7F)
    { godot::GlobalConstants::KEY_NUMLOCK, VK_NUMLOCK }, // (0x90)
    { godot::GlobalConstants::KEY_SCROLLLOCK, VK_SCROLL }, // (0x91)
    { godot::GlobalConstants::KEY_SHIFT, VK_LSHIFT }, // (0xA0)
    { godot::GlobalConstants::KEY_SHIFT, VK_RSHIFT }, // (0xA1)
    { godot::GlobalConstants::KEY_CONTROL, VK_LCONTROL }, // (0xA2)
    { godot::GlobalConstants::KEY_CONTROL, VK_RCONTROL }, // (0xA3)
    { godot::GlobalConstants::KEY_MENU, VK_LMENU }, // (0xA4)
    { godot::GlobalConstants::KEY_MENU, VK_RMENU }, // (0xA5)

    { godot::GlobalConstants::KEY_BACK, VK_BROWSER_BACK }, // (0xA6)

    { godot::GlobalConstants::KEY_FORWARD, VK_BROWSER_FORWARD }, // (0xA7)

    { godot::GlobalConstants::KEY_REFRESH, VK_BROWSER_REFRESH }, // (0xA8)

    { godot::GlobalConstants::KEY_STOP, VK_BROWSER_STOP }, // (0xA9)

    { godot::GlobalConstants::KEY_SEARCH, VK_BROWSER_SEARCH }, // (0xAA)

    { godot::GlobalConstants::KEY_FAVORITES, VK_BROWSER_FAVORITES }, // (0xAB)

    { godot::GlobalConstants::KEY_HOMEPAGE, VK_BROWSER_HOME }, // (0xAC)

    { godot::GlobalConstants::KEY_VOLUMEMUTE, VK_VOLUME_MUTE }, // (0xAD)

    { godot::GlobalConstants::KEY_VOLUMEDOWN, VK_VOLUME_DOWN }, // (0xAE)

    { godot::GlobalConstants::KEY_VOLUMEUP, VK_VOLUME_UP }, // (0xAF)

    { godot::GlobalConstants::KEY_MEDIANEXT, VK_MEDIA_NEXT_TRACK }, // (0xB0)

    { godot::GlobalConstants::KEY_MEDIAPREVIOUS, VK_MEDIA_PREV_TRACK }, // (0xB1)

    { godot::GlobalConstants::KEY_MEDIASTOP, VK_MEDIA_STOP }, // (0xB2)

    //VK_MEDIA_PLAY_PAUSE (0xB3)

    { godot::GlobalConstants::KEY_LAUNCHMAIL, VK_LAUNCH_MAIL }, // (0xB4)

    { godot::GlobalConstants::KEY_LAUNCHMEDIA, VK_LAUNCH_MEDIA_SELECT }, // (0xB5)

    { godot::GlobalConstants::KEY_LAUNCH0, VK_LAUNCH_APP1 }, // (0xB6)

    { godot::GlobalConstants::KEY_LAUNCH1, VK_LAUNCH_APP2 }, // (0xB7)

    { godot::GlobalConstants::KEY_SEMICOLON, VK_OEM_1 }, // (0xBA)

    { godot::GlobalConstants::KEY_EQUAL, VK_OEM_PLUS }, // (0xBB) // Windows 2000/XP: For any country/region, the '+' key
    { godot::GlobalConstants::KEY_COMMA, VK_OEM_COMMA }, // (0xBC) // Windows 2000/XP: For any country/region, the ',' key
    { godot::GlobalConstants::KEY_MINUS, VK_OEM_MINUS }, // (0xBD) // Windows 2000/XP: For any country/region, the '-' key
    { godot::GlobalConstants::KEY_PERIOD, VK_OEM_PERIOD }, // (0xBE) // Windows 2000/XP: For any country/region, the '.' key
    { godot::GlobalConstants::KEY_SLASH, VK_OEM_2 }, // (0xBF) //Windows 2000/XP: For the US standard keyboard, the '/?' key

    { godot::GlobalConstants::KEY_QUOTELEFT, VK_OEM_3 }, // (0xC0)
    { godot::GlobalConstants::KEY_BRACELEFT, VK_OEM_4 }, // (0xDB)
    { godot::GlobalConstants::KEY_BACKSLASH, VK_OEM_5 }, // (0xDC)
    { godot::GlobalConstants::KEY_BRACERIGHT, VK_OEM_6 }, // (0xDD)
    { godot::GlobalConstants::KEY_APOSTROPHE, VK_OEM_7 }, // (0xDE)
    /*
{VK_OEM_8 (0xDF)
{VK_OEM_102 (0xE2) // Windows 2000/XP: Either the angle bracket key or the backslash key on the RT 102-key keyboard
*/
//{ godot::GlobalConstants::KEY_PLAY, VK_PLAY},// (0xFA)

{ godot::GlobalConstants::KEY_UNKNOWN, 0 }

};