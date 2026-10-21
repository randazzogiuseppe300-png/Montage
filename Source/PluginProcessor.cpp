#include "PluginProcessor.h"
#include "PluginEditor.h"

#include <cmath>

//==============================================================================
// Preset di fabbrica
namespace
{
    struct Preset
    {
        const char* name;
        float position, scatter, grainMs, density, pitch, pitchSpread, reverse, pan,
              attack, release, volume, cutoff, resonance;
    };

    const Preset presets[] =
    {
        //  nome                    pos    scat   size   dens  pitch spread rev    pan    atk    rel    vol    cutoff   res
        // ---- Clouds & Pads ----
        { "Init", 0.30f, 0.10f, 80.0f, 20.0f, 0.0f, 0.0f, 0.00f, 0.20f, 10.0f, 300.0f, -6.0f, 18000.0f, 0.71f },
        { "Cloud Wash", 0.50f, 0.60f, 220.0f, 35.0f, 0.0f, 2.0f, 0.10f, 0.80f, 600.0f, 1500.0f, -9.0f, 9000.0f, 0.71f },
        { "Soft Fog", 0.36f, 0.68f, 260.0f, 48.0f, 0.0f, 3.5f, 0.18f, 0.85f, 537.0f, 2049.0f, -7.0f, 10600.0f, 0.95f },
        { "Warm Haze", 0.45f, 0.30f, 161.0f, 26.0f, 7.0f, 0.4f, 0.00f, 0.96f, 592.0f, 1482.0f, -9.0f, 6200.0f, 1.20f },
        { "Velvet Cloud", 0.74f, 0.70f, 232.0f, 21.0f, -12.0f, 1.4f, 0.11f, 0.72f, 566.0f, 2998.0f, -8.0f, 8250.0f, 0.71f },
        { "Slow Bloom", 0.40f, 0.48f, 162.0f, 45.0f, -5.0f, 2.2f, 0.17f, 0.77f, 913.0f, 2962.0f, -7.0f, 6600.0f, 0.96f },
        { "Morning Mist", 0.55f, 0.69f, 232.0f, 42.0f, 12.0f, 2.3f, 0.15f, 0.93f, 1140.0f, 1926.0f, -8.0f, 9800.0f, 0.91f },
        { "Glass Pad", 0.49f, 0.39f, 318.0f, 36.0f, 0.0f, 1.8f, 0.09f, 0.97f, 1179.0f, 1981.0f, -8.0f, 9250.0f, 0.85f },
        { "Floating Choir", 0.50f, 0.49f, 317.0f, 30.0f, -5.0f, 1.2f, 0.05f, 0.61f, 744.0f, 2113.0f, -9.0f, 7700.0f, 0.94f },
        { "Deep Drift", 0.31f, 0.49f, 231.0f, 43.0f, 0.0f, 3.2f, 0.11f, 0.96f, 950.0f, 1487.0f, -7.0f, 4500.0f, 1.04f },
        // ---- Glitch & Stutter ----
        { "Glitch Stutter", 0.25f, 0.02f, 25.0f, 40.0f, 0.0f, 0.0f, 0.30f, 0.30f, 2.0f, 80.0f, -6.0f, 16000.0f, 1.00f },
        { "Micro Cuts", 0.26f, 0.07f, 11.0f, 34.0f, -12.0f, 0.7f, 0.48f, 0.59f, 4.0f, 54.0f, -7.0f, 16000.0f, 0.82f },
        { "Stutter Gun", 0.09f, 0.04f, 19.0f, 57.0f, 0.0f, 0.6f, 0.18f, 0.27f, 1.0f, 72.0f, -7.0f, 14800.0f, 0.99f },
        { "Skip Machine", 0.31f, 0.05f, 28.0f, 66.0f, 0.0f, 0.5f, 0.39f, 0.28f, 1.0f, 77.0f, -5.0f, 16300.0f, 1.10f },
        { "Digital Hiccup", 0.42f, 0.01f, 10.0f, 53.0f, 0.0f, 0.6f, 0.30f, 0.43f, 1.0f, 110.0f, -7.0f, 17400.0f, 1.01f },
        { "Buffer Break", 0.05f, 0.03f, 14.0f, 52.0f, 0.0f, 0.0f, 0.30f, 0.40f, 2.0f, 51.0f, -8.0f, 17250.0f, 1.16f },
        { "Rapid Slice", 0.15f, 0.04f, 16.0f, 30.0f, 0.0f, 0.6f, 0.37f, 0.24f, 2.0f, 41.0f, -5.0f, 8300.0f, 1.14f },
        { "Chop Shop", 0.45f, 0.06f, 11.0f, 49.0f, 0.0f, 0.9f, 0.15f, 0.52f, 4.0f, 69.0f, -5.0f, 9100.0f, 1.38f },
        { "Static Jump", 0.27f, 0.04f, 32.0f, 32.0f, 12.0f, 0.2f, 0.17f, 0.48f, 5.0f, 111.0f, -7.0f, 9250.0f, 1.12f },
        { "Freeze Frame Glitch", 0.12f, 0.06f, 29.0f, 56.0f, 12.0f, 0.8f, 0.19f, 0.77f, 4.0f, 85.0f, -7.0f, 15100.0f, 1.38f },
        // ---- Reverse & Dreams ----
        { "Reverse Dream", 0.40f, 0.30f, 150.0f, 18.0f, -12.0f, 3.0f, 0.85f, 0.70f, 400.0f, 1200.0f, -8.0f, 6000.0f, 1.20f },
        { "Backwards Tide", 0.25f, 0.16f, 217.0f, 15.0f, 12.0f, 3.7f, 0.94f, 0.61f, 473.0f, 1101.0f, -7.0f, 7250.0f, 1.20f },
        { "Rewind Vapor", 0.31f, 0.34f, 110.0f, 13.0f, 5.0f, 4.8f, 0.75f, 0.84f, 573.0f, 1822.0f, -8.0f, 6400.0f, 1.15f },
        { "Mirror Lake", 0.57f, 0.17f, 235.0f, 12.0f, -7.0f, 2.0f, 0.70f, 0.93f, 701.0f, 1072.0f, -6.0f, 3200.0f, 0.96f },
        { "Ghost Reverse", 0.20f, 0.39f, 170.0f, 28.0f, -7.0f, 2.4f, 0.93f, 0.80f, 496.0f, 1612.0f, -6.0f, 4400.0f, 0.95f },
        { "Moon Backwards", 0.44f, 0.39f, 112.0f, 15.0f, 0.0f, 3.0f, 0.99f, 0.63f, 300.0f, 2029.0f, -9.0f, 7100.0f, 1.41f },
        { "Spiral Back", 0.66f, 0.25f, 241.0f, 24.0f, -7.0f, 1.5f, 0.93f, 0.66f, 699.0f, 1484.0f, -8.0f, 5900.0f, 1.31f },
        { "Dream Tunnel", 0.23f, 0.19f, 103.0f, 18.0f, -12.0f, 3.4f, 0.99f, 0.65f, 360.0f, 924.0f, -8.0f, 4700.0f, 1.34f },
        { "Lucid Reverse", 0.28f, 0.19f, 164.0f, 21.0f, 12.0f, 4.7f, 0.91f, 0.70f, 285.0f, 947.0f, -9.0f, 4200.0f, 1.14f },
        { "Time Fold", 0.87f, 0.20f, 178.0f, 11.0f, -7.0f, 1.7f, 0.71f, 0.93f, 627.0f, 2036.0f, -8.0f, 5600.0f, 1.32f },
        // ---- Dust & Texture ----
        { "Tiny Dust", 0.60f, 0.50f, 12.0f, 60.0f, 12.0f, 5.0f, 0.20f, 1.00f, 5.0f, 200.0f, -12.0f, 14000.0f, 0.71f },
        { "Sand Storm", 0.66f, 0.31f, 24.0f, 55.0f, 12.0f, 7.8f, 0.30f, 0.97f, 17.0f, 430.0f, -12.0f, 17650.0f, 1.18f },
        { "Pixel Rain", 0.56f, 0.44f, 20.0f, 52.0f, 12.0f, 4.8f, 0.24f, 0.96f, 16.0f, 296.0f, -12.0f, 9100.0f, 0.91f },
        { "Static Snow", 0.89f, 0.78f, 19.0f, 71.0f, 7.0f, 6.2f, 0.34f, 0.93f, 2.0f, 182.0f, -10.0f, 8950.0f, 1.07f },
        { "Crackle Field", 0.53f, 0.72f, 15.0f, 49.0f, 12.0f, 2.3f, 0.22f, 0.91f, 19.0f, 369.0f, -11.0f, 17500.0f, 0.75f },
        { "Fine Grain", 0.73f, 0.47f, 8.0f, 41.0f, 12.0f, 5.6f, 0.34f, 0.83f, 4.0f, 239.0f, -12.0f, 17600.0f, 0.79f },
        { "Ash Fall", 0.63f, 0.72f, 14.0f, 77.0f, 5.0f, 4.1f, 0.33f, 0.96f, 15.0f, 237.0f, -10.0f, 8200.0f, 0.86f },
        { "Insect Swarm", 0.16f, 0.74f, 11.0f, 74.0f, 12.0f, 5.9f, 0.10f, 0.87f, 5.0f, 142.0f, -13.0f, 13100.0f, 0.76f },
        { "Sparkle Dust", 0.48f, 0.74f, 16.0f, 52.0f, 19.0f, 5.7f, 0.29f, 0.87f, 16.0f, 121.0f, -11.0f, 15650.0f, 1.04f },
        { "Powder Noise", 0.92f, 0.47f, 6.0f, 79.0f, 7.0f, 6.9f, 0.33f, 0.85f, 9.0f, 131.0f, -12.0f, 12600.0f, 0.86f },
        // ---- Slow Motion & Freeze ----
        { "Slow Motion", 0.15f, 0.08f, 350.0f, 8.0f, -12.0f, 0.3f, 0.00f, 0.50f, 800.0f, 2500.0f, -6.0f, 5000.0f, 0.90f },
        { "Frozen Moment", 0.64f, 0.08f, 290.0f, 5.0f, -5.0f, 0.5f, 0.13f, 0.67f, 1432.0f, 2286.0f, -6.0f, 4950.0f, 0.76f },
        { "Amber Hold", 0.72f, 0.11f, 265.0f, 5.0f, -7.0f, 0.8f, 0.05f, 0.64f, 926.0f, 1787.0f, -6.0f, 3300.0f, 0.99f },
        { "Glacier", 0.75f, 0.05f, 327.0f, 8.0f, 0.0f, 0.4f, 0.08f, 0.43f, 882.0f, 1682.0f, -6.0f, 4450.0f, 0.90f },
        { "Stretch Time", 0.91f, 0.11f, 323.0f, 5.0f, -5.0f, 0.4f, 0.05f, 0.67f, 1365.0f, 1519.0f, -6.0f, 3350.0f, 0.80f },
        { "Still Water", 0.60f, 0.13f, 364.0f, 10.0f, -5.0f, 0.5f, 0.00f, 0.60f, 1208.0f, 3323.0f, -5.0f, 3650.0f, 0.93f },
        { "Drone Freeze", 0.51f, 0.10f, 297.0f, 7.0f, -7.0f, 0.2f, 0.12f, 0.69f, 621.0f, 4836.0f, -5.0f, 3200.0f, 0.90f },
        { "Long Exhale", 0.33f, 0.09f, 349.0f, 8.0f, -5.0f, 0.6f, 0.02f, 0.41f, 894.0f, 1875.0f, -6.0f, 7900.0f, 1.00f },
        { "Heavy Stillness", 0.64f, 0.06f, 294.0f, 8.0f, -7.0f, 0.3f, 0.13f, 0.64f, 526.0f, 2706.0f, -8.0f, 6400.0f, 0.78f },
        { "Suspended", 0.23f, 0.06f, 371.0f, 6.0f, -7.0f, 0.2f, 0.01f, 0.46f, 547.0f, 3578.0f, -6.0f, 5550.0f, 0.93f },
        // ---- Rhythmic ----
        { "Pulse Grains", 0.98f, 0.12f, 84.0f, 29.0f, 0.0f, 1.2f, 0.10f, 0.53f, 2.0f, 45.0f, -6.0f, 14400.0f, 1.54f },
        { "Tick Tock", 0.53f, 0.17f, 64.0f, 26.0f, -5.0f, 2.0f, 0.01f, 0.32f, 2.0f, 158.0f, -7.0f, 7550.0f, 1.65f },
        { "Rolling Bits", 0.87f, 0.10f, 59.0f, 23.0f, -5.0f, 0.7f, 0.20f, 0.40f, 4.0f, 95.0f, -7.0f, 11000.0f, 1.78f },
        { "Tight Shuffle", 0.14f, 0.16f, 79.0f, 14.0f, 12.0f, 1.6f, 0.17f, 0.34f, 2.0f, 150.0f, -7.0f, 17300.0f, 1.46f },
        { "Percussive Rain", 0.46f, 0.04f, 32.0f, 7.0f, 0.0f, 0.3f, 0.13f, 0.44f, 3.0f, 42.0f, -7.0f, 8950.0f, 1.30f },
        { "Hand Drum Chop", 0.68f, 0.08f, 89.0f, 6.0f, 5.0f, 1.0f, 0.09f, 0.23f, 2.0f, 156.0f, -6.0f, 14700.0f, 1.06f },
        { "Machine Heart", 0.16f, 0.04f, 80.0f, 11.0f, 0.0f, 0.9f, 0.17f, 0.29f, 2.0f, 190.0f, -7.0f, 7150.0f, 1.79f },
        { "Clockwork", 0.72f, 0.09f, 46.0f, 7.0f, 5.0f, 0.6f, 0.15f, 0.67f, 3.0f, 87.0f, -5.0f, 7550.0f, 0.91f },
        { "Snappy Grains", 0.08f, 0.14f, 63.0f, 7.0f, -5.0f, 0.3f, 0.13f, 0.29f, 7.0f, 195.0f, -7.0f, 17450.0f, 1.41f },
        { "Beat Shredder", 0.79f, 0.06f, 47.0f, 5.0f, 12.0f, 1.0f, 0.18f, 0.62f, 3.0f, 50.0f, -5.0f, 11100.0f, 1.77f },
        // ---- Lo-Fi & Tape ----
        { "Broken Tape", 0.70f, 0.25f, 60.0f, 25.0f, -5.0f, 6.0f, 0.40f, 0.40f, 20.0f, 400.0f, -7.0f, 4000.0f, 2.00f },
        { "Old Cassette", 0.38f, 0.29f, 42.0f, 27.0f, 0.0f, 5.8f, 0.40f, 0.59f, 11.0f, 360.0f, -7.0f, 3900.0f, 1.95f },
        { "Dusty Vinyl", 0.81f, 0.19f, 44.0f, 16.0f, -5.0f, 5.6f, 0.28f, 0.24f, 36.0f, 545.0f, -6.0f, 3150.0f, 1.56f },
        { "Radio Ghost", 0.90f, 0.14f, 45.0f, 16.0f, -3.0f, 5.7f, 0.20f, 0.59f, 33.0f, 219.0f, -7.0f, 1750.0f, 2.15f },
        { "Wobbly Reel", 0.72f, 0.19f, 67.0f, 20.0f, -7.0f, 7.5f, 0.38f, 0.36f, 23.0f, 579.0f, -8.0f, 2700.0f, 1.63f },
        { "Basement Tape", 0.19f, 0.13f, 68.0f, 21.0f, 0.0f, 5.6f, 0.30f, 0.29f, 32.0f, 388.0f, -7.0f, 2150.0f, 2.00f },
        { "Worn Out", 0.27f, 0.23f, 90.0f, 15.0f, -7.0f, 6.0f, 0.31f, 0.58f, 12.0f, 317.0f, -8.0f, 1650.0f, 1.04f },
        { "VHS Memory", 0.81f, 0.34f, 97.0f, 21.0f, 0.0f, 7.9f, 0.20f, 0.22f, 34.0f, 247.0f, -8.0f, 2350.0f, 1.16f },
        { "Muffled Past", 0.22f, 0.35f, 97.0f, 24.0f, -5.0f, 7.5f, 0.32f, 0.21f, 47.0f, 682.0f, -7.0f, 4150.0f, 1.52f },
        { "Dirty Loop", 0.96f, 0.37f, 116.0f, 26.0f, -5.0f, 7.9f, 0.12f, 0.24f, 29.0f, 284.0f, -9.0f, 3150.0f, 0.86f },
        // ---- Bass & Low ----
        { "Sub Cloud", 0.05f, 0.15f, 116.0f, 26.0f, -12.0f, 0.8f, 0.11f, 0.15f, 6.0f, 326.0f, -5.0f, 690.0f, 1.21f },
        { "Low Rumble", 0.29f, 0.12f, 102.0f, 28.0f, -12.0f, 0.3f, 0.03f, 0.06f, 24.0f, 348.0f, -4.0f, 970.0f, 1.60f },
        { "Deep Grain Bass", 0.24f, 0.15f, 70.0f, 38.0f, -12.0f, 1.3f, 0.16f, 0.25f, 13.0f, 523.0f, -5.0f, 530.0f, 1.64f },
        { "Heavy Machine", 0.10f, 0.17f, 67.0f, 30.0f, -12.0f, 0.9f, 0.13f, 0.10f, 35.0f, 224.0f, -5.0f, 310.0f, 1.23f },
        { "Dark Foundation", 0.26f, 0.06f, 86.0f, 29.0f, -12.0f, 1.0f, 0.11f, 0.09f, 15.0f, 770.0f, -6.0f, 1100.0f, 2.01f },
        { "Tectonic", 0.07f, 0.14f, 148.0f, 18.0f, -17.0f, 1.4f, 0.19f, 0.14f, 6.0f, 201.0f, -5.0f, 390.0f, 2.40f },
        { "Thick Mud", 0.31f, 0.07f, 72.0f, 28.0f, -12.0f, 0.0f, 0.12f, 0.03f, 28.0f, 370.0f, -7.0f, 600.0f, 1.27f },
        { "Bass Smear", 0.51f, 0.17f, 81.0f, 31.0f, -19.0f, 0.6f, 0.06f, 0.09f, 7.0f, 467.0f, -7.0f, 440.0f, 1.99f },
        { "Under Water", 0.16f, 0.14f, 89.0f, 33.0f, -19.0f, 0.3f, 0.03f, 0.10f, 15.0f, 267.0f, -5.0f, 1550.0f, 1.71f },
        { "Night Engine", 0.49f, 0.08f, 182.0f, 19.0f, -19.0f, 0.8f, 0.01f, 0.19f, 8.0f, 253.0f, -6.0f, 850.0f, 1.56f },
        // ---- Bright & Shimmer ----
        { "Shimmer Rain", 0.64f, 0.57f, 50.0f, 51.0f, 12.0f, 5.0f, 0.18f, 0.91f, 86.0f, 684.0f, -10.0f, 17400.0f, 1.05f },
        { "Crystal Sparks", 0.29f, 0.57f, 84.0f, 41.0f, 7.0f, 5.2f, 0.14f, 0.88f, 11.0f, 897.0f, -11.0f, 14350.0f, 1.06f },
        { "Bell Cloud", 0.56f, 0.45f, 73.0f, 57.0f, 19.0f, 1.1f, 0.11f, 0.83f, 41.0f, 963.0f, -10.0f, 12450.0f, 0.78f },
        { "High Glitter", 0.55f, 0.69f, 24.0f, 37.0f, 12.0f, 3.3f, 0.14f, 0.90f, 100.0f, 573.0f, -12.0f, 11350.0f, 0.70f },
        { "Silver Air", 0.76f, 0.56f, 40.0f, 37.0f, 12.0f, 4.2f, 0.26f, 0.84f, 75.0f, 1647.0f, -12.0f, 10300.0f, 0.73f },
        { "Angel Dust", 0.15f, 0.49f, 72.0f, 27.0f, 19.0f, 6.0f, 0.25f, 0.88f, 125.0f, 567.0f, -10.0f, 12350.0f, 1.10f },
        { "Ice Chimes", 0.50f, 0.48f, 34.0f, 27.0f, 12.0f, 1.2f, 0.10f, 0.93f, 12.0f, 1064.0f, -12.0f, 9750.0f, 1.08f },
        { "Bright Swarm", 0.86f, 0.54f, 66.0f, 32.0f, 12.0f, 5.0f, 0.25f, 0.94f, 17.0f, 1419.0f, -11.0f, 14450.0f, 0.93f },
        { "Star Field", 0.26f, 0.47f, 50.0f, 43.0f, 7.0f, 2.2f, 0.26f, 0.90f, 30.0f, 710.0f, -11.0f, 10850.0f, 0.88f },
        { "Glass Rain", 0.74f, 0.65f, 27.0f, 61.0f, 12.0f, 4.6f, 0.16f, 0.87f, 29.0f, 1873.0f, -12.0f, 17250.0f, 0.80f },
        // ---- Experimental ----
        { "Total Chaos", 0.60f, 0.47f, 27.0f, 58.0f, -12.0f, 9.1f, 0.60f, 0.70f, 19.0f, 378.0f, -10.0f, 1100.0f, 1.74f },
        { "Alien Choir", 0.76f, 0.56f, 28.0f, 10.0f, 3.0f, 4.1f, 0.41f, 0.87f, 238.0f, 1541.0f, -12.0f, 2750.0f, 0.82f },
        { "Broken Machine", 0.86f, 0.44f, 346.0f, 39.0f, -23.0f, 4.9f, 0.40f, 0.63f, 8.0f, 2212.0f, -8.0f, 810.0f, 1.31f },
        { "Wild Scatter", 0.23f, 0.24f, 33.0f, 49.0f, -20.0f, 9.0f, 0.47f, 0.84f, 41.0f, 800.0f, -12.0f, 1150.0f, 3.15f },
        { "Random Storm", 0.12f, 1.00f, 266.0f, 39.0f, -10.0f, 4.6f, 0.55f, 0.87f, 15.0f, 91.0f, -6.0f, 4300.0f, 0.77f },
        { "Mutant Texture", 0.41f, 0.50f, 109.0f, 39.0f, -6.0f, 5.5f, 0.23f, 0.66f, 6.0f, 2952.0f, -9.0f, 4900.0f, 2.70f },
        { "Noise Garden", 0.45f, 0.66f, 191.0f, 12.0f, -20.0f, 9.1f, 0.67f, 0.64f, 220.0f, 1638.0f, -9.0f, 1800.0f, 2.77f },
        { "Unstable Orbit", 0.62f, 0.36f, 15.0f, 40.0f, 9.0f, 6.4f, 0.73f, 0.60f, 14.0f, 242.0f, -6.0f, 10450.0f, 0.73f },
        { "Cosmic Shred", 0.67f, 0.42f, 19.0f, 9.0f, 12.0f, 4.7f, 0.52f, 0.56f, 43.0f, 1027.0f, -9.0f, 2250.0f, 2.90f },
        { "Bizarre Bloom", 0.17f, 0.36f, 21.0f, 4.0f, -16.0f, 6.0f, 0.59f, 0.68f, 4.0f, 109.0f, -10.0f, 970.0f, 2.49f },
    };

