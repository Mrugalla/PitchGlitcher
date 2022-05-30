#pragma once
#include "Label.h"

namespace gui
{

	struct BlinkyBoy :
		public Timer
	{
		BlinkyBoy();

		void init(Comp* _comp, float timeInSecs) noexcept;

		Colour getInterpolated(Colour c0, Colour c1) const noexcept;

	protected:
		Comp* comp;
		float env, inv;

		void timerCallback() override;
	};

	struct Button :
		public Comp,
		public Timer
	{
		using OnClick = std::function<void()>;
		using OnPaint = std::function<void(Graphics&, Button&)>;

		void enableLabel(const String&);

		void enableLabel(std::vector<String>&&);

		void enableParameterSwitch(PID);

		void enableParameter(PID, int /*val*/);

		Button(Utils&, String&& /*tooltip*/ = "");

		Label& getLabel() noexcept;

		std::vector<OnClick> onClick, onRightClick, onTimer;
		std::vector<OnPaint> onPaint;
		BlinkyBoy blinkyBoy;
		int toggleState;
		PID pID;
		bool locked;
	protected:
		Label label;
		std::vector<String> toggleTexts;

		void resized() override;

		void paint(Graphics&) override;

		void mouseEnter(const Mouse&) override;

		void mouseExit(const Mouse&) override;

		void mouseUp(const Mouse&) override;

		void timerCallback() override;
	};

	Button::OnPaint buttonOnPaintDefault();

	void makeTextButton(Button&, const String& /*txt*/, bool /*withToggle*/ = false, int /*targetToggleState*/ = 1);

	enum class ButtonSymbol
	{
		Empty,
		Polarity,
		StereoConfig,
		UnityGain,
		Power,
		PatchMode,
		Settings,
		Random,
		Abort,
		NumSymbols
	};

	void makeSymbolButton(Button&, ButtonSymbol, int /*targetToggleState*/ = 1);

	void makeToggleButton(Button&, const String&);

	void makeParameterSwitchButton(Button&, PID, String&& /*text*/);

	void makeParameterSwitchButton(Button&, PID, ButtonSymbol);

	template<size_t NumButtons>
	void makeParameterButtonsGroup(std::array<Button, NumButtons>&, PID, const char* /*txt*/, bool /*onlyText*/);

	void makeButtonsGroup(std::vector<std::unique_ptr<Button>>&, int /*defaultToggleStateIndex*/ = 0);

	void makeURLButton(Button&, String&& /*urlPath*/);
}

/*

toggleState == 1
	has glow

*/