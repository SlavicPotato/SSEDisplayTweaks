#pragma once

#define FN_ESSENTIAL(x) \
	virtual bool IsEssential() const { return x; };
#define FN_DRVDEF(x)                               \
	virtual int GetPriority() const { return x; }; \
	virtual DRIVER_ID GetID() const noexcept { return std::remove_reference_t<decltype(*this)>::ID; }
