#pragma once

#include "param/Param.h"

#include "Processor.h"

#include "gui/Shared.h"
#include "gui/Layout.h"
#include "gui/Utils.h"

#include "gui/Shader.h"

#include "gui/ContextMenu.h"
#include "gui/TextEditor.h"
#include "gui/Knob.h"
#include "gui/HighLevel.h"
#include "gui/LowLevel.h"

#include "gui/Tooltip.h"

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_core/juce_core.h>

#include <array>

#include "config.h"

namespace gui
{
    using Graphics = juce::Graphics;
    using Mouse = juce::MouseEvent;
    using MouseWheel = juce::MouseWheelDetails;

    struct Editor :
        public juce::AudioProcessorEditor
    {
        static constexpr int MinWidth = 100, MinHeight = 100;

        Editor(audio::Processor& p) :
            juce::AudioProcessorEditor(p),
            audioProcessor(p),
            
            layout(*this),
            utils(*this, p),

            pluginTitle(utils, JucePlugin_Name),

            lowLevel(utils),
            highLevel(utils, &lowLevel),

            tooltip(utils, "The tooltips bar leads you to wisdom."),

            contextMenuKnobs(utils),
            contextMenuButtons(utils),

            editorKnobs(utils),

            bypassed(false),
            shadr(utils, *this)
            
        {
            setComponentEffect(&shadr);

            setMouseCursor(makeCursor(CursorType::Default));
            
            layout.init(
                { 1, 3 },
                { 2, 13, 1 }
            );

            pluginTitle.font = getFontDosisExtraLight();
            addAndMakeVisible(pluginTitle);
            pluginTitle.mode = Label::Mode::TextToLabelBounds;

            addAndMakeVisible(lowLevel);
            addAndMakeVisible(highLevel);

            highLevel.init();

            addAndMakeVisible(tooltip);
            
            addAndMakeVisible(contextMenuKnobs);
            addAndMakeVisible(contextMenuButtons);

            addChildComponent(editorKnobs);
            
            setOpaque(true);
            setResizable(true, true);
            {
                auto user = audioProcessor.props.getUserSettings();
                const auto w = user->getIntValue("gui/width", PPDEditorWidth);
                const auto h = user->getIntValue("gui/height", PPDEditorHeight);
                setSize(w, h);
            }
        }
        
        ~Editor()
        {
            setComponentEffect(nullptr);
        }
        
        void paint(Graphics& g) override
        {
            g.fillAll(Colours::c(gui::ColourID::Bg));
        }
        
        void resized() override
        {
            if (getWidth() < MinWidth)
                return setSize(MinWidth, getHeight());
            if (getHeight() < MinHeight)
                return setSize(getWidth(), MinHeight);

            utils.resized();

            layout.resized();

            layout.place(pluginTitle, 1, 0, 1, 1, false);
            layout.place(lowLevel, 1, 1, 1, 1, false);
            layout.place(highLevel, 0, 0, 1, 2, false);

            tooltip.setBounds(layout.bottom().toNearestInt());

            const auto thicc = utils.thicc;
            editorKnobs.setBounds(0, 0, static_cast<int>(thicc * 42.f), static_cast<int>(thicc * 12.f));

            saveBounds();
        }

        void mouseEnter(const Mouse&) override
        {
            auto& evtSys = utils.getEventSystem();
            evtSys.notify(evt::Type::TooltipUpdated);
        }
        void mouseExit(const Mouse&) override
        {}
        void mouseDown(const Mouse&) override
        {}
        void mouseDrag(const Mouse&) override
        {}
        void mouseUp(const Mouse&) override
        {
            utils.getEventSystem().notify(EvtType::ClickedEmpty, this);
        }
        void mouseWheelMove(const Mouse&, const MouseWheel&) override
        {}

        

        audio::Processor& audioProcessor;
    
protected:
        Layout layout;
        Utils utils;

        Label pluginTitle;

        LowLevel lowLevel;
        HighLevel highLevel;

        Tooltip tooltip;

        ContextMenuKnobs contextMenuKnobs;
        ContextMenuButtons contextMenuButtons;

        TextEditorKnobs editorKnobs;

        bool bypassed;
        Shader shadr;
        

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Editor)
        //JUCE_LEAK_DETECTOR(Editor)
        //JUCE_HEAVYWEIGHT_LEAK_DETECTOR(Editor)
    private:
        
        void saveBounds()
        {
            const auto w = getWidth();
            const auto h = getHeight();
            auto user = audioProcessor.props.getUserSettings();
            user->setValue("gui/width", w);
            user->setValue("gui/height", h);
        }
    };
}

#include "configEnd.h"