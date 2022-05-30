#include "Button.h"

gui::BlinkyBoy::BlinkyBoy() :
	comp(nullptr),
	env(0.f),
	inv(0.f)
{}

void gui::BlinkyBoy::init(Comp* _comp, float timeInSecs) noexcept
{
	const auto fps = 30.f;
	comp = _comp;
	env = 1.f;
	inv = 1.f / (timeInSecs * fps);
	startTimerHz(static_cast<int>(fps));
}

gui::Colour gui::BlinkyBoy::getInterpolated(Colour c0, Colour c1) const noexcept
{
	const auto e = env * env;
	return c0.interpolatedWith(c1, e < 0.f ? 0.f : e);
}

void gui::BlinkyBoy::timerCallback()
{
	env -= inv;
	if (env < 0.f)
		stopTimer();
	comp->repaint();
}

void gui::Button::enableLabel(const String& txt)
{
	label.setText(txt);
	label.textCID = ColourID::Interact;
	addAndMakeVisible(label);
}

void gui::Button::enableLabel(std::vector<String>&& txt)
{
	toggleTexts = txt;
	label.setText("");
	label.textCID = ColourID::Interact;
	addAndMakeVisible(label);
}

void gui::Button::enableParameterSwitch(PID _pID)
{
	pID = _pID;

	stopTimer();

	onClick.push_back([param = utils.getParam(pID)]()
	{
		const auto ts = param->getValue() > .5f ? 0.f : 1.f;
		param->setValueWithGesture(ts);
	});

	onTimer.push_back([this]()
		{
			bool shallRepaint = false;

			const auto param = utils.getParam(pID);

			const auto lckd = param->isLocked();
			if (locked != lckd)
			{
				locked = lckd;
				label.textCID = locked ? ColourID::Inactive : ColourID::Interact;
				shallRepaint = true;
			}

			const auto nTs = param->getValue() > .5f ? 1 : 0;
			if (toggleState != nTs)
			{
				toggleState = nTs;

				if (toggleTexts.size() > toggleState)
					label.setText(toggleTexts[toggleState]);

				shallRepaint = true;
			}

			if (shallRepaint)
				repaintWithChildren(this);
		});

	startTimerHz(24);
}

void gui::Button::enableParameter(PID _pID, int val)
{
	pID = _pID;

	stopTimer();

	onClick.push_back([param = utils.getParam(pID), v = static_cast<float>(val)]()
	{
		const auto pVal = std::rint(param->getValueDenorm());
		const auto ts = pVal == v ? 0.f : v;
		param->setValueWithGesture(param->range.convertTo0to1(ts));
	});

	onTimer.push_back([this, val]()
		{
			bool shallRepaint = false;

			const auto param = utils.getParam(pID);

			const auto lckd = param->isLocked();
			if (locked != lckd)
			{
				locked = lckd;
				label.textCID = locked ? ColourID::Inactive : ColourID::Interact;
				shallRepaint = true;
			}

			const auto pVal = std::rint(param->getValueDenorm());
			const auto nTs = static_cast<int>(pVal);
			if (toggleState != nTs)
			{
				toggleState = nTs;

				if (toggleTexts.size() > toggleState)
					label.setText(toggleTexts[toggleState]);

				shallRepaint = true;
			}

			if (shallRepaint)
				repaintWithChildren(this);
		});

	startTimerHz(24);
}

gui::Button::Button(Utils& _utils, String&& _tooltip) :
	Comp(_utils, _tooltip),
	onClick(),
	onRightClick(),
	onTimer(),
	onPaint(),
	blinkyBoy(),
	toggleState(-1),
	pID(PID::NumParams),
	locked(false),
	label(utils, ""),
	toggleTexts()
{
}

gui::Label& gui::Button::getLabel() noexcept { return label; }

void gui::Button::resized()
{
	if (label.isVisible())
	{
		const auto thicc = utils.thicc;
		const auto thicc4 = thicc * 2.f;
		const auto bounds = getLocalBounds().toFloat().reduced(thicc4);

		label.setBounds(bounds.toNearestInt());
	}
}

void gui::Button::paint(Graphics& g)
{
	for (auto& op : onPaint)
		op(g, *this);
}

