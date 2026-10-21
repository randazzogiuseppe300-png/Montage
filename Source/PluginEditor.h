#pragma once

#include "PluginProcessor.h"

#include <memory>
#include <utility>
#include <vector>

// Palette "color grading" da cinema: arancio + teal su nero
namespace montage
{
    inline const juce::Colour bg     (0xff060609);
    inline const juce::Colour bg2    (0xff12121a);
    inline const juce::Colour panel  (0xff0d0d13);
    inline const juce::Colour orange (0xffff7a1a);
    inline const juce::Colour teal   (0xff19d3c5);
    inline const juce::Colour text   (0xffe9e9ee);
    inline const juce::Colour dim    (0xff8a8a98);
}

//==============================================================================
// Aspetto grafico personalizzato: manopole ad anello luminoso, pulsanti piatti
class MontageLookAndFeel : public juce::LookAndFeel_V4
{
public:
    MontageLookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle, juce::Slider&) override;

    void drawButtonBackground (juce::Graphics&, juce::Button&, const juce::Colour& backgroundColour,
                               bool shouldDrawButtonAsHighlighted, bool shouldDrawButtonAsDown) override;
};

//==============================================================================
// Pellicola: forma d'onda, stacchi di montaggio e grani in volo
class WaveView : public juce::Component,
                 private juce::Timer
{
public:
    explicit WaveView (MontageProcessor&);
    ~WaveView() override;

    void paint (juce::Graphics&) override;

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;

private:
    struct Particle
    {
        float x;       // 0..1 posizione nel sample
        float y;       // -1..1 pan
        float life;    // 1 -> 0
        bool reversed;
    };

    void rebuildPeaks();
    void timerCallback() override;
    void setPositionFromMouse (const juce::MouseEvent&);

    MontageProcessor& proc;

    std::vector<std::pair<float, float>> peaks;
    std::vector<Particle> particles;
    juce::String sampleName;
    int lastVersion = -1;
    uint32_t lastEvent = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveView)
};

//==============================================================================
class MontageEditor : public juce::AudioProcessorEditor,
                      public juce::FileDragAndDropTarget
{
public:
    explicit MontageEditor (MontageProcessor&);
    ~MontageEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

private:
    struct Knob
    {
        juce::Slider slider;
        juce::Label label;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    };

    struct Section
    {
        juce::String title;
        juce::Rectangle<int> bounds;
    };

    void addKnob (const juce::String& paramId, const juce::String& name, juce::Colour colour);
    void openFileChooser();

    MontageLookAndFeel laf;   // deve restare il primo membro: viene distrutto per ultimo

    MontageProcessor& proc;

    WaveView waveView;
    std::vector<std::unique_ptr<Knob>> knobs;
    std::vector<Section> sections;

    juce::TextButton loadButton { "LOAD SAMPLE" };
    juce::TextButton mutateButton { "MUTATE" };
    juce::TextButton variationButton { "VARIATION" };
    juce::TextButton prevButton { "<" };
    juce::TextButton nextButton { ">" };
    juce::ComboBox presetBox;

    std::unique_ptr<juce::FileChooser> chooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MontageEditor)
};
