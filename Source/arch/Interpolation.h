#pragma once

namespace interpolate
{
	enum class Type
	{
		Lerp,
		CubicHermiteSpline,
		NumTypes
	};

	static constexpr int NumTypes = static_cast<int>(Type::NumTypes);

	template<typename T>
	inline T lerp(const T* samples, T idx, int size) noexcept
	{
		const auto iFloor = std::floor(idx);
		const auto x = idx - iFloor;

		const auto iF = static_cast<int>(iFloor);
		const auto a = samples[iF];

		const auto iC = iF + 1;
		const auto b = iC != size ? samples[iC] : samples[0];
		
		return a + x * (b - a);
	}

	template<typename T>
	inline T cubicHermiteSpline(const T* buffer, T readHead, int size) noexcept
	{
		const auto iFloor = std::floor(readHead);
		auto i1 = static_cast<int>(iFloor);
		auto i0 = i1 - 1;
		auto i2 = i1 + 1;
		auto i3 = i1 + 2;
		if (i3 >= size) i3 -= size;
		if (i2 >= size) i2 -= size;
		if (i0 < 0) i0 += size;

		const auto t = readHead - iFloor;
		const auto v0 = buffer[i0];
		const auto v1 = buffer[i1];
		const auto v2 = buffer[i2];
		const auto v3 = buffer[i3];

		const auto c0 = v1;
		const auto c1 = static_cast<T>(.5) * (v2 - v0);
		const auto c2 = v0 - static_cast<T>(2.5) * v1 + static_cast<T>(2.) * v2 - static_cast<T>(.5) * v3;
		const auto c3 = static_cast<T>(1.5) * (v1 - v2) + static_cast<T>(.5) * (v3 - v0);

		return ((c3 * t + c2) * t + c1) * t + c0;
	}

	template<typename T>
	inline T cubicHermiteSpline(const T* buffer, T readHead) noexcept
	{
		const auto iFloor = std::floor(readHead);
		auto i1 = static_cast<int>(iFloor);
		auto i0 = i1 + 1;
		auto i2 = i1 + 2;
		auto i3 = i1 + 3;

		const auto t = readHead - iFloor;
		const auto v0 = buffer[i0];
		const auto v1 = buffer[i1];
		const auto v2 = buffer[i2];
		const auto v3 = buffer[i3];

		const auto c0 = v1;
		const auto c1 = static_cast<T>(.5) * (v2 - v0);
		const auto c2 = v0 - static_cast<T>(2.5) * v1 + static_cast<T>(2.) * v2 - static_cast<T>(.5) * v3;
		const auto c3 = static_cast<T>(1.5) * (v1 - v2) + static_cast<T>(.5) * (v3 - v0);

		return ((c3 * t + c2) * t + c1) * t + c0;
	}
}