void gui::Button::mouseEnter(const Mouse& mouse)
{
	Comp::mouseEnter(mouse);
	repaint();
}

void gui::Button::mouseExit(const Mouse&)
{
	repaint();
}

void gui::Button::mouseUp(const Mouse& mouse)
{
	if (mouse.mouseWasDraggedSinceMouseDown())
		return;

	if (mouse.mods.isLeftButtonDown())
	{
		if (locked)
			return;

		blinkyBoy.init(this, .25f);

		for (auto& oc : onClick)
			oc();
		notify(EvtType::ButtonClicked, this);
	}
	else
	{
		for (auto& oc : onRightClick)
			oc();
		notify(EvtType::ButtonRightClicked, this);
	}
}

void gui::Button::timerCallback()
{
	for (auto& ot : onTimer)
		ot();
}

gui::Button::OnPaint gui::buttonOnPaintDefault()
{
	return [](Graphics& g, Button& button)
	{
		auto col = button.blinkyBoy.getInterpolated(juce::Colours::darkgrey, juce::Colours::white);
		g.fillAll(col);
	};
}

void gui::makeTextButton(Button& b, const String& txt, bool withToggle, int targetToggleState)
{
	b.enableLabel(txt);

	b.onPaint.push_back([withToggle, targetToggleState](Graphics& g, Button& button)
		{
			const auto& utils = button.getUtils();
			const auto& blinkyBoy = button.blinkyBoy;

			auto thicc = utils.thicc;
			const auto thiccHalf = thicc * .5f;
			const bool isOver = button.isMouseOver();
			const bool isDown = button.isMouseButtonDown();
			thicc *= (isOver ? 1.1f : 1.f);

			const auto area = button.getLocalBounds().toFloat().reduced(thiccHalf);

			const auto col = blinkyBoy.getInterpolated(Colours::c(ColourID::Bg), juce::Colours::white);

			g.setColour(col);
			g.fillRoundedRectangle(area, thicc);

			g.setColour(Colours::c(ColourID::Hover));
			if (withToggle && button.toggleState == targetToggleState)
				g.fillRoundedRectangle(area, thicc);

			g.drawRoundedRectangle(area, thicc, thicc);

			if (button.isMouseOver())
			{
				g.fillRoundedRectangle(area, thicc);
				if (isDown)
					g.fillRoundedRectangle(area, thicc);
			}
		});
}

