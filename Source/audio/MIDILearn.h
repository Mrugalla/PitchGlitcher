#pragma once
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>

#include "../param/Param.h"
#include "../arch/State.h"

namespace audio
{
	class MIDILearn
	{
		using MIDIBuf = juce::MidiBuffer;
		using Params = param::Params;
		using State = sta::State;
		using Param = param::Param;
		using PID = param::PID;
		using String = juce::String;

		static constexpr float ValInv = 1.f / 128.f;

		struct CC
		{
			CC();

			void setValue(int);

			std::atomic<param::Param*> param;
		};

	public:
		MIDILearn(Params&, State&);

		void savePatch() const;
		
		void loadPatch();

		void operator()(const MIDIBuf&) noexcept;

		void assignParam(param::Param*) noexcept;
		
		void removeParam(param::Param*) noexcept;

		std::array<CC, 120> ccBuf;
		std::atomic<int> ccIdx;
	protected:
		std::atomic<param::Param*> assignableParam;
		Params& params;
		State& state;

		String getIDString(int) const;
	};
}