    const int numPresets = (int) (sizeof (presets) / sizeof (presets[0]));

    juce::AudioParameterFloatAttributes attrs (const juce::String& suffix, int decimals)
    {
        return juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([suffix, decimals] (float v, int)
                                          { return juce::String (v, decimals) + suffix; });
    }

    juce::AudioParameterFloatAttributes pctAttrs()
    {
        return juce::AudioParameterFloatAttributes()
            .withStringFromValueFunction ([] (float v, int)
                                          { return juce::String (juce::roundToInt (v * 100.0f)) + " %"; });
    }
}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout MontageProcessor::createParameterLayout()
{
    using P = juce::AudioParameterFloat;
    using R = juce::NormalisableRange<float>;
    using ID = juce::ParameterID;

    juce::AudioProcessorValueTreeState::ParameterLayout l;

    l.add (std::make_unique<P> (ID { "position", 1 },    "Position",     R (0.0f, 1.0f),                 0.30f, pctAttrs()));
    l.add (std::make_unique<P> (ID { "scatter", 1 },     "Scatter",      R (0.0f, 1.0f),                 0.10f, pctAttrs()));
    l.add (std::make_unique<P> (ID { "grainSize", 1 },   "Grain Size",   R (5.0f, 400.0f, 0.0f, 0.5f),   80.0f, attrs (" ms", 0)));
    l.add (std::make_unique<P> (ID { "density", 1 },     "Density",      R (1.0f, 80.0f, 0.0f, 0.6f),    20.0f, attrs (" /s", 0)));
    l.add (std::make_unique<P> (ID { "pitch", 1 },       "Pitch",        R (-24.0f, 24.0f, 1.0f),        0.0f,  attrs (" st", 0)));
    l.add (std::make_unique<P> (ID { "pitchSpread", 1 }, "Pitch Spread", R (0.0f, 12.0f),                0.0f,  attrs (" st", 1)));
    l.add (std::make_unique<P> (ID { "reverse", 1 },     "Reverse",      R (0.0f, 1.0f),                 0.0f,  pctAttrs()));
    l.add (std::make_unique<P> (ID { "pan", 1 },         "Stereo Spread", R (0.0f, 1.0f),                0.2f,  pctAttrs()));
    l.add (std::make_unique<P> (ID { "attack", 1 },      "Attack",       R (1.0f, 2000.0f, 0.0f, 0.4f),  10.0f, attrs (" ms", 0)));
    l.add (std::make_unique<P> (ID { "release", 1 },     "Release",      R (5.0f, 5000.0f, 0.0f, 0.4f),  300.0f, attrs (" ms", 0)));
    l.add (std::make_unique<P> (ID { "volume", 1 },      "Volume",       R (-36.0f, 6.0f),               -6.0f, attrs (" dB", 1)));
    l.add (std::make_unique<P> (ID { "cutoff", 1 },      "Filter",       R (20.0f, 20000.0f, 0.0f, 0.3f), 18000.0f, attrs (" Hz", 0)));
    l.add (std::make_unique<P> (ID { "resonance", 1 },   "Resonance",    R (0.5f, 10.0f, 0.0f, 0.5f),    0.71f, attrs ("", 2)));

    // CUTS: stacchi di montaggio
    l.add (std::make_unique<P> (ID { "cutRate", 1 },     "Cut Rate",     R (0.0f, 16.0f, 0.0f, 0.5f),    0.0f,
                                juce::AudioParameterFloatAttributes().withStringFromValueFunction ([] (float v, int)
                                { return v < 0.05f ? juce::String ("OFF") : juce::String (v, 1) + " Hz"; })));
    l.add (std::make_unique<P> (ID { "cutSteps", 1 },    "Cut Steps",    R (2.0f, 16.0f, 1.0f),          4.0f,  attrs ("", 0)));
    l.add (std::make_unique<P> (ID { "cutRandom", 1 },   "Cut Chaos",    R (0.0f, 1.0f),                 0.0f,  pctAttrs()));
    l.add (std::make_unique<P> (ID { "cutSpan", 1 },     "Cut Span",     R (0.0f, 1.0f),                 0.5f,  pctAttrs()));

    return l;
}

