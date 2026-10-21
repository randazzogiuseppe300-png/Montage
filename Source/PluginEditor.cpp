#include "PluginEditor.h"

#include <algorithm>
#include <cmath>

namespace
{
    // "CUT" -> "C U T" (per i titoli con le lettere spaziate)
    juce::String spaced (const juce::String& s)
    {
        juce::String r;
        for (auto c : s)
            r += juce::String::charToString (c) + " ";
        return r.trim();
    }

    juce::Font monoFont (float size)
    {
        return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(), size, juce::Font::plain));
    }
}

//==============================================================================
// LOOK AND FEEL
MontageLookAndFeel::MontageLookAndFeel()
{
    setColour (juce::TextButton::buttonColourId, juce::Colour (0xff1b1b25));
    setColour (juce::TextButton::textColourOffId, montage::text);
    setColour (juce::TextButton::textColourOnId, montage::text);

    setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff14141c));
    setColour (juce::ComboBox::outlineColourId, juce::Colour (0x22ffffff));
    setColour (juce::ComboBox::textColourId, montage::text);
    setColour (juce::ComboBox::arrowColourId, montage::orange);

    setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xff101017));
    setColour (juce::PopupMenu::textColourId, montage::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, montage::orange.darker (0.3f));
    setColour (juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
}

void MontageLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                           float sliderPos, float startAngle, float endAngle,
                                           juce::Slider& slider)
{
    const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (4.0f);
    const float radius = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto centre = bounds.getCentre();

    const float trackW = 4.0f;
    const float arcR = radius - trackW * 0.5f - 2.0f;
    const float bodyR = arcR - 8.0f;
    const float angle = startAngle + sliderPos * (endAngle - startAngle);
    const auto colour = slider.findColour (juce::Slider::rotarySliderFillColourId);

    // corpo della manopola
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2a2a35), centre.x, centre.y - bodyR,
                                             juce::Colour (0xff0a0a0f), centre.x, centre.y + bodyR, false));
    g.fillEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f);
    g.setColour (juce::Colour (0xff343440));
    g.drawEllipse (centre.x - bodyR, centre.y - bodyR, bodyR * 2.0f, bodyR * 2.0f, 1.0f);

    // traccia
    juce::Path track;
    track.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, endAngle, true);
    g.setColour (juce::Colour (0xff1d1d27));
    g.strokePath (track, juce::PathStrokeType (trackW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // valore, con alone luminoso
    if (sliderPos > 0.001f)
    {
        juce::Path val;
        val.addCentredArc (centre.x, centre.y, arcR, arcR, 0.0f, startAngle, angle, true);

        g.setColour (colour.withAlpha (0.22f));
        g.strokePath (val, juce::PathStrokeType (trackW + 7.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.setColour (colour);
        g.strokePath (val, juce::PathStrokeType (trackW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

    // indicatore
    juce::Path pointer;
    pointer.addRoundedRectangle (-1.5f, -bodyR + 3.0f, 3.0f, bodyR * 0.5f, 1.5f);
    g.setColour (juce::Colours::white);
    g.fillPath (pointer, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));
}

void MontageLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                               const juce::Colour& backgroundColour,
                                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown)
{
    const auto r = button.getLocalBounds().toFloat().reduced (0.5f);

    auto c = backgroundColour;
    if (shouldDrawButtonAsDown)
        c = c.brighter (0.25f);
    else if (shouldDrawButtonAsHighlighted)
        c = c.brighter (0.12f);

    g.setColour (c);
    g.fillRoundedRectangle (r, 5.0f);
    g.setColour (juce::Colours::white.withAlpha (0.10f));
    g.drawRoundedRectangle (r, 5.0f, 1.0f);
}

//==============================================================================
// PELLICOLA
WaveView::WaveView (MontageProcessor& p) : proc (p)
{
    rebuildPeaks();
    lastVersion = proc.getSampleVersion();
    lastEvent = proc.getGrainEventWrite();
    startTimerHz (30);
}

WaveView::~WaveView()
{
    stopTimer();
}

void WaveView::rebuildPeaks()
{
    peaks.clear();

    auto s = proc.getSample();

    if (s == nullptr)
    {
        sampleName = {};
        return;
    }

    sampleName = s->name;

    const int n = s->buffer.getNumSamples();
    const float* d = s->buffer.getReadPointer (0);
    const int cols = 700;

    peaks.reserve ((size_t) cols);

    for (int c = 0; c < cols; ++c)
    {
        const int i0 = (int) ((juce::int64) c * n / cols);
        const int i1 = juce::jmax (i0 + 1, (int) ((juce::int64) (c + 1) * n / cols));

        float mn = d[i0], mx = d[i0];

        for (int i = i0; i < i1 && i < n; ++i)
        {
            mn = juce::jmin (mn, d[i]);
            mx = juce::jmax (mx, d[i]);
        }

        peaks.emplace_back (mn, mx);
    }
}

void WaveView::timerCallback()
{
    const int v = proc.getSampleVersion();
    if (v != lastVersion)
    {
        lastVersion = v;
        rebuildPeaks();
    }

    // nuovi grani generati dal motore audio
    const uint32_t w = proc.getGrainEventWrite();

    if (w - lastEvent > (uint32_t) MontageProcessor::grainEventRing)
        lastEvent = w - (uint32_t) MontageProcessor::grainEventRing;

    while (lastEvent != w)
    {
        float pos = 0.0f, pan = 0.0f;
        bool rev = false;
        proc.getGrainEvent (lastEvent, pos, pan, rev);

        if (particles.size() < 700)
            particles.push_back ({ pos, pan, 1.0f, rev });

        ++lastEvent;
    }

    for (auto& p : particles)
        p.life -= 0.06f;

    particles.erase (std::remove_if (particles.begin(), particles.end(),
                                     [] (const Particle& p) { return p.life <= 0.0f; }),
                     particles.end());

    repaint();
}

void WaveView::setPositionFromMouse (const juce::MouseEvent& e)
{
    const float x = juce::jlimit (0.0f, 1.0f, (float) e.x / (float) juce::jmax (1, getWidth()));

    if (auto* prm = proc.apvts.getParameter ("position"))
        prm->setValueNotifyingHost (prm->convertTo0to1 (x));
}

void WaveView::mouseDown (const juce::MouseEvent& e)
{
    if (auto* prm = proc.apvts.getParameter ("position"))
        prm->beginChangeGesture();

    setPositionFromMouse (e);
}

void WaveView::mouseDrag (const juce::MouseEvent& e)
{
    setPositionFromMouse (e);
}

void WaveView::mouseUp (const juce::MouseEvent&)
{
    if (auto* prm = proc.apvts.getParameter ("position"))
        prm->endChangeGesture();
}

void WaveView::paint (juce::Graphics& g)
{
    const auto b = getLocalBounds().toFloat();
    const float stripH = 18.0f;

    // corpo della pellicola
    g.setColour (juce::Colour (0xff030306));
    g.fillRoundedRectangle (b, 6.0f);

    // strisce con le perforazioni
    g.setColour (juce::Colour (0xff101016));
    g.fillRect (b.getX(), b.getY(), b.getWidth(), stripH);
    g.fillRect (b.getX(), b.getBottom() - stripH, b.getWidth(), stripH);

    g.setColour (montage::bg);
    for (float x = b.getX() + 10.0f; x < b.getRight() - 14.0f; x += 24.0f)
    {
        g.fillRoundedRectangle (x, b.getY() + 5.0f, 12.0f, 8.0f, 2.0f);
        g.fillRoundedRectangle (x, b.getBottom() - stripH + 5.0f, 12.0f, 8.0f, 2.0f);
    }

    const auto frame = b.withTrimmedTop (stripH).withTrimmedBottom (stripH);

    // divisioni dei fotogrammi
    g.setColour (juce::Colours::white.withAlpha (0.05f));
    const int frames = 20;
    for (int i = 1; i < frames; ++i)
    {
        const float x = frame.getX() + frame.getWidth() * (float) i / (float) frames;
        g.drawVerticalLine ((int) x, frame.getY(), frame.getBottom());
    }

    if (peaks.empty())
    {
        g.setColour (montage::teal.withAlpha (0.85f));
        g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
        g.drawFittedText ("DROP A SAMPLE HERE", frame.toNearestInt().withTrimmedBottom (24),
                          juce::Justification::centred, 1);

        g.setColour (montage::dim);
        g.setFont (juce::FontOptions (12.0f));
        g.drawFittedText ("trascina un file audio qui, oppure premi LOAD SAMPLE",
                          frame.toNearestInt().withTrimmedTop (36), juce::Justification::centred, 1);
        return;
    }

    const float pos = proc.apvts.getRawParameterValue ("position")->load();
    const float scatter = proc.apvts.getRawParameterValue ("scatter")->load();
    const float cutRate = proc.apvts.getRawParameterValue ("cutRate")->load();
    const float cutSteps = proc.apvts.getRawParameterValue ("cutSteps")->load();
    const float cutSpan = proc.apvts.getRawParameterValue ("cutSpan")->load();

    // zona di pesca dei grani
    g.setColour (montage::orange.withAlpha (0.13f));
    auto band = [&] (float a, float c)
    {
        g.fillRect (frame.getX() + a * frame.getWidth(), frame.getY(), (c - a) * frame.getWidth(), frame.getHeight());
    };

    const float bl = pos - scatter * 0.5f;
    const float br = pos + scatter * 0.5f;
    band (juce::jmax (0.0f, bl), juce::jmin (1.0f, br));
    if (bl < 0.0f) band (1.0f + bl, 1.0f);
    if (br > 1.0f) band (0.0f, br - 1.0f);

    // onda
    const float midY = frame.getCentreY();
    const float half = frame.getHeight() * 0.46f;
    const float n = (float) peaks.size();

    g.setGradientFill (juce::ColourGradient (montage::teal.withAlpha (0.9f), 0.0f, frame.getY(),
                                             montage::orange.withAlpha (0.9f), 0.0f, frame.getBottom(), false));

    for (size_t c = 0; c < peaks.size(); ++c)
    {
        const float x = frame.getX() + ((float) c + 0.5f) * frame.getWidth() / n;
        g.drawLine (x, midY - peaks[c].second * half, x, midY - peaks[c].first * half, 1.0f);
    }

    // stacchi di montaggio (CUTS)
    const bool cutsOn = cutRate > 0.05f;
    const int steps = juce::jmax (2, juce::roundToInt (cutSteps));
    const int activeStep = proc.getCutStep();

    if (cutsOn)
    {
        for (int k = 0; k < steps; ++k)
        {
            const float p = std::fmod (pos + (float) k / (float) steps * cutSpan, 1.0f);
            const float x = frame.getX() + p * frame.getWidth();
            const bool active = (k == activeStep);

            if (active)
            {
                g.setColour (montage::teal.withAlpha (0.14f));
                g.fillRect (x - 9.0f, frame.getY(), 18.0f, frame.getHeight());
            }

            g.setColour (montage::teal.withAlpha (active ? 0.95f : 0.35f));
            g.drawLine (x, frame.getY(), x, frame.getBottom(), active ? 2.0f : 1.0f);

            juce::Path tri;
            tri.addTriangle (x - 5.0f, frame.getY(), x + 5.0f, frame.getY(), x, frame.getY() + 8.0f);
            g.fillPath (tri);
        }
    }

    // grani in volo
    for (const auto& p : particles)
    {
        const float x = frame.getX() + p.x * frame.getWidth();
        const float y = midY + p.y * half * 0.9f - (1.0f - p.life) * 10.0f;
        const auto c = p.reversed ? montage::teal : montage::orange;
        const float s = 2.5f + p.life * 2.5f;

        g.setColour (c.withAlpha (p.life * 0.18f));
        g.fillEllipse (x - s * 2.2f, y - s * 2.2f, s * 4.4f, s * 4.4f);
        g.setColour (c.brighter (0.6f).withAlpha (p.life));
        g.fillEllipse (x - s * 0.5f, y - s * 0.5f, s, s);
    }

    // linea di posizione
    const float px = frame.getX() + pos * frame.getWidth();
    g.setColour (montage::orange.withAlpha (0.25f));
    g.drawLine (px, frame.getY(), px, frame.getBottom(), 6.0f);
    g.setColour (montage::orange);
    g.drawLine (px, frame.getY(), px, frame.getBottom(), 2.0f);

    // dati in sovraimpressione
    g.setFont (monoFont (12.0f));

    g.setColour (montage::dim);
    g.drawText (sampleName, (int) frame.getX() + 10, (int) frame.getY() + 4, 320, 16,
                juce::Justification::centredLeft);

    const juce::String cutText = cutsOn
        ? "CUT " + juce::String (activeStep + 1).paddedLeft ('0', 2) + "/" + juce::String (steps).paddedLeft ('0', 2)
        : juce::String ("CUT --");

    g.setColour (cutsOn ? montage::teal : montage::dim);
    g.drawText (cutText, (int) frame.getRight() - 160, (int) frame.getY() + 4, 80, 16,
                juce::Justification::centredRight);

    const bool live = proc.getActiveVoices() > 0;
    g.setColour (live ? montage::orange : juce::Colour (0xff3a3a46));
    g.fillEllipse (frame.getRight() - 64.0f, frame.getY() + 8.0f, 8.0f, 8.0f);
    g.setColour (live ? montage::text : montage::dim);
    g.drawText ("LIVE", (int) frame.getRight() - 52, (int) frame.getY() + 4, 44, 16,
                juce::Justification::centredLeft);
}

//==============================================================================
// EDITOR
MontageEditor::MontageEditor (MontageProcessor& p)
    : juce::AudioProcessorEditor (&p), proc (p), waveView (p)
{
    setLookAndFeel (&laf);
    setSize (980, 640);

    addAndMakeVisible (waveView);

    // Ordine = ordine di layout (vedi resized)
    // Riga 1: GRAINS (6) + PITCH (2)    Riga 2: CUTS (4) + ENVELOPE (2) + OUTPUT (3)
    addKnob ("position",    "POSITION",  montage::orange);
    addKnob ("scatter",     "SCATTER",   montage::orange);
    addKnob ("grainSize",   "SIZE",      montage::orange);
    addKnob ("density",     "DENSITY",   montage::orange);
    addKnob ("reverse",     "REVERSE",   montage::orange);
    addKnob ("pan",         "WIDTH",     montage::orange);

    addKnob ("pitch",       "PITCH",     montage::orange);
    addKnob ("pitchSpread", "DRIFT",     montage::orange);

    addKnob ("cutRate",     "RATE",      montage::teal);
    addKnob ("cutSteps",    "STEPS",     montage::teal);
    addKnob ("cutRandom",   "CHAOS",     montage::teal);
    addKnob ("cutSpan",     "SPAN",      montage::teal);

    addKnob ("attack",      "ATTACK",    montage::orange);
    addKnob ("release",     "RELEASE",   montage::orange);

    addKnob ("cutoff",      "FILTER",    montage::orange);
    addKnob ("resonance",   "RES",       montage::orange);
    addKnob ("volume",      "LEVEL",     montage::orange);

    // Preset
    for (int i = 0; i < proc.getNumPrograms(); ++i)
        presetBox.addItem (proc.getProgramName (i), i + 1);

    presetBox.setSelectedItemIndex (proc.getCurrentProgram(), juce::dontSendNotification);
    presetBox.onChange = [this] { proc.setCurrentProgram (presetBox.getSelectedItemIndex()); };
    addAndMakeVisible (presetBox);

    auto stepPreset = [this] (int delta)
    {
        const int n = presetBox.getNumItems();
        if (n == 0)
            return;
        const int idx = (presetBox.getSelectedItemIndex() + delta + n) % n;
        presetBox.setSelectedItemIndex (idx, juce::sendNotification);
    };

    prevButton.onClick = [stepPreset] { stepPreset (-1); };
    nextButton.onClick = [stepPreset] { stepPreset (1); };
    addAndMakeVisible (prevButton);
    addAndMakeVisible (nextButton);

    // Pulsanti
    loadButton.onClick = [this] { openFileChooser(); };
    mutateButton.onClick = [this] { proc.mutate(); };
    variationButton.onClick = [this] { proc.variation(); };

    mutateButton.setColour (juce::TextButton::buttonColourId, montage::orange.darker (0.35f));
    mutateButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);

    addAndMakeVisible (loadButton);
    addAndMakeVisible (mutateButton);
    addAndMakeVisible (variationButton);
}

MontageEditor::~MontageEditor()
{
    setLookAndFeel (nullptr);
}

void MontageEditor::addKnob (const juce::String& paramId, const juce::String& name, juce::Colour colour)
{
    auto k = std::make_unique<Knob>();

    k->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 18);
    k->slider.setTextBoxIsEditable (false);
    k->slider.setColour (juce::Slider::rotarySliderFillColourId, colour);
    k->slider.setColour (juce::Slider::textBoxTextColourId, montage::text);
    k->slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);

    k->label.setText (name, juce::dontSendNotification);
    k->label.setJustificationType (juce::Justification::centred);
    k->label.setColour (juce::Label::textColourId, montage::dim);
    k->label.setFont (juce::FontOptions (11.0f, juce::Font::bold));

    addAndMakeVisible (k->slider);
    addAndMakeVisible (k->label);

    k->attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        proc.apvts, paramId, k->slider);

    knobs.push_back (std::move (k));
}

