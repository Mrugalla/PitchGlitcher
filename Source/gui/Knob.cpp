#include "Knob.h"

gui::Knob::Knob(Utils& u, String&& _name, PID _pID, bool _modulatable) :
    Parametr(u, _pID, _modulatable),
    Timer(),
    knobBounds(0.f, 0.f, 0.f, 0.f),
    label(u, std::move(_name)),
    dragY(0.f),
    valMeter(0.f),
    cID(ColourID::Interact)
{
    layout.init(
        { 40, 40, 40 },
        { 100, 40, 40 }
    );

    label.textCID = ColourID::Txt;
    label.just = Just::centred;
    label.mode = Label::Mode::TextToLabelBounds;

    setName(std::move(_name));
    addAndMakeVisible(label);
    startTimerHz(PPDFPSKnobs);
}

void gui::Knob::updateMeter(float v)
{
    valMeter = v;
    repaint();
}

void gui::Knob::setCID(ColourID c)
{
    cID = c;
    label.textCID = cID;
}

gui::Label& gui::Knob::getLabel() noexcept { return label; }

void gui::Knob::timerCallback()
{
    bool needsRepaint = false;

    const auto lckd = param.isLocked();
    if (locked != lckd)
        setLocked(lckd);

    const auto vn = param.getValue();
    const auto mmd = param.getMaxModDepth();
    const auto vm = param.getValMod();
    const auto mb = param.getModBias();

    if (valNorm != vn || maxModDepth != mmd || valMod != vm || modBias != mb)
    {
        valNorm = vn;
        maxModDepth = mmd;
        valMod = vm;
        modBias = mb;
        needsRepaint = true;
    }

    if (needsRepaint)
        repaint();
}

void gui::Knob::paint(juce::Graphics& g)
{
    const auto thicc = utils.thicc;
    const auto thicc2 = thicc * 2.f;
    const auto thicc3 = thicc * 3.f;
    Stroke strokeType(thicc, Stroke::JointStyle::curved, Stroke::EndCapStyle::rounded);
    const auto radius = knobBounds.getWidth() * .5f;
    const auto radiusBetween = radius - thicc;
    const auto radiusInner = radius - thicc2;
    PointF centre(
        radius + knobBounds.getX(),
        radius + knobBounds.getY()
    );

    const auto col = Colours::c(ColourID::Interact);

    if (valMeter != 0.f)
    {
        g.setColour(Colours::c(ColourID::Txt));
        Path meterArc;

        const auto meterAngle = AngleRange * valMeter - AngleWidth;

        meterArc.addCentredArc(
            centre.x, centre.y,
            radiusBetween, radiusBetween,
            0.f,
            -AngleWidth, meterAngle,
            true
        );

        strokeType.setStrokeThickness(thicc2);
        g.strokePath(meterArc, strokeType);
        strokeType.setStrokeThickness(thicc);
    }

    //draw outlines
    {
        g.setColour(col);
        Path outtaArc;

        outtaArc.addCentredArc(
            centre.x, centre.y,
            radius, radius,
            0.f,
            -AngleWidth, AngleWidth,
            true
        );
        outtaArc.addCentredArc(
            centre.x, centre.y,
            radiusInner, radiusInner,
            0.f,
            -AngleWidth, AngleWidth,
            true
        );

        g.strokePath(outtaArc, strokeType);
    }

    const auto valNormAngle = valNorm * AngleRange;
    const auto valAngle = -AngleWidth + valNormAngle;
    const auto radiusExt = radius + thicc;

    // draw modulation
    if (modulatable)
    {
        const auto valModAngle = valMod * AngleRange;
        const auto modAngle = -AngleWidth + valModAngle;
        const auto modTick = juce::Line<float>::fromStartAndAngle(centre, radiusExt, modAngle);

        g.setColour(Colours::c(ColourID::Bg));
        g.drawLine(modTick, thicc * 4.f);

        const auto maxModDepthAngle = juce::jlimit(-AngleWidth, AngleWidth, valNormAngle + maxModDepth * AngleRange - AngleWidth);
        const auto biasAngle = AngleRange * modBias - AngleWidth;

        g.setColour(Colours::c(ColourID::Bias));
        {
            Path biasPath;
            biasPath.addCentredArc(
                centre.x, centre.y,
                radiusInner, radiusInner,
                0.f,
                0.f, biasAngle,
                true
            );
            g.strokePath(biasPath, strokeType);
        }

        g.setColour(Colours::c(ColourID::Mod));
        g.drawLine(modTick.withShortenedStart(radiusInner), thicc2);
        {
            Path modPath;
            modPath.addCentredArc(
                centre.x, centre.y,
                radius, radius,
                0.f,
                maxModDepthAngle, valAngle,
                true
            );
            g.strokePath(modPath, strokeType);
        }
    }
    // draw tick
    {
        const auto tickLine = juce::Line<float>::fromStartAndAngle(centre, radiusExt, valAngle);
        g.setColour(Colours::c(ColourID::Bg));
        g.drawLine(tickLine, thicc3);
        g.setColour(col);
        g.drawLine(tickLine.withShortenedStart(radiusInner), thicc2);
    }
}

