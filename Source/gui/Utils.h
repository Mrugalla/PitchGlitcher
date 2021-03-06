#pragma once
#include "Using.h"
#include "Shared.h"
#include "Events.h"
#include "../audio/MIDILearn.h"

#include "../config.h"

namespace gui
{
	using Notify = evt::Notify;
	using Evt = evt::System::Evt;
	using EvtType = evt::Type;
	using EventSystem = evt::System;

	enum class CursorType
	{
		Default,
		Interact,
		Inactive,
		Mod,
		Bias,
		NumTypes
	};

	juce::MouseCursor makeCursor(CursorType);

	void hideCursor();
	void showCursor(const Component&, const Point* = nullptr);

	class Utils
	{
		static constexpr float DragSpeed = .5f;
	public:
		Utils(Component& /*pluginTop*/, Processor&);

		Param* getParam(PID pID) noexcept;
		const Param* getParam(PID pID) const noexcept;
		
		std::vector<Param*>& getAllParams() noexcept;
		const std::vector<Param*>& getAllParams() const noexcept;

		juce::ValueTree getState() const noexcept;
#if PPDHasMIDILearn
		void assignMIDILearn(PID pID) noexcept;
		void removeMIDILearn(PID pID) noexcept;
		const audio::MIDILearn& getMIDILearn() const noexcept;
#endif
		float getDragSpeed() const noexcept;

		float fontHeight() const noexcept;

		EventSystem& getEventSystem();
	
		const std::atomic<float>& getMeter(int i) const noexcept;

		Point getScreenPosition() const noexcept;

		void resized();

		Component& pluginTop;
		float thicc;
	protected:
		Processor& audioProcessor;
		Params& params;
		EventSystem eventSystem;
		Evt evt;
	};

	void appendRandomString(String&, Random&, int/*length*/,
		const String& /*legalChars*/ = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
}