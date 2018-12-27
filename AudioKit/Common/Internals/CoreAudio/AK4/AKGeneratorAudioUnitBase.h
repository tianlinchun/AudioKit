//
//  AKGeneratorAudioUnitBase.h
//  AudioKit
//
//  Created by Andrew Voelkel, revision history on GitHub.
//  Copyright © 2018 AudioKit. All rights reserved.
//

#pragma once

#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <AudioToolbox/AudioToolbox.h>
#import "AKDSPBase.hpp"
#import "BufferedAudioUnit.h"
#import "AKAudioUnitBase.h"

@interface AKGeneratorAudioUnitBase : AKAudioUnitBase

// Common for oscillators
- (void)setupWaveform:(int)size;
- (void)setWaveformValue:(float)value atIndex:(UInt32)index;

// Multiple waveform effects
- (void)setupIndividualWaveform:(UInt32)waveform size:(int)size;
- (void)setIndividualWaveform:(UInt32)waveform withValue:(float)value atIndex:(UInt32)index;

// STK Methods
- (void)trigger;
- (void)triggerFrequency:(float)frequency amplitude:(float)amplitude;

@end