//==============================================================================
MontageProcessor::MontageProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "MontageState", createParameterLayout())
{
    formatManager.registerBasicFormats();

    pPosition    = apvts.getRawParameterValue ("position");
    pScatter     = apvts.getRawParameterValue ("scatter");
    pGrainSize   = apvts.getRawParameterValue ("grainSize");
    pDensity     = apvts.getRawParameterValue ("density");
    pPitch       = apvts.getRawParameterValue ("pitch");
    pPitchSpread = apvts.getRawParameterValue ("pitchSpread");
    pReverse     = apvts.getRawParameterValue ("reverse");
    pPan         = apvts.getRawParameterValue ("pan");
    pAttack      = apvts.getRawParameterValue ("attack");
    pRelease     = apvts.getRawParameterValue ("release");
    pVolume      = apvts.getRawParameterValue ("volume");
    pCutoff      = apvts.getRawParameterValue ("cutoff");
    pResonance   = apvts.getRawParameterValue ("resonance");
    pCutRate     = apvts.getRawParameterValue ("cutRate");
    pCutSteps    = apvts.getRawParameterValue ("cutSteps");
    pCutRandom   = apvts.getRawParameterValue ("cutRandom");
    pCutSpan     = apvts.getRawParameterValue ("cutSpan");
}

