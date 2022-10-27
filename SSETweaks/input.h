#pragma once

namespace SDT
{
	enum KeyEvent
	{
		KeyDown = 0,
		KeyUp   = 1
	};

	typedef void (*KeyEventCallback)(KeyEvent, std::uint32_t);

	class KeyEventHandler
	{
	public:
		virtual void ReceiveEvent(KeyEvent, std::uint32_t) = 0;
	};

	class ComboKeyPressHandler :
		public KeyEventHandler
	{
	public:
		ComboKeyPressHandler() :
			m_comboKey(0),
			m_key(0),
			m_comboKeyDown(false)
		{}

		SKMP_FORCEINLINE void SetComboKey(std::uint32_t a_key)
		{
			m_comboKey     = a_key;
			m_comboKeyDown = false;
		}

		SKMP_FORCEINLINE void SetKey(std::uint32_t a_key)
		{
			m_key = a_key;
		}

		SKMP_FORCEINLINE void SetKeys(std::uint32_t a_comboKey, std::uint32_t a_key)
		{
			m_comboKey     = a_comboKey;
			m_key          = a_key;
			m_comboKeyDown = false;
		}

		virtual void OnKeyPressed() = 0;

	protected:
	private:
		bool m_comboKeyDown;

		std::uint32_t m_comboKey;
		std::uint32_t m_key;

		virtual void ReceiveEvent(KeyEvent a_event, std::uint32_t a_keyCode) override;
	};

	class DInput :
		public IDriver,
		IConfig
	{
		class KeyPressHandler : public BSTEventSink<InputEvent*>
		{
		public:
			virtual EventResult ReceiveEvent(InputEvent* const* evns, BSTEventSource<InputEvent*>* dispatcher) override;

			static KeyPressHandler* GetSingleton()
			{
				static KeyPressHandler inputEventHandler;
				return &inputEventHandler;
			}
		};

	public:
		static inline constexpr auto ID = DRIVER_ID::INPUT;

		static void RegisterForKeyEvents(KeyEventHandler* const handler);

		FN_NAMEPROC("Input");
		FN_ESSENTIAL(false);
		FN_DRVDEF(1);

	private:
		DInput() = default;

		virtual void RegisterHooks() override;
		virtual bool Prepare() override;

		static void MessageHandler(Event m_code, void* a_args);

		void DispatchKeyEvent(KeyEvent ev, std::uint32_t key);

		std::vector<KeyEventHandler*> m_handlers;

		static DInput m_Instance;
	};
}