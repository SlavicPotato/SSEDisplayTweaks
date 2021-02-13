#pragma once

namespace SDT
{
    enum KeyEvent
    {
        KeyDown = 0,
        KeyUp = 1
    };

    typedef void (*KeyEventCallback)(KeyEvent, UInt32);

    class KeyEventHandler
    {
    public:
        virtual void ReceiveEvent(KeyEvent, UInt32) = 0;
    };

    class DInput :
        public IDriver,
        IConfig
    {
        class KeyPressHandler : public BSTEventSink <InputEvent>
        {
        public:
            virtual EventResult	ReceiveEvent(InputEvent** evns, InputEventDispatcher* dispatcher) override;

            static KeyPressHandler* GetSingleton() {
                static KeyPressHandler inputEventHandler;
                return &inputEventHandler;
            }
        };

    public:
        static inline constexpr auto ID = DRIVER_ID::INPUT;

        static void RegisterForKeyEvents(KeyEventHandler* const handler);

        FN_NAMEPROC("Input")
        FN_ESSENTIAL(false)
        FN_DRVDEF(1)
    private:
        DInput() = default;

        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        static void MessageHandler(Event m_code, void* args);

        void DispatchKeyEvent(KeyEvent ev, UInt32 key);

        stl::vector<KeyEventHandler*> callbacks;

        static DInput m_Instance;
    };
}