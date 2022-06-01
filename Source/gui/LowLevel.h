#pragma once
#include "Knob.h"
#include "Shader.h"
#include "../config.h"

namespace gui
{
    struct LowLevel :
        public Comp
    {
        LowLevel(Utils& u) :
            Comp(u, "", CursorType::Default),
            grainSize(u, "Grain Size", PID::GrainSize),
            tune(u, "Tune", PID::TuneSemi),
            fine(u, "Fine", PID::TuneFine),
            feedback(u, "Feedback", PID::Feedback),
            numVoices(u, "Voices", PID::NumVoices),
            spreadTune(u, "Spread Tune", PID::SpreadTune)
        {
            
            layout.init(
                { 1, 2, 2, 2, 2, 2, 2, 1 },
                { 1, 5, 1 }
            );

            addAndMakeVisible(grainSize);
            addAndMakeVisible(tune);
            addAndMakeVisible(fine);
            addAndMakeVisible(feedback);
            addAndMakeVisible(numVoices);
            addAndMakeVisible(spreadTune);
        }

    protected:
        Knob grainSize, tune, fine, feedback, numVoices, spreadTune;
        
        void paint(Graphics&) override {}

        void resized() override
        {
            layout.resized();

            layout.place(grainSize, 1, 1, 1, 1, false);
            layout.place(tune, 2, 1, 1, 1, false);
            layout.place(fine, 3, 1, 1, 1, false);
            layout.place(feedback, 4, 1, 1, 1, false);
            layout.place(numVoices, 5, 1, 1, 1, false);
            layout.place(spreadTune, 6, 1, 1, 1, false);
        }
    };
}

#include "../configEnd.h"