#include "PluginEditor.h"
#include <cmath>

namespace
{
    const juce::Colour colMetalLight   (0xff5b5f63);
    const juce::Colour colMetalMid     (0xff474b4f);
    const juce::Colour colMetalDark    (0xff34373a);
    const juce::Colour colBottomTop    (0xff232629);
    const juce::Colour colBottomBot    (0xff16181a);
    const juce::Colour colTextBright   (0xffe6e8ea);
    const juce::Colour colTextMuted    (0xff9fa4a9);
    const juce::Colour colAccentRed    (0xffc94a38);
    const juce::Colour colVuFaceTop    (0xffd7dde2);
    const juce::Colour colVuFaceBot    (0xffb8c0c7);
}

OptoLookAndFeel::OptoLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, colTextMuted);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
}

void OptoLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                                        float sliderPosProportional, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float pos = juce::jlimit (0.0f, 1.0f, sliderPosProportional);

    if (slider.getProperties().contains ("isSmallKnob"))
    {
        drawSmallKnob (g, bounds, pos, rotaryStartAngle, rotaryEndAngle);
        return;
    }

    const bool isVu = slider.getProperties().contains ("isVuSelector");
    drawMainKnob (g, bounds, pos, rotaryStartAngle, rotaryEndAngle, isVu);
}

void OptoLookAndFeel::drawMainKnob (juce::Graphics& g, juce::Rectangle<float> bounds, float pos,
                                    float startAngle, float endAngle, bool isVuSelector)
{
    auto knobArea = bounds.reduced (6.0f);
    const float diameter = juce::jmin (knobArea.getWidth(), knobArea.getHeight());
    const float radius = diameter * 0.5f;
    const auto centre = knobArea.getCentre();
    auto circle = juce::Rectangle<float> (centre.x - radius, centre.y - radius, diameter, diameter);

    // Мягкая тень под ручкой
    g.setColour (juce::Colours::black.withAlpha (0.55f));
    g.fillEllipse (circle.translated (0.0f, 3.0f));

    // Внешний металлический обод
    juce::ColourGradient bezelGrad (colMetalLight, centre.x, circle.getY(),
                                    colMetalDark, centre.x, circle.getBottom(), false);
    g.setGradientFill (bezelGrad);
    g.fillEllipse (circle);

    g.setColour (juce::Colours::black.withAlpha (0.65f));
    g.drawEllipse (circle, 1.5f);

    // Тело ручки (тёмный анодированный металл)
    auto cap = circle.reduced (radius * 0.14f);
    juce::ColourGradient capGrad (juce::Colour (0xff2b2e31), cap.getX(), cap.getY(),
                                  juce::Colour (0xff121416), cap.getX(), cap.getBottom(), false);
    g.setGradientFill (capGrad);
    g.fillEllipse (cap);

    // Тонкий блик по верхнему краю шапки
    g.setColour (juce::Colours::white.withAlpha (0.14f));
    g.drawEllipse (cap.reduced (1.0f), 1.0f);

    // Указатель положения
    const float angle = startAngle + pos * (endAngle - startAngle);
    juce::Path pointer;
    const float ptrLen = isVuSelector ? cap.getHeight() * 0.34f : cap.getHeight() * 0.38f;
    const float ptrThick = isVuSelector ? 3.2f : 2.6f;
    pointer.addRoundedRectangle (-ptrThick * 0.5f, -cap.getHeight() * 0.44f,
                                  ptrThick, ptrLen, 1.2f);

    g.setColour (isVuSelector ? colTextBright : juce::Colour (0xfff2d27a));
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre));
}