//==============================================================================
void MontageEditor::openFileChooser()
{
    chooser = std::make_unique<juce::FileChooser> ("Scegli un sample",
                                                   juce::File(),
                                                   "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg;*.m4a");

    juce::Component::SafePointer<MontageEditor> safe (this);

    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [safe] (const juce::FileChooser& fc)
                          {
                              if (safe == nullptr)
                                  return;

                              const auto f = fc.getResult();
                              if (f.existsAsFile())
                                  safe->proc.loadSample (f);
                          });
}

bool MontageEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
        if (juce::File (f).hasFileExtension ("wav;aif;aiff;flac;mp3;ogg;m4a"))
            return true;

    return false;
}

void MontageEditor::filesDropped (const juce::StringArray& files, int, int)
{
    for (auto& f : files)
        if (juce::File (f).hasFileExtension ("wav;aif;aiff;flac;mp3;ogg;m4a"))
            if (proc.loadSample (juce::File (f)))
                return;
}

//==============================================================================
void MontageEditor::paint (juce::Graphics& g)
{
    // sfondo
    g.setGradientFill (juce::ColourGradient (montage::bg2, 0.0f, 0.0f,
                                             montage::bg, 0.0f, (float) getHeight(), false));
    g.fillAll();

    // intestazione
    g.setColour (juce::Colour (0xff0a0a10));
    g.fillRect (0, 0, getWidth(), 104);

    g.setGradientFill (juce::ColourGradient (montage::teal, 0.0f, 0.0f,
                                             montage::orange, (float) getWidth(), 0.0f, false));
    g.fillRect (0, 103, getWidth(), 3);

    // titolo grande
    g.setGradientFill (juce::ColourGradient (juce::Colours::white, 20.0f, 0.0f,
                                             montage::orange, 620.0f, 0.0f, false));
    g.setFont (juce::FontOptions (56.0f, juce::Font::bold));
    g.drawFittedText ("GIUSEPPE RANDAZZO", 20, 6, 620, 64, juce::Justification::centredLeft, 1, 0.7f);

    g.setColour (montage::teal);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("M O N T A G E", 22, 70, 220, 26, juce::Justification::centredLeft);

    g.setColour (montage::dim);
    g.setFont (monoFont (12.0f));
    g.drawText ("CUT  /  SCATTER  /  REBUILD", 250, 70, 300, 26, juce::Justification::centredLeft);

    // pannelli delle sezioni
    for (const auto& s : sections)
    {
        const auto r = s.bounds.toFloat();

        g.setColour (montage::panel);
        g.fillRoundedRectangle (r, 8.0f);
        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.drawRoundedRectangle (r.reduced (0.5f), 8.0f, 1.0f);

        g.setColour (montage::teal.withAlpha (0.9f));
        g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
        g.drawText (spaced (s.title), s.bounds.getX() + 12, s.bounds.getY() + 5, 220, 16,
                    juce::Justification::centredLeft);
    }
}

