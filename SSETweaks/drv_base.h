#pragma once

namespace SDT {
    class HookDescriptor
    {
    public:
        enum HookType {
            //kDetours,
            kWR5Call,
            kWR6Call
        };

        /*HookDescriptor(PVOID *target, PVOID detour) :
            target(target), detour(detour), type(HookType::kDetours)
        {
        }*/

        HookDescriptor(uintptr_t target, uintptr_t hook, HookType type) :
            wc_target(target), wc_hook(hook), type(type)
        {
        }

        /*PVOID *target;
        PVOID detour;*/

        uintptr_t wc_target;
        uintptr_t wc_hook;

        HookType type;
    };

    class IHook :
        protected ILog
    {
    protected:
        IHook() = default;
        virtual ~IHook() = default;

        //void RegisterHook(PVOID *target, PVOID detour);
        void RegisterHook(uintptr_t target, uintptr_t hook);
        void RegisterHook(uintptr_t target, uintptr_t hook, HookDescriptor::HookType type);
        void RegisterHook(HookDescriptor const& hdesc);
        bool InstallHooks();
    private:
        std::vector<HookDescriptor> hooks;
    };

    class IDriver :
        protected IHook
    {
        friend class IDDispatcher;
    public:
        bool IsInitialized() { return b_Initialized; }
        bool IsOK() { return b_IsOK; }

        IDriver(IDriver&) = delete;
        IDriver(const IDriver&&) = delete;

        IDriver& operator=(IDriver&) = delete;
        void operator=(const IDriver&&) = delete;

        FN_NAMEPROC("IDriver")
        FN_ESSENTIAL(false)
        FN_PRIO(99)
        FN_DRVID(INVALID_DRIVER)
    protected:
        IDriver();
        virtual ~IDriver() = default;

        //void SetPapyrusFuncRegProc(SKSEPapyrusInterface::RegisterFunctions proc);

        bool Initialize();
        void SetOK(bool b) { b_IsOK = b; }
        virtual bool Prepare() { return false; };

    private:
        virtual void LoadConfig() {};
        virtual void PostLoadConfig() {};
        virtual void Patch() {};
        virtual void RegisterHooks() {};

        //void RegisterPapyrusFunctions();

        //SKSEPapyrusInterface::RegisterFunctions papyrusFuncRegProc = nullptr;

        bool b_Initialized = false;
        bool b_IsOK = false;
    };

}