bool MontageProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    return layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void MontageProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec { sampleRate, (juce::uint32) samplesPerBlock, 2 };
    filter.prepare (spec);
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    filter.reset();

    gainSmoothed.reset (sampleRate, 0.02);
    gainSmoothed.setCurrentAndTargetValue (juce::Decibels::decibelsToGain (pVolume->load()));

    for (auto& v : voices)
        v.kill();

    cutAccum = 0.0;
    cutStep = 0;
    cutOffset = 0.0f;
}

//==============================================================================
// Programmi (preset)
int MontageProcessor::getNumPrograms() { return numPresets; }

const juce::String MontageProcessor::getProgramName (int index)
{
    if (index >= 0 && index < numPresets)
        return juce::String (index + 1).paddedLeft ('0', 3) + "  " + presets[index].name;
    return {};
}

void MontageProcessor::setParamPlain (const char* id, float plainValue)
{
    if (auto* p = apvts.getParameter (id))
        p->setValueNotifyingHost (p->convertTo0to1 (plainValue));
}

void MontageProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= numPresets)
        return;

    currentProgram = index;
    const auto& p = presets[index];

    setParamPlain ("position", p.position);
    setParamPlain ("scatter", p.scatter);
    setParamPlain ("grainSize", p.grainMs);
    setParamPlain ("density", p.density);
    setParamPlain ("pitch", p.pitch);
    setParamPlain ("pitchSpread", p.pitchSpread);
    setParamPlain ("reverse", p.reverse);
    setParamPlain ("pan", p.pan);
    setParamPlain ("attack", p.attack);
    setParamPlain ("release", p.release);
    setParamPlain ("volume", p.volume);
    setParamPlain ("cutoff", p.cutoff);
    setParamPlain ("resonance", p.resonance);

    // CUTS: attivi solo in alcune famiglie (Glitch, Rhythmic, Experimental)
    float cutRate = 0.0f, cutRandom = 0.0f, cutSpan = 0.5f;
    int cutSteps = 4;

    if (index >= 10 && index < 20)       { cutRate = 4.0f + (float) (index % 10);         cutSteps = 4 + (index % 4) * 2; cutRandom = 0.3f; cutSpan = 0.5f; }
    else if (index >= 50 && index < 60)  { cutRate = 2.0f + 0.5f * (float) (index % 10);  cutSteps = 8;                    cutRandom = 0.0f; cutSpan = 0.8f; }
    else if (index >= 90)                { cutRate = 1.0f + (float) (index % 10);         cutSteps = 2 + (index % 7) * 2; cutRandom = 0.7f; cutSpan = 1.0f; }

    setParamPlain ("cutRate", cutRate);
    setParamPlain ("cutSteps", (float) cutSteps);
    setParamPlain ("cutRandom", cutRandom);
    setParamPlain ("cutSpan", cutSpan);
}

