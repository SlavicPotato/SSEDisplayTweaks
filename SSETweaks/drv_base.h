#pragma once

namespace SDT 
{
    class HookDescriptor
    {
    public:
        enum class HookType : uint8_t {
            kWR5Call,
            kWR6Call
        };

        HookDescriptor(uintptr_t target, uintptr_t hook, HookType type) :
            wc_target(target), wc_hook(hook), type(type)
        {
        }

        uintptr_t wc_target;
        uintptr_t wc_hook;

        HookType type;
    };

    class IHook :
        protected ILog
    {
    protected:
        IHook() = default;
        virtual ~IHook() noexcept = default;

        void RegisterHook(uintptr_t target, uintptr_t hook);
        void RegisterHook(uintptr_t target, uintptr_t hook, HookDescriptor::HookType type);
        void RegisterHook(const HookDescriptor & hdesc);
        void RegisterHook(HookDescriptor&& hdesc);
        bool InstallHooks();
    private:
        stl::vector<HookDescriptor> m_hooks;
    };

    class IDriver :
        protected IHook
    {
        friend class IDDispatcher;
    public:
        static inline constexpr auto ID = DRIVER_ID::INVALID;

        SKMP_FORCEINLINE bool IsInitialized() const { return m_Initialized; }
        SKMP_FORCEINLINE bool IsOK() const { return m_IsOK; }

        IDriver(const IDriver&) = delete;
        IDriver(IDriver&&) = delete;
        IDriver& operator=(const IDriver&) = delete;
        void operator=(IDriver&&) = delete;

        FN_NAMEPROC("IDriver")
        FN_ESSENTIAL(false)
        FN_DRVDEF(-1)
    protected:
        IDriver();
        virtual ~IDriver() noexcept = default;

        SKMP_FORCEINLINE void SetOK(bool b) { m_IsOK = b; }

    private:
        virtual void LoadConfig() {};
        virtual void PostLoadConfig() {};
        virtual void Patch() {};
        virtual void RegisterHooks() {};
        [[nodiscard]] virtual bool Prepare() { return false; };

        [[nodiscard]] bool Initialize();

        bool m_Initialized;
        bool m_IsOK;
    };

}