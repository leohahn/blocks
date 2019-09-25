#pragma once

#include "Han/Core.hpp"
#include "Han/Collections/String.hpp"
#include "Han/StringBuilder.hpp"
#include <functional>

//
// Design basically taken from Hazel: https://github.com/TheCherno/Hazel/blob/master/Hazel/src/Hazel/Events/Event.h
//

#define HAN_BIND_EV_HANDLER(name) std::bind(&name, this, std::placeholders::_1)

enum class EventType
{
	MouseButtonPress,
	MouseButtonRelease,
	MouseMove,
	MouseWheel,
	TextInput,
	KeyPress,
	KeyRelease,
	Quit,
	WindowResize,
};

enum EventCategory
{
	EventCategory_None     = 0,
	EventCategory_Mouse    = HAN_BIT(0),
	EventCategory_Keyboard = HAN_BIT(1),
	EventCategory_Window   = HAN_BIT(2),
};

enum KeyCode
{
    KeyCode_Unknown =          -1,

    /* Printable keys */
    KeyCode_Space =            32,
    KeyCode_Apostrophe =       39,  /* ' */
    KeyCode_Comma =            44,  /* , */
    KeyCode_Minus =            45,  /* - */
    KeyCode_Period =           46,  /* . */
    KeyCode_Slash =            47,  /* / */
    KeyCode_0 =                48,
    KeyCode_1 =                49,
    KeyCode_2 =                50,
    KeyCode_3 =                51,
    KeyCode_4 =                52,
    KeyCode_5 =                53,
    KeyCode_6 =                54,
    KeyCode_7 =                55,
    KeyCode_8 =                56,
    KeyCode_9 =                57,
    KeyCode_Semicolon =        59,  /* ; */
    KeyCode_Equal =            61,  /* = */
    KeyCode_A =                65,
    KeyCode_B =                66,
    KeyCode_C =                67,
    KeyCode_D =                68,
    KeyCode_E =                69,
    KeyCode_F =                70,
    KeyCode_G =                71,
    KeyCode_H =                72,
    KeyCode_I =                73,
    KeyCode_J =                74,
    KeyCode_K =                75,
    KeyCode_L =                76,
    KeyCode_M =                77,
    KeyCode_N =                78,
    KeyCode_O =                79,
    KeyCode_P =                80,
    KeyCode_Q =                81,
    KeyCode_R =                82,
    KeyCode_S =                83,
    KeyCode_T =                84,
    KeyCode_U =                85,
    KeyCode_V =                86,
    KeyCode_W =                87,
    KeyCode_X =                88,
    KeyCode_Y =                89,
    KeyCode_Z =                90,
    KeyCode_LeftBracket =      91,  /* [ */
    KeyCode_Backslash =        92,  /* \ */
    KeyCode_RightBracket =     93,  /* ] */
    KeyCode_GraveAccent =      96,  /* ` */
    
    /* Function keys */
    KeyCode_Escape =           256,
    KeyCode_Enter =            257,
    KeyCode_Tab =              258,
    KeyCode_Backspace =        259,
    KeyCode_Insert =           260,
    KeyCode_Delete =           261,
    KeyCode_Right =            262,
    KeyCode_Left =             263,
    KeyCode_Down =             264,
    KeyCode_Up =               265,
    KeyCode_PageUp =           266,
    KeyCode_PageDown =         267,
    KeyCode_Home =             268,
    KeyCode_End =              269,
    KeyCode_CapsLock =         280,
    KeyCode_ScrollLock =       281,
    KeyCode_NumLock =          282,
    KeyCode_PrintScreen =      283,
    KeyCode_Pause =            284,
    KeyCode_F1 =               290,
    KeyCode_F2 =               291,
    KeyCode_F3 =               292,
    KeyCode_F4 =               293,
    KeyCode_F5 =               294,
    KeyCode_F6 =               295,
    KeyCode_F7 =               296,
    KeyCode_F8 =               297,
    KeyCode_F9 =               298,
    KeyCode_F10 =              299,
    KeyCode_F11 =              300,
    KeyCode_F12 =              301,
    KeyCode_F13 =              302,
    KeyCode_F14 =              303,
    KeyCode_F15 =              304,
    KeyCode_F16 =              305,
    KeyCode_F17 =              306,
    KeyCode_F18 =              307,
    KeyCode_F19 =              308,
    KeyCode_F20 =              309,
    KeyCode_F21 =              310,
    KeyCode_F22 =              311,
    KeyCode_F23 =              312,
    KeyCode_F24 =              313,
    KeyCode_F25 =              314,
    KeyCode_Kp0 =              320,
    KeyCode_Kp1 =              321,
    KeyCode_Kp2 =              322,
    KeyCode_Kp3 =              323,
    KeyCode_Kp4 =              324,
    KeyCode_Kp5 =              325,
    KeyCode_Kp6 =              326,
    KeyCode_Kp7 =              327,
    KeyCode_Kp8 =              328,
    KeyCode_Kp9 =              329,
    KeyCode_KpDecimal =        330,
    KeyCode_KpDivide =         331,
    KeyCode_KpMultiply =       332,
    KeyCode_KpSubtract =       333,
    KeyCode_KpAdd =            334,
    KeyCode_KpEnter =          335,
    KeyCode_KpEqual =          336,
    KeyCode_LeftShift =        340,
    KeyCode_LeftControl =      341,
    KeyCode_LeftAlt =          342,
    KeyCode_LeftSuper =        343,
    KeyCode_RightShift =       344,
    KeyCode_RightControl =     345,
    KeyCode_RightAlt =         346,
    KeyCode_RightSuper =       347,
	KeyCode_Menu =             348,
};

