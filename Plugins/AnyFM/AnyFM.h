#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <boost\circular_buffer.hpp>

#define InterpolationModeLabels { "Nearest neighbour", "Linear", "Cosine", "Cubic" }


using namespace iplug;
using namespace igraphics;
using namespace boost;

typedef circular_buffer<sample> sampleBuffer;

class AnyFM final : public Plugin
{

private:
  IParam* pGain;
  IParam* pWidth;
  IParam* pInterpolationMode;
  IParam* pModulatorMix;
  IParam* pModulatorMixShift;
  sampleBuffer leftInputBuffer = sampleBuffer(ANY_FX_BUFFER_SIZE);
  sampleBuffer rightInputBuffer = sampleBuffer(ANY_FX_BUFFER_SIZE);
  sampleBuffer inputBuffers[2] = { leftInputBuffer, rightInputBuffer };
  sampleBuffer leftModulatorBuffer = sampleBuffer(ANY_FX_BUFFER_SIZE);
  sampleBuffer rightModulatorBuffer = sampleBuffer(ANY_FX_BUFFER_SIZE);
  sampleBuffer modulatorBuffers[2] = { leftModulatorBuffer, rightModulatorBuffer };

public:
  AnyFM(const InstanceInfo& info);
#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif

  enum EParams
  {
    kGain = 0,
    kWidth,
    kInterpolationMode,
    kModulatorMix,
    kModulatorMixShift,
    kNumParams
  };

  enum EInterpolationModes {
    kNearestNeighbour,
    kLinear,
    kCosine,
    kCubic,
    kNumInterpolationModes
  };

  const int kNumPresets = 1;

};
