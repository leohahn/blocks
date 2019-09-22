#pragma once

#include "Han/Core.hpp"
#include "Han/Collections/String.hpp"
#include "Han/StringBuilder.hpp"
#include <functional>

//
// Design basically taken from Hazel: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Events/Event.h
//

enum class EventType
{
	MouseButtonPress,
	MouseButtonRelease,
	MouseMove,
	MouseWheel,
	KeyPress,
	KeyRelease,
	Quit,
};

enum EventCategory
{
	EventCategory_None     = 0,
	EventCategory_Mouse    = HAN_BIT(0),
	EventCategory_Keyboard = HAN_BIT(1),
};

enum KeyCode
{
    KeyCode_a = 'a',
    KeyCode_b = 'b',
    KeyCode_c = 'c',
    KeyCode_d = 'd',
    KeyCode_e = 'e',
    KeyCode_f = 'f',
    KeyCode_g = 'g',
    KeyCode_h = 'h',
    KeyCode_i = 'i',
    KeyCode_j = 'j',
    KeyCode_k = 'k',
    KeyCode_l = 'l',
    KeyCode_m = 'm',
    KeyCode_n = 'n',
    KeyCode_o = 'o',
    KeyCode_p = 'p',
    KeyCode_q = 'q',
    KeyCode_r = 'r',
    KeyCode_s = 's',
    KeyCode_t = 't',
    KeyCode_u = 'u',
    KeyCode_v = 'v',
    KeyCode_w = 'w',
    KeyCode_x = 'x',
    KeyCode_y = 'y',
    KeyCode_z = 'z',

    KeyCode_LeftBracket = '[',
    KeyCode_BackSlash = '\\',
    KeyCode_RightBracket = ']',
    KeyCode_Caret = '^',
    KeyCode_Underscore = '_',
    KeyCode_BackQuote = '`',

    KeyCode_Return = '\r',
    KeyCode_Escape = '\033',
    KeyCode_Backspace = '\b',
    KeyCode_Tab = '\t',
    KeyCode_Space = ' ',
    KeyCode_Exclaim = '!',
    KeyCode_QuotedBL = '"',
    KeyCode_Hash = '#',
    KeyCode_Percent = '%',
    KeyCode_Dollar = '$',
    KeyCode_Ampersand = '&',
    KeyCode_Quote = '\'',
    KeyCode_LeftParen = '(',
    KeyCode_RightParen = ')',
    KeyCode_Asterisk = '*',
    KeyCode_Plus = '+',
    KeyCode_Comma = ',',
    KeyCode_Minus = '-',
    KeyCode_Period = '.',
    KeyCode_Slash = '/',
    KeyCode_0 = '0',
    KeyCode_1 = '1',
    KeyCode_2 = '2',
    KeyCode_3 = '3',
    KeyCode_4 = '4',
    KeyCode_5 = '5',
    KeyCode_6 = '6',
    KeyCode_7 = '7',
    KeyCode_8 = '8',
    KeyCode_9 = '9',
    KeyCode_Colon = ':',
    KeyCode_Semicolon = ';',
    KeyCode_Less = '<',
    KeyCode_Equals = '=',
    KeyCode_Greater = '>',
    KeyCode_Question = '?',
    KeyCode_At = '@',
	KeyCode_CapsLock = 1073741881,
	KeyCode_F1 = 1073741882,
	KeyCode_F2 = 1073741883,
	KeyCode_F3 = 1073741884,
	KeyCode_F4 = 1073741885,
	KeyCode_F5 = 1073741886,
	KeyCode_F6 = 1073741887,
	KeyCode_F7 = 1073741888,
	KeyCode_F8 = 1073741889,
	KeyCode_F9 = 1073741890,
	KeyCode_F10 = 1073741891,
	KeyCode_F11 = 1073741892,
	KeyCode_F12 = 1073741893,
	KeyCode_PrintScreen = 1073741894,
	KeyCode_Right = 1073741903,
	KeyCode_Left = 1073741904,
	KeyCode_Down = 1073741905,
	KeyCode_Up = 1073741906,
	KeyCode_F13 = 1073741928,
	KeyCode_F14 = 1073741929,
	KeyCode_F15 = 1073741930,
	KeyCode_Mute = 1073741951,
	KeyCode_VolumeUp = 1073741952,
	KeyCode_VolumeDown = 1073741953,
	KeyCode_LCtrl = 1073742048,
	KeyCode_LShift = 1073742049,
	KeyCode_LAlt = 1073742050,
	KeyCode_RCtrl = 1073742052,
	KeyCode_RShift = 1073742053,
	KeyCode_RAlt = 1073742054,
};

class Event
{
	friend class EventDispatcher;
public:
	virtual ~Event() = default;

