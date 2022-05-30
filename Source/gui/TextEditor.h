#pragma once
#include "Button.h"

#include <functional>

#include "../config.h"

namespace gui
{
	struct TextEditor :
		public Comp,
		public Timer
	{
		TextEditor(Utils& u, const String& _tooltip, Notify&& _notify, const String& _emptyString = "enter value..") :
			Comp(u, _tooltip, std::move(_notify)),
			Timer(),
			onEscape([]() {}),
			onReturn([]() {}),
			onType([](){}),
			onRemove([](){}),

			label(u, ""),
			emptyString(_emptyString), txt(""),
			blinkyBoy(),
			tickIdx(0),
			drawTick(false)
		{
			addAndMakeVisible(label);
			label.mode = Label::Mode::TextToLabelBounds;
			setWantsKeyboardFocus(true);
		}

		TextEditor(Utils& u, const String& _tooltip, const String& _emptyString) :
			Comp(u, _tooltip),
			Timer(),
			onEscape([]() {}),
			onReturn([]() {}),
			onType([]() {}),
			onRemove([]() {}),

			label(u, ""),
			emptyString(_emptyString), txt(""),
			blinkyBoy(),
			tickIdx(0),
			drawTick(false)
		{
			addAndMakeVisible(label);
			label.mode = Label::Mode::TextToLabelBounds;
			setWantsKeyboardFocus(true);
		}

		void enable()
		{
			if (isEnabled())
				return;
			setVisible(true);
			tickIdx = label.getText().length();
			drawTick = true;
			grabKeyboardFocus();
			startTimerHz(PPDFPSTextEditor);
		}

		bool isEnabled() const noexcept
		{
			return isTimerRunning() && hasKeyboardFocus(false);
		}

		void disable()
		{
			stopTimer();
			drawTick = false;
			updateLabel();
		}

		const String& getText() const noexcept
		{
			return txt;
		}

		void setText(const String& str)
		{
			if (txt == str)
				return;
			txt = str;
			updateLabel();
		}

		bool isEmpty() const noexcept
		{
			return getText().isEmpty();
		}

		bool isNotEmpty() const noexcept
		{
			return getText().isNotEmpty();
		}

		void clear()
		{
			txt.clear();
			repaintWithChildren(this);
		}

		std::function<void()> onEscape, onReturn, onType, onRemove;
	protected:
		Label label;
		String emptyString, txt;
		BlinkyBoy blinkyBoy;
		int tickIdx;
		bool drawTick;

		void mouseUp(const Mouse& mouse) override
		{
			if (txt.isEmpty())
				return;

			if (label.mode == Label::Mode::TextToLabelBounds)
			{
				const auto x = mouse.position.x;
				const auto w = static_cast<float>(getWidth());
				const auto& font = label.font;
				const auto strWidth = font.getStringWidthFloat(txt);
				const auto xOff = (w - strWidth) * .5f;
				const auto xShifted = x - xOff;
				const auto strLen = static_cast<float>(txt.length());
				auto xRatio = xShifted / strWidth;
				auto xMapped = xRatio * strLen;
				auto xLimited = juce::jlimit(0.f, strLen, xMapped);
				tickIdx = static_cast<int>(std::rint(xLimited));
			}
			//else
			//	return; // not implemented yet cause not needed lol
			
			enable();
			drawTick = true;
			updateLabel();
		}

		void paint(Graphics& g) override
		{
			const auto thicc = utils.thicc;
			auto col = blinkyBoy.getInterpolated(Colours::c(ColourID::Hover), Colours::c(ColourID::Interact));
			g.setColour(col);
			g.drawRoundedRectangle(getLocalBounds().toFloat(), thicc, thicc);
		}

		void resized() override
		{
			label.setBounds(getLocalBounds());
		}

		void updateLabel()
		{
			if (txt.isEmpty())
			{
				label.setText(emptyString + (drawTick ? "|" : ""));
				label.textCID = ColourID::Hover;
			}
			else
			{
				label.textCID = ColourID::Txt;
				if (drawTick)
					label.setText(txt.substring(0, tickIdx) + "|" + txt.substring(tickIdx));
				else
					label.setText(txt);
			}
			label.repaint();
		}

		void timerCallback() override
		{
			if (!hasKeyboardFocus(true))
				return disable();
			drawTick = !drawTick;
			updateLabel();
		}

		bool keyPressed(const juce::KeyPress& key) override
		{
			if (key == key.escapeKey)
			{
				onEscape();
				return true;
			}
			if (key == key.returnKey)
			{
				onReturn();
				blinkyBoy.init(this, .25f);
				return true;
			}
			if (key == key.leftKey)
			{
				if (tickIdx > 0)
					--tickIdx;
				drawTick = true;
				updateLabel();
				return true;
			}
			if (key == key.rightKey)
			{
				if (tickIdx < txt.length())
					++tickIdx;
				drawTick = true;
				updateLabel();
				return true;
			}
			if (key == key.backspaceKey)
			{
				onRemove();
				txt = txt.substring(0, tickIdx - 1) + txt.substring(tickIdx);
				if (tickIdx > 0)
					--tickIdx;
				drawTick = true;
				updateLabel();
				return true;
			}
			if (key == key.deleteKey)
			{
				txt = txt.substring(0, tickIdx) + txt.substring(tickIdx + 1);
				drawTick = true;
				updateLabel();
				return true;
			}
			const auto chr = key.getTextCharacter();
			txt = txt.substring(0, tickIdx) + chr + txt.substring(tickIdx);
			++tickIdx;
			drawTick = true;
			updateLabel();
			onType();
			return true;
		}
	};

	struct TextEditorKnobs :
		public TextEditor
	{
		Notify makeNotify(TextEditorKnobs& tek)
		{
			return [&editor = tek](EvtType type, const void* stuff)
			{
				if (type == EvtType::ClickedEmpty)
				{
					editor.disable();
					editor.setVisible(false);
				}
				if (type == EvtType::EnterParametrValue)
				{
					editor.disable();
					editor.txt.clear();

					const auto parametr = static_cast<const Parametr*>(stuff);
					const auto pID = parametr->getPID();
					auto param = editor.getUtils().getParam(pID);

					editor.onEscape = [&tek = editor]()
					{
						tek.disable();
					};

					editor.onReturn = [&tek = editor, param]()
					{
						if (tek.txt.isNotEmpty())
						{
							const auto val = juce::jlimit(0.f, 1.f, param->getValueForText(tek.txt));
							param->setValueWithGesture(val);
						}
						tek.disable();
					};

					const auto mouse = juce::Desktop::getInstance().getMainMouseSource();
					const auto& utils = editor.getUtils();
					const auto screenPos = utils.getScreenPosition();
					const auto parametrScreenPos = parametr->getScreenPosition();
					const auto parametrPos = parametrScreenPos - screenPos;
					const Point parametrCentre(
						parametr->getWidth() / 2,
						parametr->getHeight() / 2
					);
					editor.setCentrePosition(parametrPos + parametrCentre);
					editor.enable();
				}
			};
		}

		TextEditorKnobs(Utils& u) :
			TextEditor(u, "Enter a value for this parameter.", makeNotify(*this))
		{

		}

		void paint(Graphics& g) override
		{
			const auto thicc = utils.thicc;
			g.setColour(Colours::c(ColourID::Darken));
			g.fillRoundedRectangle(getLocalBounds().toFloat(), thicc);

			TextEditor::paint(g);
		}
	};
}

#include "../configEnd.h"