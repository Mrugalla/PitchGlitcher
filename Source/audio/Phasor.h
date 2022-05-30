#pragma once
#include "PRM.h"

namespace audio
{
	template<typename Float>
	struct Phasor
	{
		Phasor() :
			phase(static_cast<Float>(0)),
			inc(static_cast<Float>(0)),

			fs(static_cast<Float>(1)),
			fsInv(static_cast<Float>(1))
		{}

		void prepare(Float sampleRate) noexcept
		{
			fs = sampleRate;
			fsInv = static_cast<Float>(1) / fs;
		}

		void setFrequencyMs(Float f) noexcept { inc = static_cast<Float>(1000) / (fs * f); }
		void setFrequencySecs(Float f) noexcept { inc = static_cast<Float>(1) / (fs * f); }
		void setFrequencyHz(Float f) noexcept { inc = fsInv * f; }
		void setFrequencySamples(Float f) noexcept { inc = static_cast<Float>(1) / f; }

		bool operator()() noexcept
		{
			phase += inc;
			if (phase >= static_cast<Float>(1))
			{
				--phase;
				return true;
			}
			return false;
		}

		Float process() noexcept
		{
			phase += inc;
			if (phase >= static_cast<Float>(1))
				--phase;
			return phase;
		}

		Float phase, inc;
	protected:
		Float fs, fsInv;
	};

	template<typename Float>
	struct PhasorBuffered
	{
		PhasorBuffered() :
			phasor(),
			buffer(),
			freqP(static_cast<Float>(1))
		{}

		void prepare(Float sampleRate, int blockSize) noexcept
		{
			phasor.prepare(sampleRate);
			buffer.resize(blockSize);
			freqP.prepare(sampleRate, blockSize, 13.f);
		}

		Float* withSecs(int numSamples, float freq) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
			{
				auto f = freqP(freq);
				phasor.setFrequencySecs(f);
				buffer[s] = phasor.process();
			}

			return buffer.data();
		}

		Float* withMs(int numSamples, float freq) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
			{
				auto f = freqP(freq);
				phasor.setFrequencyMs(f);
				buffer[s] = phasor.process();
			}

			return buffer.data();
		}

		Float* withHz(int numSamples, float freq) noexcept
		{
			for (auto s = 0; s < numSamples; ++s)
			{
				auto f = freqP(freq);
				phasor.setFrequencyHz(f);
				buffer[s] = phasor.process();
			}

			return buffer.data();
		}

	protected:
		Phasor<Float> phasor;
		std::vector<Float> buffer;
		PRM freqP;
	};
}