void gui::makeSymbolButton(Button& b, ButtonSymbol symbol, int targetToggleState)
{
	bool withToggle = true;
	if (symbol == ButtonSymbol::PatchMode)
		withToggle = false;
	else if (symbol == ButtonSymbol::StereoConfig)
	{
		withToggle = false;
		b.enableLabel({ "L/R", "M/S" });
	}

	b.onPaint.push_back([symbol, withToggle, targetToggleState](Graphics& g, Button& button)
	{
		const auto& utils = button.getUtils();
		const auto& blinkyBoy = button.blinkyBoy;

		auto thicc = utils.thicc;
		const bool isOver = button.isMouseOver();
		const bool isDown = button.isMouseButtonDown();
		thicc *= (isOver ? 1.1f : 1.f);

		auto bounds = button.getLocalBounds().toFloat().reduced(thicc);
		auto col = blinkyBoy.getInterpolated(Colours::c(ColourID::Bg), juce::Colours::white);

		g.setColour(col);
		g.fillRoundedRectangle(bounds, thicc);

		g.setColour(Colours::c(ColourID::Hover));
		if (withToggle && button.toggleState == targetToggleState)
			g.fillRoundedRectangle(bounds, thicc);
		g.drawRoundedRectangle(bounds, thicc, thicc);

		if (button.isMouseOver())
		{
			g.fillRoundedRectangle(bounds, thicc);
			if (isDown)
				g.fillRoundedRectangle(bounds, thicc);
		}

		bool abortable = symbol == ButtonSymbol::Settings;
		if (abortable && button.toggleState == 1 || symbol == ButtonSymbol::Abort)
			col = Colours::c(ColourID::Abort);
		else
			col = Colours::c(button.locked ? ColourID::Inactive : ColourID::Interact);
		g.setColour(col);

		if (symbol == ButtonSymbol::Polarity)
		{
			const auto thicc3 = thicc * 3.f;

			bounds = maxQuadIn(bounds).reduced(thicc3);
			g.drawEllipse(bounds, thicc);

			const LineF line(bounds.getBottomLeft(), bounds.getTopRight());
			g.drawLine(line, thicc);
		}
		else if (symbol == ButtonSymbol::UnityGain)
		{
			const auto thicc3 = thicc * 3.f;

			bounds = bounds.reduced(thicc3);

			const auto x0 = bounds.getX();
			const auto y0 = bounds.getY();

			const auto w = bounds.getWidth() * .666f;
			const auto h = bounds.getHeight() * .666f;

			const auto x1 = x0 + w * .5f;
			const auto y1 = y0 + h * .5f;

			g.drawEllipse({ x0, y0, w, h }, thicc);
			g.drawEllipse({ x1, y1, w, h }, thicc);
		}
		else if (symbol == ButtonSymbol::Power)
		{
			const auto thicc3 = thicc * 3.f;

			bounds = bounds.reduced(thicc3);

			const auto x = bounds.getX();
			const auto y = bounds.getY();
			const auto rad = bounds.getWidth() * .5f;

			const PointF centre(
				x + rad,
				y + rad
			);

			const auto pi = 3.14159265359f;

			const auto fromRads = pi * .2f;
			const auto toRads = 2.f * pi - fromRads;

			Path path;
			path.addCentredArc(
				centre.x,
				centre.y,
				rad,
				rad,
				0.f,
				fromRads,
				toRads,
				true
			);

			g.strokePath(path, juce::PathStrokeType(thicc));

			const LineF line(centre, centre.withY(y));

			g.drawLine(line, thicc);
		}
		else if (symbol == ButtonSymbol::PatchMode)
		{
			if (button.toggleState == 0)
			{
				const auto thicc3 = thicc * 3.f;
				bounds = maxQuadIn(bounds).reduced(thicc3);

				g.drawEllipse(bounds, thicc);
				const auto rad = bounds.getWidth() * .5f;
				PointF centre(
					bounds.getX() + rad,
					bounds.getY() + rad
				);
				const auto tick = LineF::fromStartAndAngle(centre, rad, PiQuart);
				g.drawLine(tick, thicc);
			}
			else
			{
				const auto thicc3 = thicc * 2.f;
				bounds = maxQuadIn(bounds).reduced(thicc3);

				const auto x0 = bounds.getX();
				const auto y0 = bounds.getY() + bounds.getHeight() * .5f;
				const auto x1 = x0 + bounds.getWidth() * .2f;
				const auto y1 = y0;
				g.drawLine(x0, y0, x1, y1, thicc);
				const auto x2 = x0 + bounds.getWidth() * .3f;
				const auto yA = bounds.getY() + bounds.getHeight() * .2f;
				const auto yB = bounds.getBottom() - bounds.getHeight() * .2f;
				g.drawLine(x1, y1, x2, yA, thicc);
				g.drawLine(x1, y1, x2, yB, thicc);
				const auto x3 = x0 + bounds.getWidth() * .7f;
				g.drawLine(x2, yA, x3, yA, thicc);
				g.drawLine(x2, yB, x3, yB, thicc);
				const auto x4 = x0 + bounds.getWidth() * .8f;
				const auto y4 = y0;
				g.drawLine(x3, yA, x4, y4, thicc);
				g.drawLine(x3, yB, x4, y4, thicc);
				const auto x5 = bounds.getRight();
				g.drawLine(x4, y4, x5, y4, thicc);
			}
		}
		else if (symbol == ButtonSymbol::Settings)
		{
			if (button.toggleState == 1)
			{
				const auto thicc3 = thicc * 3.f;
				bounds = maxQuadIn(bounds).reduced(thicc3);

				g.setFont(getFontNEL());
				g.drawFittedText("X", bounds.toNearestInt(), Just::centred, 1, 0.f);
			}
			else
			{
				const auto thicc3 = thicc * 4.f;
				bounds = maxQuadIn(bounds).reduced(thicc3);

				const auto x = bounds.getX();
				const auto y = bounds.getY();
				const auto w = bounds.getWidth();
				const auto h = bounds.getHeight();
				const auto btm = y + h;
				const auto rght = x + w;

				Stroke stroke(
					thicc,
					Stroke::JointStyle::curved,
					Stroke::EndCapStyle::rounded
				);

				const auto tickWidth = .2f;
				const auto rad = w * tickWidth;
				const auto angle0 = 0.f - PiQuart;
				const auto angle1 = PiHalf + PiQuart;

				{
					const auto centreX = x;
					const auto centreY = btm;

					Path path;
					path.addCentredArc(
						centreX, centreY,
						rad, rad,
						0.f, angle0, angle1,
						true
					);

					g.strokePath(path, stroke);
				}

				{
					const auto centreX = rght;
					const auto centreY = y;
					Path path;
					path.addCentredArc(
						centreX, centreY,
						rad, rad,
						Pi, angle0, angle1,
						true
					);

					g.strokePath(path, stroke);
				}

				{
					const auto padding = rad;

					const auto x0 = x + padding;
					const auto y0 = btm - padding;
					const auto x1 = rght - padding;
					const auto y1 = y + padding;

					g.drawLine(x0, y0, x1, y1, thicc);
				}
			}
		}
		else if (symbol == ButtonSymbol::Abort)
		{
			const auto thicc3 = thicc * 3.f;
			bounds = maxQuadIn(bounds).reduced(thicc3);

			g.setFont(getFontNEL());
			g.drawFittedText("X", bounds.toNearestInt(), Just::centred, 1, 0.f);
		}
		else if (symbol == ButtonSymbol::Random)
		{
			const auto thicc3 = thicc * 2.f;
			bounds = maxQuadIn(bounds).reduced(thicc3);

			const auto minDimen = std::min(bounds.getWidth(), bounds.getHeight());
			const auto radius = minDimen * .5f;
			const auto pointSize = radius * .4f;
			const auto pointRadius = pointSize * .5f;
			const auto d4 = minDimen / 4.f;
			const auto x0 = d4 * 1.2f + bounds.getX();
			const auto x1 = d4 * 2.8f + bounds.getX();
			for (auto i = 1; i < 4; ++i)
			{
				const auto y = d4 * i + bounds.getY();
				g.fillEllipse(x0 - pointRadius, y - pointRadius, pointSize, pointSize);
				g.fillEllipse(x1 - pointRadius, y - pointRadius, pointSize, pointSize);
			}
		}
	});
}

