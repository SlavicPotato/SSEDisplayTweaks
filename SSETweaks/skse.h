#pragma once

#include <ext/ISKSE.h>
#include <ext/ISettingCollection.h>

namespace SDT
{
	static inline constexpr std::size_t MAX_TRAMPOLINE_BRANCH  = 320;
	static inline constexpr std::size_t MAX_TRAMPOLINE_CODEGEN = 1024;

	class ISKSE :
		public ISKSEBase<
			SKSEInterfaceFlags::kTrampoline |
				SKSEInterfaceFlags::kMessaging,
			512ui64,
			1536ui64>,
		public ISettingCollection
	{
	public:
		[[nodiscard]] inline static constexpr auto& GetSingleton()
		{
			return m_Instance;
		}

		[[nodiscard]] inline static constexpr auto& GetBranchTrampoline()
		{
			return m_Instance.GetTrampoline(TrampolineID::kBranch);
		}

		[[nodiscard]] inline static constexpr auto& GetLocalTrampoline()
		{
			return m_Instance.GetTrampoline(TrampolineID::kLocal);
		}

	private:
		ISKSE() = default;

		virtual const char*   GetLogPath(std::uint32_t a_version) const override;
		virtual void          OnLogOpen() override;
		virtual const char*   GetPluginName() const override;
		virtual std::uint32_t GetPluginVersion() const override;
		virtual bool          CheckRuntimeVersion(std::uint32_t a_version) const override;
		virtual bool          CheckInterfaceVersion(
					 std::uint32_t a_interfaceID,
					 std::uint32_t a_interfaceVersion,
					 std::uint32_t a_compiledInterfaceVersion) const override;

		static ISKSE m_Instance;
	};
}
