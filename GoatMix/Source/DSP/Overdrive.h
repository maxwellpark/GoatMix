/*
  ==============================================================================

    Overdrive.h
    Created: 2 Nov 2017 2:21:08pm
    Author:  Jonathon Racz

  ==============================================================================
*/

#pragma once

#include "JuceHeader.h"
#include "Gain.h"

/** Based on the overdrive from the DSPDemo project. */
class Overdrive :
    public dsp::ProcessorBase
{
public:
    struct Parameters :
        dsp::ProcessorState
    {
        float gainUp = 1.0f;
        float gainDown = 1.0f;
        using Ptr = ReferenceCountedObjectPtr<Parameters>;
    };

    Overdrive() = default;
    ~Overdrive() = default;

    void prepare(const dsp::ProcessSpec& spec) override
    {
        auto& waveShaper = overdrive.get<1>();
        waveShaper.functionToUse = std::tanh;

        /*
        auto& dcFilter = overdrive.get<2>();
        dcFilter.state = dsp::IIR::Coefficients<float>::makeHighPass(spec.sampleRate, 5.0f);
         */

        overdrive.prepare(spec);
        updateParameters();
    }

    void process(const dsp::ProcessContextReplacing<float>& context) override
    {
        updateParameters();
        overdrive.process(context);
    }

    void reset() override
    {
        overdrive.reset();
    }

    Parameters::Ptr params = new Parameters;

private:
    void updateParameters()
    {
        auto& gainUp = overdrive.get<0>();
        gainUp.params->gain = params->gainUp;

        auto& gainDown = overdrive.get<2>();
        gainDown.params->gain = params->gainDown;
    }

    dsp::ProcessorChain<
        Gain,
        dsp::WaveShaper<float>,
        /*dsp::ProcessorDuplicator<
            dsp::IIR::Filter<float>,
            dsp::IIR::Coefficients<float>>,*/
        Gain> overdrive;
};