void MontageEditor::resized()
{
    // Controlli in alto a destra
    prevButton.setBounds (640, 16, 30, 26);
    presetBox.setBounds (674, 16, 222, 26);
    nextButton.setBounds (900, 16, 30, 26);

    loadButton.setBounds (640, 54, 120, 28);
    mutateButton.setBounds (766, 54, 90, 28);
    variationButton.setBounds (862, 54, 104, 28);

    waveView.setBounds (14, 114, getWidth() - 28, 186);

    // Sezioni e manopole
    struct SecDef { const char* title; int first; int count; int row; };
    static const SecDef defs[] =
    {
        { "GRAINS",   0, 6, 0 },
        { "PITCH",    6, 2, 0 },
        { "CUTS",     8, 4, 1 },
        { "ENVELOPE", 12, 2, 1 },
        { "OUTPUT",   14, 3, 1 },
    };

    sections.clear();

    const int totalW = getWidth() - 28;
    const int gap = 10, pad = 6, titleH = 22, labelH = 16, rowH = 150;
    const int rowY[2] = { 312, 472 };

    for (int row = 0; row < 2; ++row)
    {
        int knobsInRow = 0, secsInRow = 0;

        for (auto& d : defs)
            if (d.row == row)
            {
                knobsInRow += d.count;
                ++secsInRow;
            }

        const int cellW = (totalW - gap * (secsInRow - 1) - pad * 2 * secsInRow) / knobsInRow;
        int x = 14;

        for (auto& d : defs)
        {
            if (d.row != row)
                continue;

            const int secW = pad * 2 + d.count * cellW;
            sections.push_back ({ d.title, juce::Rectangle<int> (x, rowY[row], secW, rowH) });

            for (int k = 0; k < d.count; ++k)
            {
                auto& kn = *knobs[(size_t) (d.first + k)];
                const int kx = x + pad + k * cellW;

                kn.label.setBounds (kx, rowY[row] + titleH, cellW, labelH);
                kn.slider.setBounds (kx, rowY[row] + titleH + labelH, cellW, rowH - titleH - labelH - 4);
            }

            x += secW + gap;
        }
    }
}