	virtual EventType GetType() const = 0;
	virtual const char* GetName() const = 0;
	virtual int GetCategoryFlags() const = 0;
	virtual String ToString() const
	{
		return GetName();
	}

public:
	bool handled = false;

private:
	EventType _type;
};

class QuitEvent : public Event
{
public:
	EventType GetType() const override { return GetStaticType(); }
	const char* GetName() const override { return "Quit"; }
	int GetCategoryFlags() const override { return EventCategory_None; }
	static EventType GetStaticType() { return EventType::Quit; }
};

class KeyReleaseEvent : public Event
{
public:
	KeyCode key_code;

	KeyReleaseEvent(KeyCode key_code)
		: key_code(key_code)
	{}

	String ToString() const override
	{
		StringBuilder str;
		str << "KeyRelease(" << (char)key_code << ")";
		return str.ToString();
	}

	EventType GetType() const override { return GetStaticType(); }
	const char* GetName() const override { return "KeyRelease"; }
	int GetCategoryFlags() const override { return EventCategory_Keyboard; }
	static EventType GetStaticType() { return EventType::KeyRelease; }
};

class KeyPressEvent : public Event
{
public:
	KeyCode key_code;
	int repeat_count;

	KeyPressEvent(KeyCode key_code, int repeat_count)
		: key_code(key_code)
		, repeat_count(repeat_count)
	{}

	String ToString() const override
	{
		StringBuilder str;
		str << "KeyPressed(" << (char)key_code << ", repeat = " << repeat_count << ")";
		return str.ToString();
	}

	EventType GetType() const override { return GetStaticType(); }
	const char* GetName() const override { return "KeyPress"; }
	int GetCategoryFlags() const override { return EventCategory_Keyboard; }
	static EventType GetStaticType() { return EventType::KeyPress; }
};

class MouseButtonReleaseEvent : public Event
{
public:
	int button_index;

	MouseButtonReleaseEvent(int button_index)
		: button_index(button_index)
	{}

	String ToString() const override
	{
		StringBuilder str;
		str << "MouseButtonRelease(" << button_index << ")";
		return str.ToString();
	}

	EventType GetType() const override { return GetStaticType(); }
	const char* GetName() const override { return "MouseButtonRelease"; }
	int GetCategoryFlags() const override { return EventCategory_Mouse; }
	static EventType GetStaticType() { return EventType::MouseButtonRelease; }
};

class MouseButtonPressEvent : public Event
{
public:
	int button_index;
	int click_count;

	MouseButtonPressEvent(int button_index, int click_count)
		: button_index(button_index)
		, click_count(click_count)
	{}

	String ToString() const override
	{
		StringBuilder str;
		str << "MouseButtonPress(" << button_index << ", clicks = " << click_count << ")";
		return str.ToString();
	}

	EventType GetType() const override { return GetStaticType(); }
	const char* GetName() const override { return "MouseButtonPress"; }
	int GetCategoryFlags() const override { return EventCategory_Mouse; }
	static EventType GetStaticType() { return EventType::MouseButtonPress; }
};

class MouseMoveEvent : public Event
{
public:
	int x;
	int xrel;
	int y;
	int yrel;

	MouseMoveEvent(int x, int xrel, int y, int yrel)
		: x(x)
		, xrel(xrel)
		, y(y)
		, yrel(yrel)
	{}

	String ToString() const override
	{
		StringBuilder str;
		str << "MouseMove(x = " << x << ", y = " << y << ")";
		return str.ToString();
	}

	EventType GetType() const override { return GetStaticType(); }
	const char* GetName() const override { return "MouseMove"; }
	int GetCategoryFlags() const override { return EventCategory_Mouse; }
	static EventType GetStaticType() { return EventType::MouseMove; }
};

class MouseWheelEvent : public Event
{
public:
	int x;
	int y;

	MouseWheelEvent(int x, int y)
		: x(x)
		, y(y)
	{}

	String ToString() const override
	{
		StringBuilder str;
		str << "MouseWheel(x = " << x << ", y = " << y << ")";
		return str.ToString();
	}

	EventType GetType() const override { return GetStaticType(); }
	const char* GetName() const override { return "MouseWheel"; }
	int GetCategoryFlags() const override { return EventCategory_Mouse; }
	static EventType GetStaticType() { return EventType::MouseWheel; }
};

class EventDispatcher
{
	template<typename T>
	using EventFn = std::function<bool(T&)>;
public:
	EventDispatcher(Event& event)
		: _event(event)
	{
	}

	template<typename T>
	bool Dispatch(EventFn<T> func)
	{
		if (_event.GetType() == T::GetStaticType())
		{
			_event.handled = func(static_cast<T&>(_event));
			return true;
		}
		return false;
	}
private:
	Event& _event;
};
