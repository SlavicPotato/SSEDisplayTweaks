#include "pch.h"

namespace SDT
{
	DInput DInput::m_Instance;

	void ComboKeyPressHandler::ReceiveEvent(KeyEvent a_event, std::uint32_t a_keyCode)
	{
		if (a_event == KeyEvent::KeyDown)
		{
			if (m_comboKey && a_keyCode == m_comboKey)
			{
				m_comboKeyDown = true;
			}

			if (a_keyCode == m_key && (!m_comboKey || m_comboKeyDown))
			{
				OnKeyPressed();
			}
		}
		else
		{
			if (m_comboKey && a_keyCode == m_comboKey)
			{
				m_comboKeyDown = false;
			}
		}
	}

	void DInput::RegisterHooks()
	{
		IEvents::RegisterForEvent(Event::OnMessage, MessageHandler);
	}

	bool DInput::Prepare()
	{
		return true;
	}

	void DInput::RegisterForKeyEvents(KeyEventHandler* const h)
	{
		m_Instance.m_handlers.emplace_back(h);
	}

	void DInput::MessageHandler(Event m_code, void* a_args)
	{
		auto message = static_cast<SKSEMessagingInterface::Message*>(a_args);

		switch (message->type)
		{
		case SKSEMessagingInterface::kMessage_InputLoaded:
			{
				if (m_Instance.m_handlers.empty())
				{
					break;
				}

				auto evd = InputEventDispatcher::GetSingleton();
				if (evd)
				{
					evd->AddEventSink(KeyPressHandler::GetSingleton());
					m_Instance.Debug("Added input event sink");
				}
				else
				{
					m_Instance.Error("Could not get InputEventDispatcher");
				}
			}
			break;
		}
	}

	void DInput::DispatchKeyEvent(KeyEvent ev, std::uint32_t key)
	{
		for (const auto& h : m_handlers)
		{
			h->ReceiveEvent(ev, key);
		}
	}

	auto DInput::KeyPressHandler::ReceiveEvent(
		InputEvent* const*           evns,
		BSTEventSource<InputEvent*>* dispatcher)
		-> EventResult
	{
		if (!evns)
		{
			return EventResult::kContinue;
		}

		for (auto it = *evns; it; it = it->next)
		{
			auto buttonEvent = it->AsButtonEvent();
			if (!buttonEvent)
			{
				continue;
			}

			if (buttonEvent->device != INPUT_DEVICE::kKeyboard)
			{
				continue;
			}

			std::uint32_t keyCode = buttonEvent->GetIDCode();

			if (!keyCode || keyCode >= InputMap::kMaxMacros)
			{
				continue;
			}

			if (buttonEvent->IsDown())
			{
				m_Instance.DispatchKeyEvent(KeyEvent::KeyDown, keyCode);
			}
			else if (buttonEvent->IsUpLF())
			{
				m_Instance.DispatchKeyEvent(KeyEvent::KeyUp, keyCode);
			}
		}

		return EventResult::kContinue;
	}
}