enum KeyMod
{
	KeyMod_None  = 0,
	KeyMod_Shift = HAN_BIT(0),
	KeyMod_Ctrl  = HAN_BIT(1),
	KeyMod_Alt   = HAN_BIT(2),
	KeyMod_Super = HAN_BIT(3),
};

enum MouseButton
{
	MouseButton_Left = 0,
	MouseButton_Right = 1,
	MouseButton_Middle = 2,
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
        return String(GetName());
	}

public:
	bool handled = false;
};

class QuitEvent : public Event
{
public:
	EventType GetType() const override { return GetStaticType(); }
	const char* GetName() const override { return "Quit"; }
	int GetCategoryFlags() const override { return EventCategory_None; }
	static EventType GetStaticType() { return EventType::Quit; }
};

class WindowResizeEvent : public Event
{
public:
	int width;
	int height;

	WindowResizeEvent(int w, int h)
		: width(w)
		, height(h)
	{}

	String ToString() const override
	{
		StringBuilder str;
		str << "WindowResize(" << width << ", " << height << ")";
		return str.ToString();
	}

	EventType GetType() const override { return GetStaticType(); }
	const char* GetName() const override { return "WindowResize"; }
	int GetCategoryFlags() const override { return EventCategory_Window; }
	static EventType GetStaticType() { return EventType::WindowResize; }
};

class KeyReleaseEvent : public Event
{
public:
	KeyCode key_code;
	int mod_flags;

	KeyReleaseEvent(KeyCode key_code, int mod_flags)
		: key_code(key_code)
		, mod_flags(mod_flags)
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
	int mod_flags;

	KeyPressEvent(KeyCode key_code, int repeat_count, int mod_flags)
		: key_code(key_code)
		, repeat_count(repeat_count)
		, mod_flags(mod_flags)
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

class TextInputEvent : public Event
{
public:
	char text[32];

	TextInputEvent(char c[32])
	{
		memcpy(text, c, ARRAY_SIZE(text));
	}

	String ToString() const override
	{
		StringBuilder str;
		str << "TextInput(" << text << ")";
		return str.ToString();
	}

	EventType GetType() const override { return GetStaticType(); }
	const char* GetName() const override { return "TextInput"; }
	int GetCategoryFlags() const override { return EventCategory_Keyboard; }
	static EventType GetStaticType() { return EventType::TextInput; }
};

class MouseButtonReleaseEvent : public Event
{
public:
	MouseButton button;

	MouseButtonReleaseEvent(MouseButton button)
		: button(button)
	{}

	String ToString() const override
	{
		StringBuilder str;
		str << "MouseButtonRelease(" << (int)button << ")";
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
	MouseButton button;
	int click_count;

	MouseButtonPressEvent(MouseButton button, int click_count)
		: button(button)
		, click_count(click_count)
	{}

	String ToString() const override
	{
		StringBuilder str;
		str << "MouseButtonPress(" << (int)button << ", clicks = " << click_count << ")";
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