void OptoLookAndFeel::drawSmallKnob (juce::Graphics& g, juce::Rectangle<float> bounds, float pos,
                                     float startAngle, float endAngle)
{
    auto area = bounds.reduced (4.0f);
    const float d = juce::jmin (area.getWidth(), area.getHeight());
    const float r = d * 0.5f;
    const auto c = area.getCentre();
    auto circle = juce::Rectangle<float> (c.x - r, c.y - r, d, d);

    g.setColour (juce::Colours::black.withAlpha (0.5f));
    g.fillEllipse (circle.translated (0.0f, 2.0f));

    juce::ColourGradient grad (juce::Colour (0xff34383c), c.x, circle.getY(),
                               juce::Colour (0xff141618), c.x, circle.getBottom(), false);
    g.setGradientFill (grad);
    g.fillEllipse (circle);

    g.setColour (juce::Colours::black.withAlpha (0.7f));
    g.drawEllipse (circle, 1.2f);

    const float angle = startAngle + pos * (endAngle - startAngle);
    juce::Path line;
    line.addRoundedRectangle (-1.2f, -r * 0.82f, 2.4f, r * 0.42f, 1.0f);
    g.setColour (colTextBright);
    g.fillPath (line, juce::AffineTransform::rotation (angle).translated (c));
}

void OptoLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& button,
                                       bool, bool)
{
    auto area = button.getLocalBounds().toFloat();
    const bool isLimit = button.getToggleState();

    // Подписи COMPRESS сверху и LIMIT снизу, как на референсе
    g.setFont (juce::FontOptions (11.5f, juce::Font::bold));
    g.setColour (!isLimit ? colTextBright : colTextMuted);
    g.drawText ("COMPRESS", area.removeFromTop (20.0f), juce::Justification::centred);

    g.setColour (isLimit ? colTextBright : colTextMuted);
    g.drawText ("LIMIT", area.removeFromBottom (20.0f), juce::Justification::centred);

    // Металлический тумблер по центру
    const float toggleW = 24.0f;
    const float toggleH = 44.0f;
    auto slot = juce::Rectangle<float> (area.getCentreX() - toggleW * 0.5f,
                                       area.getCentreY() - toggleH * 0.5f,
                                       toggleW, toggleH);

    // Гайка / посадочное кольцо
    auto collar = slot.reduced (3.0f);
    g.setColour (juce::Colour (0xff1c1e20));
    g.fillRoundedRectangle (collar, 5.0f);
    g.setColour (juce::Colours::white.withAlpha (0.12f));
    g.drawRoundedRectangle (collar, 5.0f, 1.0f);

    // Рычаг тумблера (вверх = COMPRESS, вниз = LIMIT)
    const float leverY = isLimit ? slot.getBottom() - 16.0f : slot.getY() + 2.0f;
    auto lever = juce::Rectangle<float> (slot.getCentreX() - 5.5f, leverY, 11.0f, 14.0f);

    juce::ColourGradient leverGrad (juce::Colour (0xffd0d5da), lever.getX(), lever.getY(),
                                    juce::Colour (0xff5c6166), lever.getX(), lever.getBottom(), false);
    g.setGradientFill (leverGrad);
    g.fillRoundedRectangle (lever, 3.0f);
    g.setColour (juce::Colours::black.withAlpha (0.6f));
    g.drawRoundedRectangle (lever, 3.0f, 1.0f);
}

void OptoLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                          const juce::Colour&, bool isHighlighted, bool)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);
    const bool active = button.getToggleState();

    juce::Colour top = active ? juce::Colour (0xff4a4f54) : juce::Colour (0xff2b2f33);
    juce::Colour bot = active ? juce::Colour (0xff32363a) : juce::Colour (0xff1b1e21);

    if (isHighlighted && !active)
        top = top.brighter (0.08f);

    juce::ColourGradient grad (top, bounds.getX(), bounds.getY(),
                               bot, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRoundedRectangle (bounds, 3.0f);

    g.setColour (active ? colTextBright.withAlpha (0.45f) : juce::Colours::black.withAlpha (0.75f));
    g.drawRoundedRectangle (bounds, 3.0f, 1.0f);
}

void OptoLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button, bool, bool)
{
    const bool active = button.getToggleState();
    g.setColour (active ? colTextBright : colTextMuted);
    g.setFont (juce::FontOptions (11.0f, active ? juce::Font::bold : juce::Font::plain));
    g.drawText (button.getButtonText(), button.getLocalBounds().toFloat(),
                juce::Justification::centred);
}

// ==============================================================================

