/*
  ==============================================================================

    DistortionPreview.cpp
    Created: 30 Oct 2017 5:22:47pm
    Author:  Jonathon Racz

  ==============================================================================
*/

#include "DistortionPreview.h"
#include "GMLookAndFeel.h"

DistortionPreview::DistortionPreview(DistortionChain::Parameters::Ptr params, ChangeBroadcaster& _paramChangeSource) :
    paramChangeSource(_paramChangeSource),
    freshSignal(1, generatorFrequency),
    processedSignal(1, generatorFrequency)
{
    setBufferedToImage(true);
    // Generate a clean signal.
    dsp::Oscillator<float> signalGenerator(&std::sinf);
    dsp::ProcessSpec spec;
    spec.sampleRate = static_cast<uint32>(generatorFrequency * 2.0f);
    spec.maximumBlockSize = static_cast<uint32>(generatorFrequency);
    spec.numChannels = 1;
    distortion.params = params;
    distortion.prepare(spec);

    float* freshSignalWrite = freshSignal.getWritePointer(0);
    for (size_t i = 0; i < freshSignal.getNumSamples(); ++i)
    {
        freshSignalWrite[i] = std::sin((2.0f * MathConstants<float>::pi) * (i / static_cast<float>(freshSignal.getNumSamples())));
    }

    performDistortion();
    paramChangeSource.addChangeListener(this);
}

void DistortionPreview::performDistortion()
{
    processedSignal.makeCopyOf(freshSignal);
    dsp::AudioBlock<float> block(processedSignal);
    dsp::ProcessContextReplacing<float> context(block);
    distortion.process(context);
}

void DistortionPreview::paint(Graphics& g)
{
    GMLookAndFeel& lf = static_cast<GMLookAndFeel&>(getLookAndFeel());

    // Center line
    {
        g.setColour(findColour(GMLookAndFeel::ColourIds::outlineLight));
        g.fillRect(Rectangle<int>(0,
                                  (getHeight() / 2) - lf.borderThickness / 2,
                                  getWidth(),
                                  lf.borderThickness));
    }

    g.setColour(findColour(GMLookAndFeel::ColourIds::outline));

    // Outer border
    {
        Path outline;
        outline.addRectangle(getLocalBounds());
        g.strokePath(outline, PathStrokeType(lf.borderThickness));
    }

    // Run distortion DSP
    performDistortion();

    // Draw preview wave
    {
        Path wavePath;
        wavePath.preallocateSpace(processedSignal.getNumSamples());
        float pathXDelta = getWidth() / static_cast<float>(processedSignal.getNumSamples());
        for (int channel = 0; channel < processedSignal.getNumChannels(); channel++)
        {
            const float* sample = processedSignal.getReadPointer(channel);
            for (int i = 0; i < processedSignal.getNumSamples(); ++i)
            {
                float yPos = (sample[i] * getHeight() * 0.45f) + (getHeight() / 2.0f);
                if (i == 0)
                    wavePath.startNewSubPath(0, yPos);
                else
                    wavePath.lineTo(static_cast<float>(i) * pathXDelta, yPos);
            }
        }
        g.strokePath(wavePath, PathStrokeType(lf.borderThickness* 2.0f));
    }
}

void DistortionPreview::changeListenerCallback(ChangeBroadcaster* source)
{
    repaint();
}
