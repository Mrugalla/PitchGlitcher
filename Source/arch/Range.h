#pragma once

#include "juce_core/juce_core.h"

namespace range
{
	struct Range
	{
		Range(float _start = 0.f, float _end = 1.f) :
			start(_start),
			end(_end),
			len(end - start)
		{}

		float lerp(float x) const noexcept
		{
			return start + x * len;
		}

		float start, end, len;
	};
}

namespace makeRange
{
	using Range = juce::NormalisableRange<float>;

	inline Range biased(float start, float end, float bias/*[-1, 1]*/) noexcept
	{
		// https://www.desmos.com/calculator/ps8q8gftcr
		const auto a = bias * .5f + .5f;
		const auto a2 = 2.f * a;
		const auto aM = 1.f - a;
		
		const auto r = end - start;
		const auto aR = r * a;
		if (bias != 0.f)
			return
		{
				start, end,
				[a2, aM, aR](float min, float max, float x)
				{
					if (x < 0.f)
						return min;
					if (x > 1.f)
						return max;
					const auto denom = aM - x + a2 * x;
					if (denom == 0.f)
						return min;
					return min + aR * x / denom;
				},
				[a2, aM, aR](float min, float max, float x)
				{
					if (x < min)
						return 0.f;
					if (x > max)
						return 1.f;
					const auto denom = a2 * min + aR - a2 * x - min + x;
					if (denom == 0.f)
						return 0.f;
					return aM * (x - min) / denom;
				},
				nullptr
		};
		else return { start, end };
	}

	inline Range toggle() noexcept
	{
		return { 0.f, 1.f, 1.f };
	}

	inline Range stepped(float start, float end, float steps = 1.f) noexcept
	{
		return
		{
				start, end,
				[range = end - start](float min, float, float normalized)
				{
					return min + normalized * range;
				},
				[rangeInv = 1.f / (end - start)](float min, float, float denormalized)
				{
					return (denormalized - min) * rangeInv;
				},
				[steps, stepsInv = 1.f / steps](float, float, float val)
				{
					return std::rint(val * stepsInv) * steps;
				}
		};
	}

	// advanced one(s):

	inline Range withCentre(float start, float end, float centre, float tolerance = .0001f) noexcept
	{
		auto b = 0.f;
		auto bInc = 1.f;
		auto range = biased(start, end, b);
		auto nVal = range.convertFrom0to1(.5f);
		auto dist = nVal - centre;
		auto dif = std::abs(dist);
		if (dif < tolerance)
			return range;
		do
		{
			bInc *= .5f;
			b += dist > 0.f ? -bInc : bInc;
			
			range = biased(start, end, b);
			nVal = range.convertFrom0to1(.5f);
			dist = nVal - centre;
			dif = std::abs(dist);

		} while (dif > tolerance);

		return range;
	}
}