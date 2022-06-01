#pragma once
#include "ButtonParameterRandomizer.h"

#if PPDHasPatchBrowser
#include "PatchBrowser.h"
#endif

#include "Knob.h"
#include "Menu.h"
#include "MIDICCMonitor.h"
#include "LowLevel.h"

#include "../config.h"

namespace gui
{
	struct HighLevel :
		public Comp
	{
		HighLevel(Utils& u, LowLevel* _lowLevel) :
			Comp(u, "", CursorType::Default),
#if PPDHasPatchBrowser
			patchBrowser(u),
			patchBrowserButton(u, patchBrowser),
#endif
			macro(u, "Macro", PID::Macro, false),
			parameterRandomizer(u),
#if PPDHasGainIn
			gainIn(u, "In", PID::GainIn),
			meterIn(gainIn, u.getMeter(0)),
			gainOut(u, "Out", PID::Gain),
#else
			gainOut(u, "Gain", PID::Gain),
#endif
			meterOut(gainOut, u.getMeter(PPDHasGainIn ? 1 : 0)),
			mix(u, "Mix", PID::Mix),
#if PPDHasUnityGain
			unityGain(u, param::toTooltip(PID::UnityGain)),
#endif
#if PPDHasHQ
			hq(u, param::toTooltip(PID::HQ)),
#endif
#if PPDHasStereoConfig
			stereoConfig(u, param::toTooltip(PID::StereoConfig)),
#endif
			power(u, param::toTooltip(PID::Power)),
#if PPDHasPolarity
			polarity(u, param::toTooltip(PID::Polarity)),
#endif

			ccMonitor(u, u.getMIDILearn()),

			lowLevel(_lowLevel),

			menu(nullptr),
			menuButton(u, "Click here to open or close the panel with the advanced settings.")
		{
#if PPDHasPatchBrowser
			layout.init(
				{ 1, 8, 1, 8, 1, 8, 1, 8, 1, 1 },
				{ 1, 8, 1, 5, 1, 21, 1, 21, 1, 21, 1, 8, 5 }
			);
#else
			layout.init(
				{ 1, 8, 1, 8, 1, 8, 1, 8, 1, 1 },
				{ 1, 8, 1, 21, 1, 21, 1, 21, 1, 8, 5 }
			);
#endif
			
#if PPDHasPatchBrowser
			addAndMakeVisible(patchBrowserButton);
#endif
			addAndMakeVisible(macro);
			addAndMakeVisible(parameterRandomizer);
			parameterRandomizer.add(utils.getAllParams());
#if PPDHasGainIn
			addAndMakeVisible(gainIn);
#endif
			addAndMakeVisible(gainOut);
			addAndMakeVisible(mix);
#if PPDHasUnityGain
			makeParameterSwitchButton(unityGain, PID::UnityGain, ButtonSymbol::UnityGain);
			addAndMakeVisible(unityGain);
#endif
#if PPDHasHQ
			makeParameterSwitchButton(hq, PID::HQ, "HQ");
			hq.getLabel().mode = Label::Mode::TextToLabelBounds;
			addAndMakeVisible(hq);
#endif
#if PPDHasStereoConfig
			makeParameterSwitchButton(stereoConfig, PID::StereoConfig, ButtonSymbol::StereoConfig);
			stereoConfig.getLabel().mode = Label::Mode::TextToLabelBounds;
			addAndMakeVisible(stereoConfig);
#endif
			makeParameterSwitchButton(power, PID::Power, ButtonSymbol::Power);
			addAndMakeVisible(power);
#if PPDHasPolarity
			makeParameterSwitchButton(polarity, PID::Polarity, ButtonSymbol::Polarity);
			addAndMakeVisible(polarity);
#endif
			addAndMakeVisible(ccMonitor);

			makeSymbolButton(menuButton, ButtonSymbol::Settings);
			menuButton.toggleState = 0;
			menuButton.onClick.push_back([this]()
			{
				auto& btn = menuButton;

				auto& ts = btn.toggleState;
				ts = ts == 0 ? 1 : 0;
				repaintWithChildren(&btn);

				if (ts == 1)
				{
					auto& pluginTop = utils.pluginTop;

					const auto xml = loadXML(BinaryData::menu_xml, BinaryData::menu_xmlSize);
					if (xml == nullptr)
						return;
					const auto vt = ValueTree::fromXml(*xml);
					if (!vt.isValid())
						return;

					menu.reset(new Menu(utils, vt));
					pluginTop.addAndMakeVisible(*menu);

					const auto bounds1 = lowLevel->getBounds().toFloat();
					const auto bounds0 = bounds1.withLeft(static_cast<float>(pluginTop.getRight()));

					menu->defineBounds(bounds0, bounds1);
					menu->initWidget(.1f, false);
				}
				else
				{
					menu.reset(nullptr);
				}
			});
			addAndMakeVisible(menuButton);

			setInterceptsMouseClicks(false, true);
		}

		void init()
		{
#if PPDHasPatchBrowser
			auto& pluginTop = utils.pluginTop;
			pluginTop.addChildComponent(patchBrowser);
#endif
		}

		void paint(Graphics& g) override
		{
			//g.setColour(juce::Colours::white.withAlpha(.2f));
			//layout.paint(g);
			
			g.setFont(getFontDosisMedium());
			g.setColour(Colours::c(ColourID::Hover));
			
			//layout.label(g, "<", 7.f, 3.f, .5f, 1.f, false);
			//layout.label(g, ">", 7.5f, 3.f, .5f, 1.f, false);
			//layout.label(g, "v", 1.5f, 5.f, .5f, .25f, true);
			
			g.fillRect(layout.right());

#if PPDGainIn
			const auto thicc = utils.thicc;
			const auto thicc3 = thicc * 3.f;
			const Stroke stroke(thicc, Stroke::JointStyle::curved, Stroke::EndCapStyle::rounded);
			const auto gainArea = layout(1.f, 7.f, 7.f, 1.f);
			g.drawFittedText("Gain", gainArea.toNearestInt(), Just::centredTop, 1);
			drawRectEdges(g, gainArea, thicc3, stroke);
#endif
		}

		void resized() override
		{
			layout.resized();

#if PPDHasPatchBrowser
			layout.place(patchBrowserButton, 1.f, 3.f, 5.f, 1.f, false);
#endif
			const auto patchBrowserOffset = PPDHasPatchBrowser ? 2.f : 0.f;

			layout.place(menuButton, 1.f, 1.f, 1.f, 1.f, true);
			layout.place(parameterRandomizer, 7.f, 1.f, 1.f, 1.f, true);

			layout.place(macro, 3.f, 3.f + patchBrowserOffset, 3.f, 1.f, true);
			
#if PPDGainIn
			layout.place(gainIn, 1.f, 5.f + patchBrowserOffset, 2.5f, 2.f, true);
#if PPDUnityGain
			layout.place(unityGain, 3.6f, 5.2f + patchBrowserOffset, 1.8f, .6f, true);
#endif
			layout.place(gainOut, 5.5f, 5.f + patchBrowserOffset, 2.5f, 2.f, true);
#else
			layout.place(gainOut, 3.f, 5.2f + patchBrowserOffset, 3.f, 1.6f, true);
#endif
			
			layout.place(mix, 3.f, 7.f + patchBrowserOffset, 3.f, 1.f, true);

			layout.place(power, 1.f, 9.f + patchBrowserOffset, 1.f, 1.f, true);
#if PPDHasPolarity
			layout.place(polarity, 3.f, 9.f + patchBrowserOffset, 1.f, 1.f, true);
#endif
#if PPDHasStereoConfig
			layout.place(stereoConfig, 5.f, 9.f + patchBrowserOffset, 1.f, 1.f, true);
#endif
			layout.place(hq, 7.f, 9.f + patchBrowserOffset, 1.f, 1.f, true);

			layout.place(ccMonitor, 1.f, 10.f + patchBrowserOffset, 3.f, 1.f, false);

#if PPDHasPatchBrowser
			patchBrowser.setBounds(lowLevel->getBounds());
#endif

			if (menu != nullptr)
			{
				menu->defineBounds(menu->getBounds().toFloat(), lowLevel->getBounds().toFloat());
				menu->initWidget(.3f, false);
			}
		}

	protected:
#if PPDHasPatchBrowser
		PatchBrowser patchBrowser;
		ButtonPatchBrowser patchBrowserButton;
#endif
		Knob macro;
		ButtonParameterRandomizer parameterRandomizer;
#if PPDHasGainIn
		Knob gainIn;
		KnobMeter meterIn;
#endif
		Knob gainOut;
		KnobMeter meterOut;
		Knob mix;
#if PPDHasUnityGain
		Button unityGain;
#endif
#if PPDHasHQ
		Button hq;
#endif
#if PPDHasStereoConfig
		Button stereoConfig;
#endif
		Button power;
#if PPDHasPolarity
		Button polarity;
#endif

		MIDICCMonitor ccMonitor;

		LowLevel* lowLevel;
		std::unique_ptr<Menu> menu;
		Button menuButton;
	};
}


#include "../configEnd.h"