//==============================================================================
// Varianti del suono
void MontageProcessor::mutate()
{
    juce::Random rnd;

    static const float pitches[] = { -12.f, -7.f, -5.f, 0.f, 0.f, 0.f, 3.f, 5.f, 7.f, 12.f };

    setParamPlain ("position", rnd.nextFloat());
    setParamPlain ("scatter", rnd.nextFloat() * 0.7f);
    setParamPlain ("grainSize", 10.0f + std::pow (rnd.nextFloat(), 1.5f) * 300.0f);
    setParamPlain ("density", 3.0f + rnd.nextFloat() * 50.0f);
    setParamPlain ("pitch", pitches[rnd.nextInt (10)]);
    setParamPlain ("pitchSpread", rnd.nextFloat() * rnd.nextFloat() * 12.0f);
    setParamPlain ("reverse", rnd.nextFloat() * 0.6f);
    setParamPlain ("pan", rnd.nextFloat());

    setParamPlain ("cutRate", rnd.nextBool() ? 0.0f : 1.0f + rnd.nextFloat() * 12.0f);
    setParamPlain ("cutSteps", (float) (2 + rnd.nextInt (15)));
    setParamPlain ("cutRandom", rnd.nextFloat() * 0.8f);
    setParamPlain ("cutSpan", 0.2f + rnd.nextFloat() * 0.8f);
}

