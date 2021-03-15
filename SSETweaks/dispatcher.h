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

        template <class T>
        [[nodiscard]] static auto GetDriver();

        FN_NAMEPROC("Dispatcher");
    private:
        IDDispatcher() = default;

        void PreProcessDrivers();
        bool InitializeDrivers_Impl();

        std::vector<IDriver*> m_drivers;
        std::unordered_map<DRIVER_ID, IDriver*> m_drivermap;

        static IDDispatcher m_Instance;
    };

    template <class T>
    auto IDDispatcher::GetDriver()
    {
        using type = std::remove_all_extents_t<std::remove_reference_t<std::remove_cv_t<T>>>;

        static_assert(std::is_base_of_v<IDriver, type>);

        return static_cast<type*>(GetDriver(type::ID));
    }

}