#pragma once
#include "WHead.h"
#include "EnvelopeFollower.h"
#include <array>

#include "config.h"

namespace audio
{
	class Meters
	{
		static constexpr float RiseInMs = .01f, FallInMs = 42.f;

		struct Val
		{
			Val() :
				rect(0.f),
				val(0.f),
				env(0.f),
				envFol()
			{}

			float rect, val;
			std::atomic<float> env;
			EnvFol envFol;
		};
	public:
		enum Type
		{
#if PPDHasGainIn
			In,
#endif
			Out,
			NumTypes
		};
	
		Meters() :
			vals(),
			wHead(),
			lenInv(1.f),
			length(1)
		{
		}

		void prepare(float sampleRate, int blockSize)
		{
			length = static_cast<int>(sampleRate / PPDFPSMeters);
			wHead.prepare(blockSize, length);
			lenInv = 1.f / static_cast<float>(length);
			for(auto& v: vals)
				v.envFol.prepare(PPDFPSMeters);
		}

#if PPDHasGainIn
		void processIn(const float** samples, int numChannels, int numSamples) noexcept
		{
			wHead(numSamples);

			process(vals[Type::In], samples, numChannels, numSamples);
		}
#endif

		void processOut(const float** samples, int numChannels, int numSamples) noexcept
		{
#if !PPDHasGainIn
			wHead(numSamples);
#endif
			process(vals[Type::Out], samples, numChannels, numSamples);
		}

		const std::atomic<float>& operator()(int i) const noexcept
		{
			return vals[i].env;
		}

	protected:
		std::array<Val, NumTypes> vals;
		WHead wHead;
		float lenInv;
		int length;

	private:
		void process(Val& val, const float** samples, int numChannels, int numSamples) noexcept
		{
			auto& rect = val.rect;
			auto& vVal = val.val;
			auto& envFol = val.envFol;

#if PPDMetersUseRMS
			if (numChannels == 1)
			{
				auto smpls = samples[0];


				for (auto s = 0; s < numSamples; ++s)
				{
					const auto w = wHead[s];

					if (w == 0)
					{
						vVal = std::sqrt(rect * lenInv);
						val.env.store(envFol.process(
							vVal,
							RiseInMs,
							FallInMs
						));

						rect = 0.f;
					}

					const auto smpl = smpls[s];
					rect += smpl * smpl;
				}
			}
			else
			{
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto w = wHead[s];

					if (w == 0)
					{
						vVal = std::sqrt(rect * lenInv) * .5f;
						val.env.store(envFol.process(
							vVal,
							RiseInMs,
							FallInMs
						));

						rect = 0.f;
					}

					const auto smpl = samples[0][s] + samples[1][s];
					rect += smpl * smpl;
				}
			}
#else
			if (numChannels == 1)
			{
				const auto smpls = samples[0];

				for (auto s = 0; s < numSamples; ++s)
				{
					const auto w = wHead[s];

					if (w == 0)
					{
						vVal = std::sqrt(rect);
						val.env.store(envFol.process(
							vVal,
							RiseInMs,
							FallInMs
						));

						rect = 0.f;
					}

					const auto smpl = smpls[s];
					rect = rect < smpl ? smpl : rect;
				}
			}
			else
			{
				for (auto s = 0; s < numSamples; ++s)
				{
					const auto w = wHead[s];

					if (w == 0)
					{
						vVal = rect * .5f;
						val.env.store(envFol.process(
							vVal,
							RiseInMs,
							FallInMs
						));

						rect = 0.f;
					}

					const auto smpl = samples[0][s] + samples[1][s];
					rect = rect < smpl ? smpl : rect;
				}
			}
#endif
		}
	};
}

#include "configEnd.h"