void MontageProcessor::variation()
{
    juce::Random rnd;

    static const char* ids[] = { "position", "scatter", "grainSize", "density",
                                 "pitchSpread", "reverse", "pan", "cutRandom", "cutSpan" };

    for (auto* id : ids)
    {
        if (auto* p = apvts.getParameter (id))
        {
            const float v = juce::jlimit (0.0f, 1.0f, p->getValue() + (rnd.nextFloat() - 0.5f) * 0.2f);
            p->setValueNotifyingHost (v);
        }
    }
}

//==============================================================================
// Caricamento sample
bool MontageProcessor::loadSample (const juce::File& file)
{
    std::unique_ptr<juce::AudioFormatReader> reader (formatManager.createReaderFor (file));

    if (reader == nullptr)
        return false;

    // massimo 60 secondi
    const juce::int64 maxLen = (juce::int64) (reader->sampleRate * 60.0);
    const int len = (int) juce::jmin (reader->lengthInSamples, maxLen);

    if (len < 64)
        return false;

    auto data = std::make_shared<SampleData>();
    data->buffer.setSize (juce::jmax (1, juce::jmin ((int) reader->numChannels, 2)), len);
    data->buffer.clear();
    data->sampleRate = reader->sampleRate;
    data->name = file.getFileName();

    reader->read (&data->buffer, 0, len, 0, true, true);

    std::shared_ptr<SampleData> old;
    {
        const juce::SpinLock::ScopedLockType sl (sampleLock);
        old = sampleData;
        sampleData = data;
    }

    apvts.state.setProperty ("samplePath", file.getFullPathName(), nullptr);
    ++sampleVersion;
    return true;
}

