#include "pch.h"

namespace SDT
{
    DInput DInput::m_Instance;

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
        m_Instance.callbacks.push_back(h);
    }

    void DInput::MessageHandler(Event m_code, void* args)
    {
        auto message = reinterpret_cast<SKSEMessagingInterface::Message*>(args);

        switch (message->type)
        {
        case SKSEMessagingInterface::kMessage_InputLoaded:
        {
            if (m_Instance.callbacks.empty()) {
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
        for (const auto h : callbacks) {
            h->ReceiveEvent(ev, key);
        }
    }

    auto DInput::KeyPressHandler::ReceiveEvent(InputEvent** evns, InputEventDispatcher* dispatcher)
        -> EventResult
    {
        if (!*evns) {
            return kEvent_Continue;
        }

        for (InputEvent* e = *evns; e; e = e->next)
        {
            if (e->eventType == InputEvent::kEventType_Button)
            {
                ButtonEvent* t = IRTTI::Cast<ButtonEvent>(e, RTTI::InputEvent, RTTI::ButtonEvent);

                UInt32	deviceType = t->deviceType;

                if (deviceType != kDeviceType_Keyboard) {
                    continue;
                }

                UInt32	keyCode = t->keyMask;

                if (keyCode >= InputMap::kMaxMacros)
                    continue;

                if (t->flags != 0 && t->timer == 0.0)
                {
                    m_Instance.DispatchKeyEvent(KeyEvent::KeyDown, keyCode);
                }
                else if (t->flags == 0 && t->timer != 0)
                {
                    m_Instance.DispatchKeyEvent(KeyEvent::KeyUp, keyCode);
                }
            }
        }

        return kEvent_Continue;
    }
}