OptoCompEditor::OptoCompEditor (OptoCompProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&laf);

    const float pi = juce::MathConstants<float>::pi;

    configureBigKnob (gainKnob, "gain", "GAIN");
    configureBigKnob (peakKnob, "peak", "PEAK REDUCTION");

    configureSmallKnob (hiFreqKnob, "tube", "HI FREQ");
    configureSmallKnob (mixKnob,  "mix",  "MIX");
    configureSmallKnob (trimKnob, "gain", "TRIM");
    // Привязываем TRIM к "gain" только если отдельного параметра "trim" ещё нет в APVTS;
    // если есть "trim", замени "gain" выше на "trim".

    modeSwitch.setClickingTogglesState (true);
    addAndMakeVisible (modeSwitch);
    modeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (
        proc.apvts, "mode", modeSwitch);

    for (auto* btn : { &btn50Hz, &btn60Hz, &btnOff })
    {
        btn->setRadioGroupId (1001);
        btn->setClickingTogglesState (true);
        addAndMakeVisible (*btn);
    }
    btnOff.setToggleState (true, juce::dontSendNotification);

    btn50Hz.onClick = [this] { analogMode = 0; repaint (bottomPanelBounds.toNearestInt()); };
    btn60Hz.onClick = [this] { analogMode = 1; repaint (bottomPanelBounds.toNearestInt()); };
    btnOff.onClick  = [this] { analogMode = 2; repaint (bottomPanelBounds.toNearestInt()); };

    vuDisplayKnob.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    vuDisplayKnob.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    vuDisplayKnob.setRange (0.0, 2.0, 1.0);
    vuDisplayKnob.setValue (1.0, juce::dontSendNotification);
    vuDisplayKnob.setRotaryParameters (pi * 1.25f, pi * 2.75f, true);
    vuDisplayKnob.getProperties().set ("isVuSelector", true);
    vuDisplayKnob.onValueChange = [this]
    {
        setVuMode (static_cast<VuMode> (juce::roundToInt (vuDisplayKnob.getValue())));
    };
    addAndMakeVisible (vuDisplayKnob);

    setSize (920, 390);
    startTimerHz (60);
}

OptoCompEditor::~OptoCompEditor()
{
    setLookAndFeel (nullptr);
}

void OptoCompEditor::configureBigKnob (RotaryControl& ctrl, const juce::String& paramId,
                                       const juce::String& labelText)
{
    const float pi = juce::MathConstants<float>::pi;
    ctrl.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    ctrl.slider.setTextBoxStyle (juce::Slider::NoTextBox, true, 0, 0);
    ctrl.slider.setRotaryParameters (pi * 1.22f, pi * 2.78f, true);
    addAndMakeVisible (ctrl.slider);

    ctrl.titleLabel.setText (labelText, juce::dontSendNotification);
    ctrl.titleLabel.setJustificationType (juce::Justification::centred);
    ctrl.titleLabel.setColour (juce::Label::textColourId, colTextBright);
    ctrl.titleLabel.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    addAndMakeVisible (ctrl.titleLabel);

    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, paramId, ctrl.slider);
}

void OptoCompEditor::configureSmallKnob (RotaryControl& ctrl, const juce::String& paramId,
                                         const juce::String& labelText)
{
    const float pi = juce::MathConstants<float>::pi;
    ctrl.slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    ctrl.slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 56, 15);
    ctrl.slider.setRotaryParameters (pi * 1.25f, pi * 2.75f, true);
    ctrl.slider.getProperties().set ("isSmallKnob", true);
    addAndMakeVisible (ctrl.slider);

    ctrl.titleLabel.setText (labelText, juce::dontSendNotification);
    ctrl.titleLabel.setJustificationType (juce::Justification::centred);
    ctrl.titleLabel.setColour (juce::Label::textColourId, colTextMuted);
    ctrl.titleLabel.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    addAndMakeVisible (ctrl.titleLabel);

    ctrl.attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, paramId, ctrl.slider);
}

void OptoCompEditor::setVuMode (VuMode newMode)
{
    vuMode = newMode;
    repaint (topPanelBounds.toNearestInt());
}

