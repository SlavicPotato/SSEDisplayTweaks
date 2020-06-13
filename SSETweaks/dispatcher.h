#pragma once

namespace SDT
{
    class IDDispatcher :
        ILog
    {
    public:
        static void RegisterDriver(IDriver* const);
        static bool InitializeDrivers();
        static bool DriverOK(uint32_t id);
        static IDriver* GetDriver(uint32_t id);

        template <typename T>
        static T* GetDriver(int id)
        {
            return reinterpret_cast<T*>(GetDriver(id));
        }

        FN_NAMEPROC("Dispatcher")
    private:
        IDDispatcher() = default;

        void PostProcessDrivers();
        bool InitializeDrivers_Impl();

        std::vector<IDriver*> drivers;
        std::unordered_map<uint32_t, IDriver*> drivermap;

        static IDDispatcher m_Instance;
    };
}