std::shared_ptr<SampleData> MontageProcessor::getSample()
{
    const juce::SpinLock::ScopedLockType sl (sampleLock);
    return sampleData;
}

//==============================================================================
// MIDI
void MontageProcessor::noteOn (int note, float velocity)
{
    Voice* v = nullptr;

    for (auto& x : voices)
        if (! x.active) { v = &x; break; }

    if (v == nullptr)
        for (auto& x : voices)
            if (x.releasing) { v = &x; break; }

    if (v == nullptr)
    {
        v = &voices[(size_t) (stealIndex % maxVoices)];
        ++stealIndex;
    }

    v->start (note, velocity);
}

void MontageProcessor::noteOff (int note)
{
    for (auto& v : voices)
        if (v.active && ! v.releasing && v.note == note)
            v.releasing = true;
}

void MontageProcessor::handleMidi (const juce::MidiMessage& m)
{
    if (m.isNoteOn())
        noteOn (m.getNoteNumber(), m.getFloatVelocity());
    else if (m.isNoteOff())
        noteOff (m.getNoteNumber());
    else if (m.isAllNotesOff())
    {
        for (auto& v : voices)
            if (v.active)
                v.releasing = true;
    }
    else if (m.isAllSoundOff())
    {
        for (auto& v : voices)
            v.kill();
    }
}

//==============================================================================
// Motore granulare
float MontageProcessor::readSample (const SampleData& s, int channel, double pos)
{
    const int n = s.buffer.getNumSamples();

    pos = std::fmod (pos, (double) n);
    if (pos < 0.0)
        pos += (double) n;

    int i0 = (int) pos;
    if (i0 >= n) i0 = n - 1;
    int i1 = i0 + 1;
    if (i1 >= n) i1 = 0;

    const float frac = (float) (pos - (double) i0);
    const float* d = s.buffer.getReadPointer (juce::jmin (channel, s.buffer.getNumChannels() - 1));

    return d[i0] + frac * (d[i1] - d[i0]);
}

void MontageProcessor::spawnGrain (Voice& v, const SampleData& s, int grainLen, double srcRatio)
{
    for (auto& g : v.grains)
    {
        if (g.active)
            continue;

        const double n = (double) s.buffer.getNumSamples();
        const float jitter = random.nextFloat() * 2.0f - 1.0f;

        g.pos = ((double) (params.position + cutOffset) + (double) (jitter * params.scatter * 0.5f)) * n;

        const float semis = (float) (v.note - 60) + params.pitch
                          + (random.nextFloat() * 2.0f - 1.0f) * params.pitchSpread;
        const double ratio = std::pow (2.0, (double) semis / 12.0) * srcRatio;
        const bool reversed = random.nextFloat() < params.reverseProb;

        g.inc = reversed ? -ratio : ratio;
        g.age = 0;
        g.length = grainLen;

        const float pan = (random.nextFloat() * 2.0f - 1.0f) * params.panSpread; // -1..1
        const float angle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
        g.gainL = std::cos (angle) * 1.41421356f;
        g.gainR = std::sin (angle) * 1.41421356f;

        double p01 = std::fmod (g.pos / n, 1.0);
        if (p01 < 0.0)
            p01 += 1.0;
        pushGrainEvent ((float) p01, pan, reversed);

        g.active = true;
        return;
    }
}

void MontageProcessor::updateParams()
{
    params.position    = pPosition->load();
    params.scatter     = pScatter->load();
    params.grainMs     = pGrainSize->load();
    params.density     = pDensity->load();
    params.pitch       = pPitch->load();
    params.pitchSpread = pPitchSpread->load();
    params.reverseProb = pReverse->load();
    params.panSpread   = pPan->load();
    params.attackMs    = pAttack->load();
    params.releaseMs   = pRelease->load();
    params.cutoff      = pCutoff->load();
    params.resonance   = pResonance->load();
    params.cutRate     = pCutRate->load();
    params.cutSteps    = pCutSteps->load();
    params.cutRandom   = pCutRandom->load();
    params.cutSpan     = pCutSpan->load();

    gainSmoothed.setTargetValue (juce::Decibels::decibelsToGain (pVolume->load()));
}

