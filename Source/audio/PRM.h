#pragma once
#include "../arch/Smooth.h"

namespace audio
{
	struct PRM
	{
		PRM(float startVal) :
			smooth(startVal),
			buf()
		{}

		void prepare(float Fs, int blockSize, float smoothLenMs)
		{
			buf.resize(blockSize);
			smooth.makeFromDecayInMs(smoothLenMs, Fs);
		}

		float* operator()(float value, int numSamples) noexcept
		{
			smooth(buf.data(), value, numSamples);
			return buf.data();
		}

		float* operator()(int numSamples) noexcept
		{
			smooth(buf.data(), numSamples);
			return buf.data();
		}

		float operator()(float value) noexcept
		{
			return smooth(value);
		}

		smooth::Smooth<float> smooth;
		std::vector<float> buf;
	};
}