void OptoCompEditor::drawBrushedPanel (juce::Graphics& g, juce::Rectangle<float> bounds,
                                     juce::Colour topCol, juce::Colour botCol, int seed)
{
    juce::ColourGradient grad (topCol, bounds.getX(), bounds.getY(),
                               botCol, bounds.getX(), bounds.getBottom(), false);
    g.setGradientFill (grad);
    g.fillRect (bounds);

    // Тонкий горизонтальный шлиф по металлу
    juce::Random rng (seed);
    for (int i = 0; i < 140; ++i)
    {
        const float y = bounds.getY() + rng.nextFloat() * bounds.getHeight();
        const float x = bounds.getX() + rng.nextFloat() * bounds.getWidth();
        const float len = 25.0f + rng.nextFloat() * 140.0f;
        const bool light = rng.nextBool();
        g.setColour ((light ? juce::Colours::white : juce::Colours::black)
                         .withAlpha (0.025f + rng.nextFloat() * 0.03f));
        g.drawHorizontalLine (juce::roundToInt (y), x, juce::jmin (bounds.getRight(), x + len));
    }
}

void OptoCompEditor::drawLogoAndTitle (juce::Graphics& g, juce::Rectangle<float> area)
{
    // Иконка волновой формы в стиле референса
    auto iconBox = area.removeFromTop (44.0f).withSizeKeepingCentre (68.0f, 38.0f);
    juce::Path wave;
    const float w = iconBox.getWidth();
    const float h = iconBox.getHeight();
    const float cy = iconBox.getCentreY();
    const float x0 = iconBox.getX();

    wave.startNewSubPath (x0, cy);
    wave.lineTo (x0 + w * 0.22f, cy);
    wave.lineTo (x0 + w * 0.30f, cy - h * 0.36f);
    wave.lineTo (x0 + w * 0.37f, cy + h * 0.22f);
    wave.lineTo (x0 + w * 0.44f, cy - h * 0.48f);
    wave.lineTo (x0 + w * 0.52f, cy + h * 0.45f);
    wave.lineTo (x0 + w * 0.60f, cy - h * 0.24f);
    wave.lineTo (x0 + w * 0.68f, cy + h * 0.14f);
    wave.lineTo (x0 + w * 0.76f, cy);
    wave.lineTo (x0 + w, cy);

    g.setColour (colTextBright);
    g.strokePath (wave, juce::PathStrokeType (2.6f, juce::PathStrokeType::mitered,
                                              juce::PathStrokeType::rounded));

    area.removeFromTop (6.0f);
    g.setColour (colTextBright);
    g.setFont (juce::FontOptions (21.0f, juce::Font::bold));
    g.drawText ("OPTOCOMP", area.removeFromTop (28.0f), juce::Justification::centred);
}

void OptoCompEditor::drawScaleTicks (juce::Graphics& g, juce::Point<float> centre,
                                     float radius, float startAngle, float endAngle,
                                     bool withNumbers)
{
    const int majorCount = 10; // 0, 10, 20 ... 100
    const int subPerMajor = 4;

    for (int i = 0; i <= majorCount * subPerMajor; ++i)
    {
        const float t = (float) i / (float) (majorCount * subPerMajor);
        const float angle = startAngle + t * (endAngle - startAngle);
        const bool isMajor = (i % subPerMajor == 0);

        const float outerR = radius + 12.0f;
        const float innerR = outerR - (isMajor ? 8.0f : 4.5f);

        const float sinA = std::sin (angle);
        const float cosA = -std::cos (angle);

        juce::Point<float> p1 (centre.x + innerR * sinA, centre.y + innerR * cosA);
        juce::Point<float> p2 (centre.x + outerR * sinA, centre.y + outerR * cosA);

        g.setColour (isMajor ? colTextBright.withAlpha (0.85f) : colTextMuted.withAlpha (0.55f));
        g.drawLine ({ p1, p2 }, isMajor ? 1.4f : 0.9f);

        if (isMajor && withNumbers)
        {
            const int val = (i / subPerMajor) * 10;
            const float numR = outerR + 11.0f;
            juce::Point<float> np (centre.x + numR * sinA, centre.y + numR * cosA);

            g.setColour (colTextMuted);
            g.setFont (juce::FontOptions (10.5f, juce::Font::plain));
            g.drawText (juce::String (val),
                        juce::Rectangle<float> (np.x - 16.0f, np.y - 8.0f, 32.0f, 16.0f),
                        juce::Justification::centred);
        }
    }
}

