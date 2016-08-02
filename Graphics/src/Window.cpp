#include "stdafx.h"
#include "Window.hpp"
#include "KeyMap.hpp"
#include <SDL2/SDL_syswm.h>

namespace Graphics
{
	/* SDL Instance singleton */
	class SDL
	{
	protected:
		SDL()
		{
			SDL_SetMainReady();
			int r = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
			assert(r == 0);
		}
	public:
		~SDL()
		{
			SDL_Quit();
		}
		static SDL& Main()
		{
			static SDL sdl;
			return sdl;
		}
	};

	class Window_Impl
	{
	public:
		// Handle to outer class to send delegates
        DesktopWindow& outer;
	public:
        Window_Impl(DesktopWindow& outer, Vector2i size) : outer(outer)
		{
			SDL::Main();

			// Initialize button mapping
			m_keyMapping.AddRangeMapping('a', 'z', Key::A);
			m_keyMapping.AddRangeMapping('0', '9', Key::Top0);
			m_keyMapping.AddRangeMapping(SDLK_F1, SDLK_F12, Key::F1);
			m_keyMapping.AddMapping(SDLK_PRINTSCREEN, Key::PrntScr);
			m_keyMapping.AddMapping(SDLK_SCROLLLOCK, Key::ScrollLock);
			m_keyMapping.AddMapping(SDLK_PAUSE, Key::Pause);
			m_keyMapping.AddMapping(SDLK_ESCAPE, Key::Escape);
			m_keyMapping.AddMapping(SDLK_BACKQUOTE, Key::Tilde);
			m_keyMapping.AddMapping(SDLK_PAGEUP, Key::PageUp);
			m_keyMapping.AddMapping(SDLK_PAGEDOWN, Key::PageDown);
			m_keyMapping.AddMapping(SDLK_RETURN, Key::Return);
			m_keyMapping.AddMapping(SDLK_PLUS, Key::Plus);
			m_keyMapping.AddMapping(SDLK_MINUS, Key::Minus);
			m_keyMapping.AddMapping(SDLK_LEFT, Key::ArrowLeft);
			m_keyMapping.AddMapping(SDLK_RIGHT, Key::ArrowRight);
			m_keyMapping.AddMapping(SDLK_UP, Key::ArrowUp);
			m_keyMapping.AddMapping(SDLK_DOWN, Key::ArrowDown);
			m_keyMapping.AddMapping(SDLK_SPACE, Key::Space);
			m_keyMapping.AddMapping(SDLK_BACKSPACE, Key::Backspace);

			m_clntSize = size;

			m_caption = L"Window";
			String titleUtf8 = Utility::ConvertToUTF8(m_caption);
			m_window = SDL_CreateWindow(*titleUtf8, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
				m_clntSize.x, m_clntSize.y, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
			assert(m_window);
		}
		~Window_Impl()
		{
			SDL_DestroyWindow(m_window);
		}

		void SetWindowPos(const Vector2i& pos)
		{
			SDL_SetWindowPosition(m_window, pos.x, pos.y);
		}
		Vector2i GetWindowPos() const
		{
			Vector2i res;
			SDL_GetWindowPosition(m_window, &res.x, &res.y);
			return res;
		}

		void SetWindowSize(const Vector2i& size)
		{
			SDL_SetWindowSize(m_window, size.x, size.y);
		}
		Vector2i GetWindowSize() const
		{
			Vector2i res;
			SDL_GetWindowSize(m_window, &res.x, &res.y);
			return res;
		}

		void SetWindowStyle(WindowStyle style)
		{
		}

		/* input handling */
		void HandleKeyEvent(SDL_Keycode code, uint8 newState, int32 repeat)
		{
			SDL_Keymod m = SDL_GetModState();
			m_modKeys = ModifierKeys::None;
			if((m & KMOD_ALT) != 0)
			{
				(uint8&)m_modKeys |= (uint8)ModifierKeys::Alt;
			}
			if((m & KMOD_CTRL) != 0)
			{
				(uint8&)m_modKeys |= (uint8)ModifierKeys::Ctrl;
			}
			if((m & KMOD_SHIFT) != 0)
			{
				(uint8&)m_modKeys |= (uint8)ModifierKeys::Shift;
			}

			Graphics::Key keyCode = m_keyMapping.Translate(code);
			if(keyCode == Key::None)
				return; // Not mapped

			uint8& currentState = m_keyStates[(uint8)keyCode];
			if(currentState != newState)
			{
				currentState = newState;
				if(newState == 1)
				{
					outer.OnKeyPressed.Call(keyCode);
				}
				else
				{
					outer.OnKeyReleased.Call(keyCode);
				}
			}
			if(currentState == 1)
			{
				outer.OnKeyRepeat.Call(keyCode);
			}
		}

		/* Window show hide, positioning, etc.*/
		void Show()
		{
			SDL_ShowWindow(m_window);
		}
		void Hide()
		{
			SDL_HideWindow(m_window);
		}
		void SetCaption(const WString& cap)
		{
			m_caption = L"Window";
			String titleUtf8 = Utility::ConvertToUTF8(m_caption);
			SDL_SetWindowTitle(m_window, *titleUtf8);
		}

		// Update loop
		Timer t;
		bool Update()
		{
			SDL_Event evt;
			while(SDL_PollEvent(&evt))
			{
				if(evt.type == SDL_EventType::SDL_KEYDOWN)
				{
					if(m_textComposition.composition.empty())
					{
						// Ignore key input when composition is being typed
						HandleKeyEvent(evt.key.keysym.sym, 1, evt.key.repeat);
					}
				}
				else if(evt.type == SDL_EventType::SDL_KEYUP)
				{
					HandleKeyEvent(evt.key.keysym.sym, 0, 0);
				}
				else if(evt.type == SDL_EventType::SDL_QUIT)
				{
					m_closed = true;
				}
				else if(evt.type == SDL_EventType::SDL_WINDOWEVENT)
				{
					if(evt.window.windowID == SDL_GetWindowID(m_window))
					{
						if(evt.window.event == SDL_WindowEventID::SDL_WINDOWEVENT_RESIZED)
						{
							Vector2i newSize(evt.window.data1, evt.window.data2);
							outer.OnResized.Call(newSize);
						}
					}
				}
				else if(evt.type == SDL_EventType::SDL_TEXTINPUT)
				{
					WString wstr = Utility::ConvertToWString(evt.text.text);
					outer.OnTextInput.Call(wstr);
				}
				else if(evt.type == SDL_EventType::SDL_TEXTEDITING)
				{
					SDL_Rect scr;
					SDL_GetWindowPosition(m_window, &scr.x, &scr.y);
					SDL_GetWindowSize(m_window, &scr.w, &scr.h);
					SDL_SetTextInputRect(&scr);

					m_textComposition.composition = Utility::ConvertToWString(evt.edit.text);
					m_textComposition.cursor = evt.edit.start;
					m_textComposition.selectionLength = evt.edit.length;
					outer.OnTextComposition.Call(m_textComposition);
				}
			}
			return !m_closed;
		}

		void SwitchFullscreen(uint32 monitorID)
		{
			if(m_fullscreen)
			{
				SDL_SetWindowFullscreen(m_window, 0);
				m_fullscreen = false;
			}
			else
			{
				SDL_SetWindowFullscreen(m_window, SDL_WINDOW_FULLSCREEN);
				m_fullscreen = true;
			}
		}

		SDL_Window* m_window;

		// Window Input State
		uint8 m_keyStates[256] = { 0 };
		KeyMap m_keyMapping;
		ModifierKeys m_modKeys = ModifierKeys::None;

		// Text input / IME stuff
		TextComposition m_textComposition;

		// Various window state
		bool m_active = true;
		bool m_closed = false;
		bool m_fullscreen = false;
		uint32 m_style;
		Vector2i m_clntSize;
		WString m_caption;
	};
    DesktopWindow::DesktopWindow(Vector2i size)
	{
		m_impl = new Window_Impl(*this, size);
	}
	DesktopWindow::~DesktopWindow()
	{
		delete m_impl;
	}
	void DesktopWindow::Show()
	{
		m_impl->Show();
	}
	void DesktopWindow::Hide()
	{
		m_impl->Hide();
	}
	bool DesktopWindow::Update()
	{
		return m_impl->Update();
	}
	void* DesktopWindow::Handle()
	{
		return m_impl->m_window;
	}
	void DesktopWindow::SetCaption(const WString& cap)
	{
		m_impl->SetCaption(cap);
	}
	void DesktopWindow::Close()
	{
		m_impl->m_closed = true;
	}
	void DesktopWindow::SetWindowStyle(WindowStyle style)
	{
		m_impl->SetWindowStyle(style);
	}
	Vector2i DesktopWindow::GetWindowPos() const
	{
		return m_impl->GetWindowPos();
	}
	void DesktopWindow::SetWindowPos(const Vector2i& pos)
	{
		m_impl->SetWindowPos(pos);
	}

