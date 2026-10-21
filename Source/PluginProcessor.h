#pragma once

#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

#include <array>
#include <atomic>
#include <cstdint>
#include <memory>

// Sample caricato in memoria (immutabile dopo il caricamento)
struct SampleData
{
    juce::AudioBuffer<float> buffer;
    double sampleRate = 44100.0;
    juce::String name;
};

class MontageProcessor : public juce::AudioProcessor
{
public:
    MontageProcessor();
    ~MontageProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Montage"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 5.0; }

    int getNumPrograms() override;
    int getCurrentProgram() override { return currentProgram; }
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // Sample
    bool loadSample (const juce::File& file);
    std::shared_ptr<SampleData> getSample();
    int getSampleVersion() const { return sampleVersion.load(); }

    // Dati per la grafica (scritti dal thread audio, letti dal thread messaggi)
    static constexpr int grainEventRing = 512;
    uint32_t getGrainEventWrite() const { return evWrite.load (std::memory_order_acquire); }
    void getGrainEvent (uint32_t index, float& pos, float& pan, bool& reversed) const
    {
        const auto i = (size_t) (index % (uint32_t) grainEventRing);
        pos = evPos[i].load (std::memory_order_relaxed);
        pan = evPan[i].load (std::memory_order_relaxed);
        reversed = evRev[i].load (std::memory_order_relaxed) > 0.5f;
    }
    int getCutStep() const { return cutStepIndex.load(); }
    int getActiveVoices() const { return activeVoices.load(); }

    // Varianti
    void mutate();     // ricalcola in modo drastico i parametri principali
    void variation();  // piccola deriva casuale intorno ai valori attuali

    juce::AudioProcessorValueTreeState apvts;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void setParamPlain (const char* id, float plainValue);

    // ---- motore granulare ----
    struct Grain
    {
        bool active = false;
        double pos = 0.0;   // posizione di lettura nel sample (in campioni sorgente)
        double inc = 1.0;   // incremento per campione di uscita (negativo = reverse)
        int age = 0;
        int length = 1;
        float gainL = 1.0f;
        float gainR = 1.0f;
    };

    static constexpr int maxGrains = 48;
    static constexpr int maxVoices = 8;

    struct Voice
    {
        bool active = false;
        bool releasing = false;
        int note = -1;
        float velocity = 0.0f;
        float env = 0.0f;
        double spawnAccum = 0.0;
        std::array<Grain, maxGrains> grains;

        void start (int n, float vel)
        {
            active = true;
            releasing = false;
            note = n;
            velocity = vel;
            env = 0.0f;
            spawnAccum = 1.0e9; // genera subito il primo grano
            for (auto& g : grains)
                g.active = false;
        }

        void kill()
        {
            active = false;
            releasing = false;
            for (auto& g : grains)
                g.active = false;
        }
    };

    struct BlockParams
    {
        float position = 0.3f, scatter = 0.1f, grainMs = 80.0f, density = 20.0f;
        float pitch = 0.0f, pitchSpread = 0.0f, reverseProb = 0.0f, panSpread = 0.2f;
        float attackMs = 10.0f, releaseMs = 300.0f, cutoff = 18000.0f, resonance = 0.71f;
        float cutRate = 0.0f, cutSteps = 4.0f, cutRandom = 0.0f, cutSpan = 0.5f;
    };

    void updateParams();
    void advanceCut();
    void pushGrainEvent (float pos, float pan, bool reversed);
    void handleMidi (const juce::MidiMessage& m);
    void noteOn (int note, float velocity);
    void noteOff (int note);
    void spawnGrain (Voice& v, const SampleData& s, int grainLen, double srcRatio);
    void render (juce::AudioBuffer<float>& buffer, int start, int end, const SampleData& sample);

    static float readSample (const SampleData& s, int channel, double pos);

    std::array<Voice, maxVoices> voices;
    int stealIndex = 0;
    BlockParams params;
    juce::Random random;
    double currentSampleRate = 44100.0;

    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::SmoothedValue<float, juce::ValueSmoothingTypes::Linear> gainSmoothed;

    // Parametri (puntatori cache, sicuri per il thread audio)
    std::atomic<float>* pPosition = nullptr;
    std::atomic<float>* pScatter = nullptr;
    std::atomic<float>* pGrainSize = nullptr;
    std::atomic<float>* pDensity = nullptr;
    std::atomic<float>* pPitch = nullptr;
    std::atomic<float>* pPitchSpread = nullptr;
    std::atomic<float>* pReverse = nullptr;
    std::atomic<float>* pPan = nullptr;
    std::atomic<float>* pAttack = nullptr;
    std::atomic<float>* pRelease = nullptr;
    std::atomic<float>* pVolume = nullptr;
    std::atomic<float>* pCutoff = nullptr;
    std::atomic<float>* pResonance = nullptr;
    std::atomic<float>* pCutRate = nullptr;
    std::atomic<float>* pCutSteps = nullptr;
    std::atomic<float>* pCutRandom = nullptr;
    std::atomic<float>* pCutSpan = nullptr;

    // Sample condiviso tra thread messaggi e thread audio
    juce::AudioFormatManager formatManager;
    juce::SpinLock sampleLock;
    std::shared_ptr<SampleData> sampleData;
    std::atomic<int> sampleVersion { 0 };

    // CUTS: stacchi di montaggio
    double cutAccum = 0.0;
    int cutStep = 0;
    float cutOffset = 0.0f;
    std::atomic<int> cutStepIndex { 0 };
    std::atomic<int> activeVoices { 0 };

    // Eventi grano per la grafica
    std::array<std::atomic<float>, grainEventRing> evPos, evPan, evRev;
    std::atomic<uint32_t> evWrite { 0 };

    int currentProgram = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MontageProcessor)
};
