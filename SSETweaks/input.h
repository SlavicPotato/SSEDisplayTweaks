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
        virtual void ReceiveEvent(KeyEvent, UInt32) {};
    };

    // E8 
    class InputEventDispatcherEx :
        public EventDispatcherEx<InputEvent, InputEvent*>
    {
    public:
        UInt32			unk058;			// 058
        UInt32			pad05C;			// 05C
        BSInputDevice* keyboard;		// 060 
        BSInputDevice* mouse;		// 068
        BSInputDevice* gamepad;		// 070
        BSInputDevice* vkeyboard;	// 078	- New in SE  .?AVBSWin32VirtualKeyboardDevice@@
        UInt8			unk080;			// 080
        UInt8			unk081;			// 081
        UInt8			pad082[6];		// 082
        BSTEventSource<void*>	unk088;	// 088	- TODO: template type
        UInt8			unk0E0;			// 0E0
        UInt8			pad0E1[7];		// 0E1

        bool	IsGamepadEnabled(void);

        static InputEventDispatcherEx* GetSingleton();
    private:
        inline static auto inputEventDispatcher = IAL::Addr<InputEventDispatcherEx**>(516574);
    };
    STATIC_ASSERT(offsetof(InputEventDispatcherEx, gamepad) == 0x70);
    STATIC_ASSERT(sizeof(InputEventDispatcherEx) == 0xE8);

    template <>
    class BSTEventSinkEx <InputEvent>
    {
    public:
        virtual ~BSTEventSinkEx() {};
        virtual	EventResult ReceiveEvent(InputEvent** evn, InputEventDispatcherEx* dispatcher) = 0;
    };

    class DInput :
        public IDriver,
        IConfig
    {
        class KeyPressHandler : public BSTEventSinkEx <InputEvent>
        {
        public:
            virtual EventResult	ReceiveEvent(InputEvent** evns, InputEventDispatcherEx* dispatcher) override;

            static KeyPressHandler* GetSingleton() {
                static KeyPressHandler inputEventHandler;
                return &inputEventHandler;
            }
        };

    public:
        static void RegisterForKeyEvents(KeyEventHandler* const handler);

        FN_NAMEPROC("Input")
        FN_ESSENTIAL(false)
        FN_PRIO(1)
        FN_DRVID(DRIVER_INPUT)
    private:
        DInput() = default;

        virtual void RegisterHooks() override;
        virtual bool Prepare() override;

        static void MessageHandler(Event m_code, void* args);

        void DispatchKeyEvent(KeyEvent ev, UInt32 key);

        std::vector<KeyEventHandler*> callbacks;

        static DInput m_Instance;
    };
}