#pragma once

namespace SDT 
{
	class JITASM
		: public Xbyak::CodeGenerator
	{
	public:
		JITASM(size_t maxSize = Xbyak::DEFAULT_MAX_CODE_SIZE);
		virtual ~JITASM() noexcept;

		void done();
		uintptr_t get();

	private:
		bool _endedAlloc;
	};
}