#include "pch.h"

#pragma warning(disable: 4073)
#pragma init_seg(lib)

namespace SDT
{
    IDDispatcher IDDispatcher::m_Instance;

    void IDDispatcher::RegisterDriver(IDriver* const drv)
    {
        m_Instance.drivers.push_back(drv);
    }

    bool IDDispatcher::DriverOK(uint32_t id)
    {
        return m_Instance.drivermap[id]->IsOK();
    }

    IDriver* IDDispatcher::GetDriver(uint32_t id)
    {
        if (m_Instance.drivermap.count(id)) {
            return m_Instance.drivermap[id];
        }
        else {
            return nullptr;
        }
    }

    bool IDDispatcher::InitializeDrivers()
    {
        return m_Instance.InitializeDrivers_Impl();
    }

    bool IDDispatcher::InitializeDrivers_Impl()
    {
        PostProcessDrivers();

        std::sort(drivers.begin(), drivers.end(),
            [](const auto a, const auto b) -> bool
            {
                return a->GetPriority() < b->GetPriority();
            });

        for (const auto drv : drivers)
        {
            drv->SetOK(drv->Prepare());
            if (drv->IsOK()) {
                continue;
            }

            if (drv->IsEssential()) {
                FatalError("Essential driver check failed: %s", drv->ModuleName());
                return false;
            }
            else {
                Error("Driver check failed: %s", drv->ModuleName());
            }
        }

        size_t count = 0;
        for (const auto drv : drivers)
        {
            if (!drv->IsOK()) {
                continue;
            }

            Debug("Initializing: %s", drv->ModuleName());
            if (!drv->Initialize()) {
                FatalError("Driver initialization failed: %s", drv->ModuleName());
                return false;
            }
            count++;
        }

        Message("%zu drivers initialized", count);

        return true;
    }

    void IDDispatcher::PostProcessDrivers()
    {
        for (const auto drv : drivers) {
            int driver_id = drv->GetID();

            ASSERT(driver_id > -1);
            ASSERT(!drivermap.count(driver_id));

            drivermap[driver_id] = drv;
        }
    }
}