void gui::Knob::resized()
{
    const auto thicc = utils.thicc;

    layout.resized();

    knobBounds = layout(0, 0, 3, 2, true).reduced(thicc);
    layout.place(modDial, 1, 1, 1, 1, true);
    layout.place(label, 0, 2, 3, 1, false);
}

void gui::Knob::mouseEnter(const Mouse& mouse)
{
    Comp::mouseEnter(mouse);
    label.setText(param.getCurrentValueAsText());
    label.repaint();
}

void gui::Knob::mouseExit(const Mouse&)
{
    label.setText(getName());
    label.repaint();
}

void gui::Knob::mouseDown(const Mouse& mouse)
{
    if (mouse.mods.isLeftButtonDown())
    {
        hideCursor();

        if (param.isInGesture())
            return;
        param.beginGesture();
        dragY = mouse.position.y / utils.getDragSpeed();
        {
            label.setText(param.getCurrentValueAsText());
            label.repaint();
        }
    }
}

void gui::Knob::mouseDrag(const Mouse& mouse)
{
    if (mouse.mods.isLeftButtonDown())
    {
        const auto dragYNew = mouse.position.y / utils.getDragSpeed();
        auto dragOffset = dragYNew - dragY;
        if (mouse.mods.isShiftDown())
            dragOffset *= SensitiveDrag;
        const auto newValue = juce::jlimit(0.f, 1.f, param.getValue() - dragOffset);
        param.setValueNotifyingHost(newValue);
        dragY = dragYNew;
        {
            label.setText(param.getCurrentValueAsText());
            label.repaint();
        }
        notify(EvtType::ParametrDragged, this);
    }
}

void gui::Knob::mouseUp(const Mouse& mouse)
{
    if (mouse.mods.isLeftButtonDown())
    {
        if (!mouse.mouseWasDraggedSinceMouseDown())
        {
            if (mouse.mods.isCtrlDown())
                param.setValueNotifyingHost(param.getDefaultValue());
            else
            {
                juce::Point<float> centre(
                    static_cast<float>(getWidth()) * .5f,
                    static_cast<float>(getHeight()) * .5f
                );
                const juce::Line<float> fromCentre(centre, mouse.position);
                const auto angle = fromCentre.getAngle();

                const auto newValue = juce::jlimit(0.f, 1.f, (angle + AngleWidth) / AngleRange);
                param.setValue(newValue);
            }
        }
        param.endGesture();
    }
    else if (mouse.mods.isRightButtonDown())
        if (!mouse.mouseWasDraggedSinceMouseDown())
            if (mouse.mods.isCtrlDown())
                param.setValueWithGesture(param.getDefaultValue());
            else
                notify(EvtType::ParametrRightClicked, this);
    showCursor(*this);
    {
        label.setText(param.getCurrentValueAsText());
        label.repaint();
    }
}

void gui::Knob::mouseWheelMove(const Mouse& mouse, const juce::MouseWheelDetails& wheel)
{
    if (mouse.mods.isAnyMouseButtonDown())
        return;
    const bool reversed = wheel.isReversed ? -1.f : 1.f;
    const bool isTrackPad = wheel.deltaY * wheel.deltaY < .0549316f;
    if (isTrackPad)
        dragY = reversed * wheel.deltaY;
    else
    {
        const auto deltaYPos = wheel.deltaY > 0.f ? 1.f : -1.f;
        dragY = reversed * WheelDefaultSpeed * deltaYPos;
    }
    if (mouse.mods.isShiftDown())
        dragY *= SensitiveDrag;
    const auto newValue = juce::jlimit(0.f, 1.f, param.getValue() + dragY);
    param.setValueWithGesture(newValue);
    {
        label.setText(param.getCurrentValueAsText());
        label.repaint();
    }
}

gui::KnobMeter::KnobMeter(Knob& _knob, const Val& _val) :
    knob(_knob),
    val(_val),
    env(0.f)
{
    startTimerHz(static_cast<int>(PPDFPSMeters));
}

void gui::KnobMeter::timerCallback()
{
    auto e = val.load();
    e = std::floor(e * 128.f) * .0078125f;
    if (env == e)
        return;
    env = e;
    knob.updateMeter(env > 1.f ? 1.f : env);
}

#include "../configEnd.h"