void MontageProcessor::render (juce::AudioBuffer<float>& buffer, int start, int end, const SampleData& sample)
{
    float* outL = buffer.getWritePointer (0);
    float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : nullptr;

    const double hostSR = currentSampleRate;
    const double srcRatio = sample.sampleRate / hostSR;
    const int grainLen = juce::jmax (32, (int) (params.grainMs * 0.001 * hostSR));
    const double interval = hostSR / (double) juce::jmax (1.0f, params.density);
    const float overlap = juce::jmax (1.0f, params.density * params.grainMs * 0.001f);
    const float norm = 1.0f / std::sqrt (overlap);
    const float attackInc = 1.0f / juce::jmax (1.0f, params.attackMs * 0.001f * (float) hostSR);
    const float releaseDec = 1.0f / juce::jmax (1.0f, params.releaseMs * 0.001f * (float) hostSR);

    filter.setCutoffFrequency (juce::jlimit (20.0f, (float) (hostSR * 0.45), params.cutoff));
    filter.setResonance (params.resonance);

    const double cutInterval = hostSR / (double) juce::jmax (0.05f, params.cutRate);

    for (int i = start; i < end; ++i)
    {
        float l = 0.0f, r = 0.0f;

        if (params.cutRate > 0.05f)
        {
            cutAccum += 1.0;
            if (cutAccum >= cutInterval)
            {
                cutAccum -= cutInterval;
                if (cutAccum > cutInterval)
                    cutAccum = 0.0;
                advanceCut();
            }
        }
        else if (cutOffset != 0.0f || cutStep != 0)
        {
            cutOffset = 0.0f;
            cutStep = 0;
            cutAccum = 0.0;
            cutStepIndex.store (0, std::memory_order_relaxed);
        }

        for (auto& v : voices)
        {
            if (! v.active)
                continue;

            if (v.releasing)
            {
                v.env -= releaseDec;
                if (v.env <= 0.0f)
                {
                    v.kill();
                    continue;
                }
            }
            else if (v.env < 1.0f)
            {
                v.env = juce::jmin (1.0f, v.env + attackInc);
            }

            v.spawnAccum += 1.0;
            if (v.spawnAccum >= interval)
            {
                v.spawnAccum -= interval;
                if (v.spawnAccum > interval)
                    v.spawnAccum = 0.0;
                spawnGrain (v, sample, grainLen, srcRatio);
            }

            float vl = 0.0f, vr = 0.0f;

            for (auto& g : v.grains)
            {
                if (! g.active)
                    continue;

                const float t = (float) g.age / (float) g.length;
                const float w = 0.5f * (1.0f - std::cos (juce::MathConstants<float>::twoPi * t));

                const float sl = readSample (sample, 0, g.pos);
                const float sr = readSample (sample, 1, g.pos);

                vl += sl * w * g.gainL;
                vr += sr * w * g.gainR;

                g.pos += g.inc;
                if (++g.age >= g.length)
                    g.active = false;
            }

            const float e = v.env * (0.25f + 0.75f * v.velocity);
            l += vl * e;
            r += vr * e;
        }

        const float gain = gainSmoothed.getNextValue() * norm;
        l = filter.processSample (0, l * gain);
        r = filter.processSample (1, r * gain);

        outL[i] = l;
        if (outR != nullptr)
            outR[i] = r;
    }
}

void MontageProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;

    buffer.clear();
    updateParams();

    const int numSamples = buffer.getNumSamples();

    const juce::SpinLock::ScopedTryLockType lock (sampleLock);
    const SampleData* sample = lock.isLocked() ? sampleData.get() : nullptr;

    int cursor = 0;

    for (const auto metadata : midi)
    {
        const int pos = juce::jlimit (0, numSamples, metadata.samplePosition);

        if (pos > cursor)
        {
            if (sample != nullptr)
                render (buffer, cursor, pos, *sample);
            cursor = pos;
        }

        handleMidi (metadata.getMessage());
    }

    if (cursor < numSamples && sample != nullptr)
        render (buffer, cursor, numSamples, *sample);

    midi.clear();

    int count = 0;
    for (auto& v : voices)
        if (v.active)
            ++count;
    activeVoices.store (count, std::memory_order_relaxed);
}

//==============================================================================
void MontageProcessor::advanceCut()
{
    const int steps = juce::jmax (2, juce::roundToInt (params.cutSteps));

    if (random.nextFloat() < params.cutRandom)
        cutStep = random.nextInt (steps);
    else
        cutStep = (cutStep + 1) % steps;

    cutOffset = (float) cutStep / (float) steps * params.cutSpan;
    cutStepIndex.store (cutStep, std::memory_order_relaxed);
}

void MontageProcessor::pushGrainEvent (float pos, float pan, bool reversed)
{
    const uint32_t w = evWrite.load (std::memory_order_relaxed);
    const auto i = (size_t) (w % (uint32_t) grainEventRing);

    evPos[i].store (pos, std::memory_order_relaxed);
    evPan[i].store (pan, std::memory_order_relaxed);
    evRev[i].store (reversed ? 1.0f : 0.0f, std::memory_order_relaxed);
    evWrite.store (w + 1, std::memory_order_release);
}

juce::AudioProcessorEditor* MontageProcessor::createEditor()
{
    return new MontageEditor (*this);
}

void MontageProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void MontageProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*xml));

        const auto path = apvts.state.getProperty ("samplePath").toString();

        if (path.isNotEmpty())
        {
            juce::File f (path);
            if (f.existsAsFile())
                loadSample (f);
        }
    }
}

//==============================================================================
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MontageProcessor();
}
