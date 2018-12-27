//
//  AKAutoPannerDSP.hpp
//  AudioKit
//
//  Created by Aurelius Prochazka, revision history on Github.
//  Copyright © 2018 AudioKit. All rights reserved.
//

#pragma once

#import <AVFoundation/AVFoundation.h>

typedef NS_ENUM(AUParameterAddress, AKAutoPannerParameter) {
    AKAutoPannerParameterFrequency,
    AKAutoPannerParameterDepth,
    AKAutoPannerParameterRampDuration
};

#import "AKLinearParameterRamp.hpp"  // have to put this here to get it included in umbrella header

#ifndef __cplusplus

AKDSPRef createAutoPannerDSP(int nChannels, double sampleRate);

#else

#import "AKSoundpipeDSPBase.hpp"

class AKAutoPannerDSP : public AKSoundpipeDSPBase {

    sp_osc *_trem;
    sp_ftbl *_tbl;
    sp_panst *_panst;
    UInt32 _tbl_size = 4096;

private:
    AKLinearParameterRamp frequencyRamp;
    AKLinearParameterRamp depthRamp;
   
public:
    AKAutoPannerDSP() {
        frequencyRamp.setTarget(10.0, true);
        frequencyRamp.setDurationInSamples(10000);
        depthRamp.setTarget(1.0, true);
        depthRamp.setDurationInSamples(10000);
    }

    /** Uses the ParameterAddress as a key */
    void setParameter(AUParameterAddress address, float value, bool immediate) override {
        switch (address) {
            case AKAutoPannerParameterFrequency:
                frequencyRamp.setTarget(value, immediate);
                break;
            case AKAutoPannerParameterDepth:
                depthRamp.setTarget(value, immediate);
                break;
            case AKAutoPannerParameterRampDuration:
                frequencyRamp.setRampDuration(value, _sampleRate);
                depthRamp.setRampDuration(value, _sampleRate);
                break;
        }
    }

    /** Uses the ParameterAddress as a key */
    float getParameter(AUParameterAddress address) override {
        switch (address) {
            case AKAutoPannerParameterFrequency:
                return frequencyRamp.getTarget();
            case AKAutoPannerParameterDepth:
                return depthRamp.getTarget();
            case AKAutoPannerParameterRampDuration:
                return frequencyRamp.getRampDuration(_sampleRate);
                return depthRamp.getRampDuration(_sampleRate);
        }
        return 0;
    }

    void init(int _channels, double _sampleRate) override {
        AKSoundpipeDSPBase::init(_channels, _sampleRate);
        sp_osc_create(&_trem);
        sp_osc_init(_sp, _trem, _tbl, 0);
        _trem->freq = 10.0;
        _trem->amp = 1.0;
        sp_panst_create(&_panst);
        sp_panst_init(_sp, _panst);
        _panst->pan = 0;
    }

    void setupWaveform(uint32_t size) override {
        _tbl_size = size;
        sp_ftbl_create(_sp, &_tbl, _tbl_size);
    }

    void setWaveformValue(uint32_t index, float value) override {
        _tbl->tbl[index] = value;
    }

    void deinit() override {
        sp_osc_destroy(&_trem);
        sp_panst_destroy(&_panst);
    }

    void process(uint32_t frameCount, uint32_t bufferOffset) override {

        for (int frameIndex = 0; frameIndex < frameCount; ++frameIndex) {
            int frameOffset = int(frameIndex + bufferOffset);

            // do ramping every 8 samples
            if ((frameOffset & 0x7) == 0) {
                frequencyRamp.advanceTo(_now + frameOffset);
                depthRamp.advanceTo(_now + frameOffset);
            }
            _trem->freq = frequencyRamp.getValue();
            _trem->amp = depthRamp.getValue();

            float temp = 0;
            float *tmpin[2];
            float *tmpout[2];
            for (int channel = 0; channel < _nChannels; ++channel) {
                float *in  = (float *)_inBufferListPtr->mBuffers[channel].mData  + frameOffset;
                float *out = (float *)_outBufferListPtr->mBuffers[channel].mData + frameOffset;

                if (channel < 2) {
                    tmpin[channel] = in;
                    tmpout[channel] = out;
                }
                if (!_playing) {
                    *out = *in;
                }
            }
            if (_playing) {
                sp_osc_compute(_sp, _trem, NULL, &temp);
                _panst->pan = 2.0 * temp - 1.0;
                sp_panst_compute(_sp, _panst, tmpin[0], tmpin[1], tmpout[0], tmpout[1]);
            }

        }
    }
};

#endif
