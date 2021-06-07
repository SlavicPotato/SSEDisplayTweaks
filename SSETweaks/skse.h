#pragma once

#include <ext/ISKSE.h>
#include <ext/ISettingCollection.h>

namespace SDT
{
    static inline constexpr std::size_t MAX_TRAMPOLINE_BRANCH = 320;
    static inline constexpr std::size_t MAX_TRAMPOLINE_CODEGEN = 1024;

    class ISKSE :
        public ISKSEBase<
        SKSEInterfaceFlags::kTrampoline |
        SKSEInterfaceFlags::kMessaging,
        320ui64,
        1024ui64>,
        public ISettingCollection
    {
    public:

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetSingleton() {
            return m_Instance;
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetBranchTrampoline() {
            return m_Instance.GetTrampoline(TrampolineID::kBranch);
        }

        [[nodiscard]] SKMP_FORCEINLINE static auto& GetLocalTrampoline() {
            return m_Instance.GetTrampoline(TrampolineID::kLocal);
        }

    private:
        ISKSE() = default;

        virtual void OnLogOpen() override;
        virtual const char* GetLogPath() const override;
        virtual const char* GetPluginName() const override;
        virtual UInt32 GetPluginVersion() const override;
        virtual bool CheckRuntimeVersion(UInt32 a_version) const override;
        virtual bool CheckInterfaceVersion(UInt32 a_interfaceID, UInt32 a_interfaceVersion, UInt32 a_compiledInterfaceVersion) const override;

        static ISKSE m_Instance;
    };
}

