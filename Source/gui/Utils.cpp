#include "Utils.h"

namespace gui
{

	Utils::Utils(Component& _pluginTop, Processor& _audioProcessor) :
		pluginTop(_pluginTop),
		audioProcessor(_audioProcessor),
		params(audioProcessor.params),
		eventSystem(),
		evt(eventSystem),
		thicc(1.f)
	{
		Colours::c.init(audioProcessor.props.getUserSettings());
	}

	Param* Utils::getParam(PID pID) noexcept { return params[pID]; }
	const Param* Utils::getParam(PID pID) const noexcept { return params[pID]; }

	std::vector<Param*>& Utils::getAllParams() noexcept
	{
		return params.data();
	}
	const std::vector<Param*>& Utils::getAllParams() const noexcept
	{
		return params.data();
	}

	juce::ValueTree Utils::getState() const noexcept
	{
		return audioProcessor.state.getState();
	}

	void Utils::assignMIDILearn(PID pID) noexcept
	{
		audioProcessor.midiLearn.assignParam(params[pID]);
	}
	void Utils::removeMIDILearn(PID pID) noexcept
	{
		audioProcessor.midiLearn.removeParam(params[pID]);
	}
	const audio::MIDILearn& Utils::getMIDILearn() const noexcept
	{
		return audioProcessor.midiLearn;
	}

	void Utils::resized()
	{
		auto a = std::min(pluginTop.getWidth(), pluginTop.getHeight());
		auto t = static_cast<float>(a) * .005f;
		thicc = t < 1.f ? 1.f : t;
	}

	float Utils::getDragSpeed() const noexcept
	{
		const auto height = static_cast<float>(pluginTop.getHeight());
		const auto speed = DragSpeed * height;
		return speed;
	}

	float Utils::fontHeight() const noexcept
	{
		const auto w = static_cast<float>(pluginTop.getWidth());
		const auto h = static_cast<float>(pluginTop.getHeight());

		const auto avr = (w + h) * .5f;
		const auto norm = (avr - 500.f) / 500.f;
		return std::floor(8.5f + norm * 5.f);
	}

	EventSystem& Utils::getEventSystem()
	{
		return eventSystem;
	}

	const std::atomic<float>& Utils::getMeter(int i) const noexcept
	{
		return audioProcessor.meters(i);
	}

	Point Utils::getScreenPosition() const noexcept { return pluginTop.getScreenPosition(); }

	juce::MouseCursor makeCursor(CursorType c)
	{
		Image img = juce::ImageCache::getFromMemory(BinaryData::cursor_png, BinaryData::cursor_pngSize).createCopy();

		const auto w = img.getWidth();
		const auto h = img.getHeight();

		const Colour imgCol(0xff37946e);

		Colour col;

		if (c == CursorType::Default)
			col = Colours::c(ColourID::Txt);
		else if (c == CursorType::Interact)
			col = Colours::c(ColourID::Interact);
		else if (c == CursorType::Inactive)
			col = Colours::c(ColourID::Inactive);
		else if (c == CursorType::Mod)
			col = Colours::c(ColourID::Mod);
		else if (c == CursorType::Bias)
			col = Colours::c(ColourID::Bias);

		for (auto y = 0; y < h; ++y)
			for (auto x = 0; x < w; ++x)
				if (img.getPixelAt(x, y) == imgCol)
					img.setPixelAt(x, y, col);

		static constexpr int scale = 3;
		img = img.rescaled(w * scale, h * scale, Graphics::ResamplingQuality::lowResamplingQuality);

		return { img, 0, 0 };
	}

	void hideCursor()
	{
		juce::Desktop::getInstance().getMainMouseSource().enableUnboundedMouseMovement(true, false);
	}

	void showCursor(const Component& comp, const Point* pos)
	{
		auto mms = juce::Desktop::getInstance().getMainMouseSource();
		const Point centre(comp.getWidth() / 2, comp.getHeight() / 2);
		if (pos == nullptr)
			pos = &centre;
		mms.setScreenPosition((comp.getScreenPosition() + *pos).toFloat());
		mms.enableUnboundedMouseMovement(false, true);
	}

	void appendRandomString(String& str, Random& rand, int length, const String& legalChars)
	{
		const auto max = static_cast<float>(legalChars.length() - 1);

		for (auto i = 0; i < length; ++i)
		{
			auto idx = static_cast<int>(rand.nextFloat() * max);
			str += legalChars[idx];
		}
	}
}