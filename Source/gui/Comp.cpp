#include "Comp.h"

gui::Comp::Comp(Utils& _utils, const String& _tooltip, CursorType _cursorType) :
	utils(_utils),
	layout(*this),
	evts(),
	tooltip(_tooltip),
	cursorType(_cursorType)
{
	evts.reserve(1);
	evts.emplace_back(utils.getEventSystem(), makeNotifyBasic(this));

	setMouseCursor(makeCursor(cursorType));
}

gui::Comp::Comp(Utils& _utils, const String& _tooltip, Notify&& _notify, CursorType _cursorType) :
	utils(_utils),
	layout(*this),
	evts(),
	tooltip(_tooltip),
	cursorType(_cursorType)
{
	evts.reserve(2);
	evts.emplace_back(utils.getEventSystem(), makeNotifyBasic(this));
	evts.emplace_back(utils.getEventSystem(), _notify);

	setMouseCursor(makeCursor(cursorType));
}

const gui::Utils& gui::Comp::getUtils() const noexcept { return utils; }
gui::Utils& gui::Comp::getUtils() noexcept { return utils; }

const gui::String* gui::Comp::getTooltip() const noexcept { return &tooltip; }
gui::String* gui::Comp::getTooltip() noexcept { return &tooltip; }

void gui::Comp::setTooltip(String&& t)
{
	tooltip = t;
	setInterceptsMouseClicks(true, true);
}

void gui::Comp::setCursorType(CursorType ct)
{
	if (cursorType != ct)
	{
		cursorType = ct;
		updateCursor();
	}
}

void gui::Comp::updateCursor()
{
	setMouseCursor(makeCursor(cursorType));
}

const gui::Layout& gui::Comp::getLayout() const noexcept { return layout; };

void gui::Comp::initLayout(const std::vector<int>& xL, const std::vector<int>& yL)
{
	layout.init(xL, yL);
}

void gui::Comp::initLayout(const String& xL, const String& yL)
{
	layout.fromStrings(xL, yL);
}

void gui::Comp::notify(EvtType type, const void* stuff)
{
	evts[0](type, stuff);
}

void gui::Comp::paint(Graphics& g)
{
	g.setColour(juce::Colour(0xffff0000));
	g.drawRect(getLocalBounds().toFloat(), 1.f);
}

void gui::Comp::mouseEnter(const Mouse&)
{
	notify(EvtType::TooltipUpdated, &tooltip);
}

void gui::Comp::mouseUp(const Mouse&)
{
	notify(EvtType::ClickedEmpty, this);
}

gui::Notify gui::Comp::makeNotifyBasic(Comp* c)
{
	return [c](const EvtType type, const void*)
	{
		if (type == EvtType::ColourSchemeChanged)
		{
			c->updateCursor();
			c->repaint();
		}
	};
}

gui::CompWidgetable::CompWidgetable(Utils& u, String&& _tooltip, CursorType _cursorType) :
	Comp(u, std::move(_tooltip), _cursorType),
	bounds0(),
	bounds1(),
	widgetEnvelope(0.f),
	widgetInc(1.f)
{
}

gui::CompWidgetable::CompWidgetable(Utils& u, String&& _tooltip, Notify&& _notify, CursorType _cursorType) :
	Comp(u, std::move(_tooltip), std::move(_notify), _cursorType),
	bounds0(),
	bounds1(),
	widgetEnvelope(0.f),
	widgetInc(1.f)
{
}

void gui::CompWidgetable::defineBounds(const BoundsF& b0, const BoundsF& b1)
{
	bounds0 = b0;
	bounds1 = b1;
}

void gui::CompWidgetable::initWidget(float lengthInSecs, bool _widgetEnv)
{
	widgetEnvelope = _widgetEnv ? 1.f : 0.f;
	widgetInc = 1.f / (30.f * lengthInSecs) * (_widgetEnv ? -1.f : 1.f);
	startTimerHz(30);
}

void gui::CompWidgetable::updateBounds()
{
	const auto x = static_cast<int>(bounds0.getX() + widgetEnvelope * (bounds1.getX() - bounds0.getX()));
	const auto y = static_cast<int>(bounds0.getY() + widgetEnvelope * (bounds1.getY() - bounds0.getY()));
	const auto w = static_cast<int>(bounds0.getWidth() + widgetEnvelope * (bounds1.getWidth() - bounds0.getWidth()));
	const auto h = static_cast<int>(bounds0.getHeight() + widgetEnvelope * (bounds1.getHeight() - bounds0.getHeight()));

	setBounds(x, y, w, h);
}

void gui::CompWidgetable::timerCallback()
{
	widgetEnvelope += widgetInc;
	if (widgetEnvelope < 0.f || widgetEnvelope > 1.f)
	{
		stopTimer();
		widgetEnvelope = std::rint(widgetEnvelope);
	}

	updateBounds();
}