	Vector2i DesktopWindow::GetWindowSize() const
{
		return m_impl->GetWindowSize();
	}
	void DesktopWindow::SetWindowSize(const Vector2i& size)
	{
		m_impl->SetWindowSize(size);
	}
	void DesktopWindow::SwitchFullscreen(uint32 monitorID)
	{
		m_impl->SwitchFullscreen(monitorID);
	}
	bool DesktopWindow::IsKeyPressed(Key key) const
	{
		return m_impl->m_keyStates[(uint8)key] > 0;
	}

	Graphics::ModifierKeys DesktopWindow::GetModifierKeys() const
	{
		return m_impl->m_modKeys;
	}

	void DesktopWindow::StartTextInput()
	{
		SDL_StartTextInput();
	}
	void DesktopWindow::StopTextInput()
	{
		SDL_StopTextInput();
	}
	const Graphics::TextComposition& DesktopWindow::GetTextComposition() const
	{
		return m_impl->m_textComposition;
	}
}

namespace Graphics
{
	ModifierKeys operator&(ModifierKeys l, ModifierKeys r)
	{
		return (ModifierKeys)((uint8)l & (uint8)r);
	}
	ModifierKeys operator|(ModifierKeys l, ModifierKeys r)
	{
		return (ModifierKeys)((uint8)l | (uint8)r);
	}
}