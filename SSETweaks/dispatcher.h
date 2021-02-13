#pragma once

namespace SDT
{
    class IDDispatcher :
        ILog
    {
    public:
        static void RegisterDriver(IDriver* const);
        static bool InitializeDrivers();
        static bool DriverOK(DRIVER_ID const id);
        static IDriver* GetDriver(DRIVER_ID const id);

        template <typename T>
        [[nodiscard]] static T* GetDriver()
        {
            static_assert(std::is_base_of_v<IDriver, T>);
            return static_cast<T*>(GetDriver(T::ID));
        }

        FN_NAMEPROC("Dispatcher")
    private:
        IDDispatcher() = default;

        void PostProcessDrivers();
        bool InitializeDrivers_Impl();

        stl::vector<IDriver*> m_drivers;
        stl::unordered_map<DRIVER_ID, IDriver*> m_drivermap;

        static IDDispatcher m_Instance;
    };
}