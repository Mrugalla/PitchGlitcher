#pragma once
#include <vector>

namespace audio
{
	struct WHead
	{
		WHead() :
			buf(),
			wHead(0),
			delaySize(1)
		{}

		void prepare(int blockSize, int _delaySize)
		{
			delaySize = _delaySize;
			if (delaySize != 0)
			{
				wHead = wHead % delaySize;
				buf.resize(blockSize);
			}
		}

		void operator()(int numSamples) noexcept
		{
			for (auto s = 0; s < numSamples; ++s, wHead = (wHead + 1) % delaySize)
				buf[s] = wHead;
		}

		int operator[](int i) const noexcept { return buf[i]; }

		const int* data() const noexcept { return buf.data(); }
	protected:
		std::vector<int> buf;
		int wHead, delaySize;
	};
}