void gui::makeToggleButton(Button& b, const String& txt)
{
	makeTextButton(b, txt, true);
	b.onClick.push_back([&button = b]()
	{
		button.toggleState = !button.toggleState;
	});
}

void gui::makeParameterSwitchButton(Button& b, PID pID, String&& txt)
{
	makeTextButton(b, std::move(txt), true);
	b.enableParameterSwitch(pID);
}

void gui::makeParameterSwitchButton(Button& b, PID pID, ButtonSymbol symbol)
{
	makeSymbolButton(b, symbol);
	b.enableParameterSwitch(pID);
}

template<size_t NumButtons>
void gui::makeParameterButtonsGroup(std::array<Button, NumButtons>& btns, PID pID, const char* txt, bool onlyText)
{
	for (auto i = 0; i < NumButtons; ++i)
	{
		auto& btn = btns[i];

		const auto ts = i + 1;

		makeTextButton(btn, String::charToString(txt[i]), true, onlyText, ts);
		btn.enableParameter(pID, ts);
	}

}

void gui::makeButtonsGroup(std::vector<std::unique_ptr<Button>>& btns, int defaultToggleStateIndex)
{
	for (auto& btn : btns)
		btn->toggleState = 0;
	btns[defaultToggleStateIndex]->toggleState = 1;

	for (auto i = 0; i < btns.size(); ++i)
	{
		auto& btn = *btns[i];

		btn.onClick.push_back([&buttons = btns, i]()
			{
				for (auto& btn : buttons)
				{
					btn->toggleState = 0;
					repaintWithChildren(btn.get());
				}

				buttons[i]->toggleState = 1;
			});
	}
}

void gui::makeURLButton(Button& b, String&& urlPath)
{
	const juce::URL url(urlPath);

	b.onClick.push_back([url]()
	{
		url.launchInDefaultBrowser();
	});
}

/*

toggleState == 1
	has glow

*/
