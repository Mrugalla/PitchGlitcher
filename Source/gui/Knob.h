#pragma once
#include "GUIParams.h"

#include "../config.h"

namespace gui
{
    struct Knob :
        public Parametr,
        public Timer
    {
        static constexpr float AngleWidth = PiQuart * 3.f;
        static constexpr float AngleRange = AngleWidth * 2.f;

        Knob(Utils&, String&& /*name*/, PID, bool /*modulatable*/ = true);

        void updateMeter(float);

        void setCID(ColourID);

        Label& getLabel() noexcept;

    protected:
        BoundsF knobBounds;
        Label label;
        float dragY, valMeter;
        ColourID cID;

        void timerCallback() override;

        void paint(juce::Graphics&) override;

        void resized() override;

        void mouseEnter(const Mouse&) override;

        void mouseExit(const Mouse&) override;

        void mouseDown(const Mouse&) override;

        void mouseDrag(const Mouse&) override;

        void mouseUp(const Mouse&) override;

        void mouseWheelMove(const Mouse&, const juce::MouseWheelDetails&) override;

    };

    class KnobMeter :
        public Timer
    {
        using Val = std::atomic<float>;
    public:
        KnobMeter(Knob&, const Val&);

    protected:
        Knob& knob;
        const Val& val;
        float env;

        void timerCallback() override;
    };
}