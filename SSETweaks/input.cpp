#include "pch.h"

namespace SDT
{
    DInput DInput::m_Instance;

    void ComboKeyPressHandler::ReceiveEvent(KeyEvent a_event, UInt32 a_keyCode)
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
            if (m_Instance.m_handlers.empty()) {
                break;
            }

            auto evd = InputEventDispatcher::GetSingleton();
            if (evd) {
                evd->AddEventSink(KeyPressHandler::GetSingleton());
                m_Instance.Debug("Added input event sink");
            }
            else {
                m_Instance.Error("Could not get InputEventDispatcher");
            }
        }
        break;
        }
    }

    void DInput::DispatchKeyEvent(KeyEvent ev, UInt32 key)
    {
        for (const auto& h : m_handlers) {
            h->ReceiveEvent(ev, key);
        }
    }

    auto DInput::KeyPressHandler::ReceiveEvent(InputEvent** evns, InputEventDispatcher* dispatcher)
        -> EventResult
    {
        if (!*evns) {
            return kEvent_Continue;
        }

        for (auto inputEvent = *evns; inputEvent; inputEvent = inputEvent->next)
        {
            if (inputEvent->eventType != InputEvent::kEventType_Button)
                continue;

            auto buttonEvent = RTTI<ButtonEvent>::Cast(inputEvent);
            if (!buttonEvent)
                continue;

            if (buttonEvent->deviceType != kDeviceType_Keyboard) {
                continue;
            }

            UInt32 keyCode = buttonEvent->keyMask;

            if (keyCode >= InputMap::kMaxMacros)
                continue;

            if (buttonEvent->flags != 0)
            {
                if (buttonEvent->timer == 0.0f)
                    m_Instance.DispatchKeyEvent(KeyEvent::KeyDown, keyCode);
            }
            else
            {
                m_Instance.DispatchKeyEvent(KeyEvent::KeyUp, keyCode);
            }

        }

        return kEvent_Continue;
    }
}