void OptoCompEditor::drawVuMeter (juce::Graphics& g, juce::Rectangle<float> bounds)
{
    // Внешняя тёмная рамка прибора
    g.setColour (juce::Colour (0xff121416));
    g.fillRoundedRectangle (bounds, 6.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawRoundedRectangle (bounds, 6.0f, 1.0f);

    auto face = bounds.reduced (10.0f);
    juce::ColourGradient faceGrad (colVuFaceTop, face.getX(), face.getY(),
                                   colVuFaceBot, face.getX(), face.getBottom(), false);
    g.setGradientFill (faceGrad);
    g.fillRoundedRectangle (face, 4.0f);

    // Дуга шкалы внутри VU-метра
    const float pi = juce::MathConstants<float>::pi;
    const float arcStart = -pi * 0.24f;
    const float arcEnd   =  pi * 0.24f;
    const float pivotY   = face.getBottom() + face.getHeight() * 0.72f;
    const juce::Point<float> pivot (face.getCentreX(), pivotY);
    const float arcRadius = face.getHeight() * 1.18f;

    juce::Path scaleArc;
    scaleArc.addCentredArc (pivot.x, pivot.y, arcRadius, arcRadius,
                           0.0f, arcStart, arcEnd, true);
    g.setColour (juce::Colour (0xff2b3036));
    g.strokePath (scaleArc, juce::PathStrokeType (1.4f));

    // Деления VU (-20 .. 0 .. +3 dB)
    struct Tick { float normPos; const char* label; bool red; };
    const Tick ticks[] = {
        { -1.00f, "-20", false },
        { -0.76f, "-10", false },
        { -0.56f, "-7",  false },
        { -0.38f, "-5",  false },
        { -0.18f, "-3",  false },
        { -0.04f, "-2",  false },
        {  0.10f, "-1",  false },
        {  0.25f, "0",   false },
        {  0.46f, "1",   true  },
        {  0.68f, "2",   true  },
        {  0.92f, "3",   true  }
    };

    for (const auto& t : ticks)
    {
        const float ang = t.normPos * arcEnd;
        const float sinA = std::sin (ang);
        const float cosA = -std::cos (ang);

        const float rIn  = arcRadius - 6.0f;
        const float rOut = arcRadius + 4.0f;
        const float rTxt = arcRadius + 13.0f;

        g.setColour (t.red ? colAccentRed : juce::Colour (0xff24282d));
        g.drawLine (pivot.x + rIn * sinA, pivot.y + rIn * cosA,
                    pivot.x + rOut * sinA, pivot.y + rOut * cosA, 1.3f);

        if (t.label != nullptr)
        {
            g.setFont (juce::FontOptions (10.5f, juce::Font::bold));
            g.drawText (t.label,
                        juce::Rectangle<float> (pivot.x + rTxt * sinA - 14.0f,
                                                pivot.y + rTxt * cosA - 7.0f, 28.0f, 14.0f),
                        juce::Justification::centred);
        }
    }

    // Стрелка прибора
    const float needleAng = juce::jlimit (-1.0f, 1.0f, displayedNeedlePos) * arcEnd;
    const float nSin = std::sin (needleAng);
    const float nCos = -std::cos (needleAng);
    const float needleLen = arcRadius + 6.0f;

    g.setColour (colAccentRed);
    g.drawLine (pivot.x + 12.0f * nSin, pivot.y + 12.0f * nCos,
                pivot.x + needleLen * nSin, pivot.y + needleLen * nCos, 1.8f);

    // Внутренний винт / тень под стеклом
    g.setColour (juce::Colours::white.withAlpha (0.14f));
    g.drawRoundedRectangle (face.reduced (0.5f), 4.0f, 1.0f);
}

void OptoCompEditor::paint (juce::Graphics& g)
{
    drawBrushedPanel (g, topPanelBounds, colMetalLight, colMetalMid, 101);
    drawBrushedPanel (g, bottomPanelBounds, colBottomTop, colBottomBot, 202);

    // Разделительная линия между верхней и нижней панелями
    g.setColour (juce::Colours::black.withAlpha (0.85f));
    g.drawHorizontalLine (juce::roundToInt (bottomPanelBounds.getY()), 0.0f, (float) getWidth());
    g.setColour (juce::Colours::white.withAlpha (0.08f));
    g.drawHorizontalLine (juce::roundToInt (bottomPanelBounds.getY()) + 1, 0.0f, (float) getWidth());

    // Левый блок: логотип и название
    auto leftArea = topPanelBounds.removeFromLeft (topPanelBounds.getWidth() * 0.17f);
    drawLogoAndTitle (g, leftArea.reduced (8.0f, 22.0f));

    // Шкалы вокруг больших ручек GAIN и PEAK REDUCTION
    const float pi = juce::MathConstants<float>::pi;
    const float startA = pi * 1.22f;
    const float endA   = pi * 2.78f;

    auto gainR = gainKnob.slider.getBounds().toFloat().reduced (6.0f);
    const float gainRadius = juce::jmin (gainR.getWidth(), gainR.getHeight()) * 0.5f;
    drawScaleTicks (g, gainR.getCentre(), gainRadius, startA, endA, true);

    auto peakR = peakKnob.slider.getBounds().toFloat().reduced (6.0f);
    const float peakRadius = juce::jmin (peakR.getWidth(), peakR.getHeight()) * 0.5f;
    drawScaleTicks (g, peakR.getCentre(), peakRadius, startA, endA, true);

    // VU-метр
    drawVuMeter (g, vuMeterBounds);

    // Правый блок: VU DISPLAY (INPUT / GR / OUTPUT)
    auto vuArea = vuDisplayKnob.getBounds().toFloat();
    const float vuRadius = juce::jmin (vuArea.getWidth(), vuArea.getHeight()) * 0.36f;
    drawScaleTicks (g, vuArea.getCentre(), vuRadius, pi * 1.25f, pi * 2.75f, false);

    g.setColour (colTextBright);
    g.setFont (juce::FontOptions (11.5f, juce::Font::bold));
    g.drawText ("VU DISPLAY",
                juce::Rectangle<float> (vuArea.getX() - 20.0f, vuArea.getY() - 24.0f,
                                        vuArea.getWidth() + 40.0f, 18.0f),
                juce::Justification::centred);

    g.setColour (vuMode == VuMode::Input ? colTextBright : colTextMuted);
    g.setFont (juce::FontOptions (11.0f, vuMode == VuMode::Input ? juce::Font::bold : juce::Font::plain));
    g.drawText ("INPUT", vuArea.getX() - 46.0f, vuArea.getCentreY() - 10.0f, 42.0f, 20.0f,
                juce::Justification::centredRight);

    g.setColour (vuMode == VuMode::GainReduction ? colAccentRed : colTextMuted);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("GR", vuArea.getCentreX() - 20.0f, vuArea.getY() - 6.0f, 40.0f, 18.0f,
                juce::Justification::centred);

    g.setColour (vuMode == VuMode::Output ? colTextBright : colTextMuted);
    g.setFont (juce::FontOptions (11.0f, vuMode == VuMode::Output ? juce::Font::bold : juce::Font::plain));
    g.drawText ("OUTPUT", vuArea.getRight() + 4.0f, vuArea.getCentreY() - 10.0f, 54.0f, 20.0f,
                juce::Justification::centredLeft);

    // Надпись ANALOG слева внизу
    g.setColour (colTextBright);
    g.setFont (juce::FontOptions (12.0f, juce::Font::bold));
    g.drawText ("ANALOG",
                juce::Rectangle<float> (bottomPanelBounds.getX() + 18.0f,
                                        bottomPanelBounds.getCentreY() - 10.0f,
                                        76.0f, 20.0f),
                juce::Justification::centredLeft);
}

void OptoCompEditor::resized()
{
    auto bounds = getLocalBounds().toFloat();
    const float bottomH = 74.0f;

    bottomPanelBounds = bounds.removeFromBottom (bottomH);
    topPanelBounds    = bounds;

    // --- Верхняя панель ---
    auto top = topPanelBounds;
    top.removeFromLeft (top.getWidth() * 0.17f); // место под логотип и OPTOCOMP

    auto switchCol = top.removeFromLeft (92.0f);
    modeSwitch.setBounds (switchCol.reduced (4.0f, 46.0f).toNearestInt());

    auto rightSection = top.removeFromRight (165.0f);
    vuDisplayKnob.setBounds (rightSection.withSizeKeepingCentre (96.0f, 96.0f)
                                         .translated (0.0f, 12.0f).toNearestInt());

    // Между тумблером и VU-селектором: GAIN, VU METER, PEAK REDUCTION
    const float midW = top.getWidth();
    auto gainCol = top.removeFromLeft (midW * 0.29f);
    auto vuCol   = top.removeFromLeft (midW * 0.42f);
    auto peakCol = top;

    auto layoutBig = [] (RotaryControl& k, juce::Rectangle<float> col)
    {
        auto knobRect = col.reduced (26.0f, 34.0f).translated (0.0f, -8.0f);
        const float s = juce::jmin (knobRect.getWidth(), knobRect.getHeight());
        k.slider.setBounds (knobRect.withSizeKeepingCentre (s, s).toNearestInt());
        k.titleLabel.setBounds (juce::Rectangle<float> (col.getX(),
                                                       knobRect.getCentreY() + s * 0.5f + 22.0f,
                                                       col.getWidth(), 20.0f).toNearestInt());
    };

    layoutBig (gainKnob, gainCol);
    layoutBig (peakKnob, peakCol);

    vuMeterBounds = vuCol.reduced (14.0f, 52.0f);

    // --- Нижняя панель ---
    auto bot = bottomPanelBounds.reduced (14.0f, 14.0f);
    bot.removeFromLeft (84.0f); // место под ANALOG

    auto freqGroup = bot.removeFromLeft (186.0f).reduced (0.0f, 8.0f);
    const float btnW = freqGroup.getWidth() / 3.0f;
    btn50Hz.setBounds (freqGroup.removeFromLeft (btnW).reduced (2.0f, 0.0f).toNearestInt());
    btn60Hz.setBounds (freqGroup.removeFromLeft (btnW).reduced (2.0f, 0.0f).toNearestInt());
    btnOff.setBounds  (freqGroup.reduced (2.0f, 0.0f).toNearestInt());

    auto layoutSmall = [] (RotaryControl& k, juce::Rectangle<float> col)
    {
        auto titleArea = col.removeFromLeft (64.0f);
        k.titleLabel.setBounds (titleArea.toNearestInt());
        k.slider.setBounds (col.removeFromLeft (98.0f).toNearestInt());
    };

    bot.removeFromLeft (12.0f);
    layoutSmall (hiFreqKnob, bot.removeFromLeft (170.0f));
    bot.removeFromLeft (14.0f);
    layoutSmall (mixKnob, bot.removeFromLeft (170.0f));
    bot.removeFromLeft (14.0f);
    layoutSmall (trimKnob, bot.removeFromLeft (170.0f));
}

void OptoCompEditor::timerCallback()
{
    const float rawGr = juce::jmax (0.0f, proc.grMeterDb.load());
    smoothGrDb += 0.25f * (rawGr - smoothGrDb);

    float targetNorm = 0.0f;
    if (vuMode == VuMode::GainReduction)
    {
        // 0 dB -> правее центра (~0.25), больше GR -> стрелка уходит влево в минус
        const float grNorm = juce::jlimit (0.0f, 1.0f, smoothGrDb / 20.0f);
        targetNorm = 0.25f - grNorm * 1.25f;
    }
    else
    {
        // Базовый отклик для INPUT / OUTPUT, чтобы стрелка не стояла мёртво
        const float approxLevel = juce::jlimit (0.0f, 1.0f, smoothGrDb / 14.0f);
        targetNorm = -0.85f + approxLevel * 1.15f;
    }

    displayedNeedlePos += 0.22f * (targetNorm - displayedNeedlePos);
    repaint (vuMeterBounds.getExpanded (4.0f).toNearestInt());
}
