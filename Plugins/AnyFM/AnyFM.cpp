#include "AnyFM.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

AnyFM::AnyFM(const InstanceInfo& info) : Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kGain)->InitDouble("Gain", 1, -2, 2, 1, "%");
  this->pGain = GetParam(kGain);

  GetParam(kWidth)->InitDouble("Width", 3, 0, ANY_FX_MAX_WIDTH, 0.01, "smp");
  this->pWidth = GetParam(kWidth);

  GetParam(kInterpolationMode)->InitEnum("Interpolation", kLinear, InterpolationModeLabels);
  this->pInterpolationMode = GetParam(kInterpolationMode);

  GetParam(kModulatorMix)->InitPercentage("Modulator Mix", 0, 0, 1);
  this->pModulatorMix = GetParam(kModulatorMix);

  GetParam(kModulatorMixShift)->InitPercentage("Modulator Mix Shift", 0, -1, 1);
  this->pModulatorMixShift = GetParam(kModulatorMixShift);

  // init buffers
  for (int s = 0; s < ANY_FX_BUFFER_SIZE; s++) {
    for (int c = 0; c < 2; c++) {
      inputBuffers[c].push_back(0);
      modulatorBuffers[c].push_back(0);
    }
  }
}

#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)
#define CLAMP(x, upper, lower) (MIN(upper, MAX(x, lower)))
#define CLAMP0(x, upper) (MIN(upper, MAX(x, 0)))
#define CLAMP1ABS(x) (MIN(1, MAX(x, -1)))

// #define TEST

#if IPLUG_DSP
void AnyFM::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = pGain->Value();
  const double width = pWidth->Value();
  const int interpolationMode = (int)pInterpolationMode->Value();
  const double modulatorMix = pModulatorMix->Value();
  const double modulatorMixShift = pModulatorMixShift->Value();

  const int nInChannels = NInChansConnected();
  const int nOutChannels = NOutChansConnected();

  if (nOutChannels == 2 && nInChannels == 4)
  {
    for (int c = 0; c < 2; c++)
    {
      for (int s = 0; s < nFrames; s++)
      {
        int globalOffset;
        sample iOffset, modulator, clamped, offset, weight;

        modulator = inputs[2 + c][s];
        clamped = CLAMP1ABS(modulator * gain);
        offset = ANY_FX_0_OFFSET - clamped * width / 2;
        weight = modf(offset, &iOffset);

        sample samples[4];
        for (int i = 0; i < 4; i++) {
          globalOffset = s - i - iOffset;
          samples[i]
            = globalOffset < 0
            ? inputBuffers[c][globalOffset + ANY_FX_BUFFER_SIZE]
            : inputs[c][globalOffset];
        }

        sample smpPrev = samples[0];
        sample smpA = samples[1];
        sample smpB = samples[2];
        sample smpNext = samples[3];
        sample result;

        // sauce: http://paulbourke.net/miscellaneous/interpolation/
        switch (interpolationMode)
        {
        case kNearestNeighbour: {
          result = weight < .5 ? smpA : smpB;
          break;
        }

        default:
        case kLinear: {
          result = smpA * (1 - weight) + smpB * weight;
          break;
        }

        case kCosine: {
          auto mu2 = (1 - cos(weight * PI)) / 2;
          result = smpA * (1 - mu2) + smpB * mu2;
          break;
        }

        case kCubic: {
          /*
          double a0,a1,a2,a3,mu2;
          mu2 = mu*mu;
          a0 = y3 - y2 - y0 + y1;
          a1 = y0 - y1 - a0;
          a2 = y2 - y0;
          a3 = y1;
          return(a0*mu*mu2+a1*mu2+a2*mu+a3);
          */
          auto mu2 = weight * weight;
          auto a0 = smpNext - smpB - smpPrev + smpA;
          auto a1 = smpPrev - smpA - a0;
          auto a2 = smpB - smpPrev;
          result = a0 * weight * mu2 + a1 * mu2 + a2 * weight + smpA;
          break;
        }
        }

        // add modulator
        {
          offset = ANY_FX_0_OFFSET - modulatorMixShift * ANY_FX_0_OFFSET / 2;
          iOffset = round(offset);
          globalOffset = s - iOffset;
          double modulatorSample = globalOffset < 0
            ? modulatorBuffers[c][globalOffset + ANY_FX_BUFFER_SIZE]
            : inputs[c + 2][globalOffset];
          outputs[c][s] += modulatorSample * modulatorMix;
        }

        // save result
        outputs[c][s] = result;
  }
}

    // copy buffers
    for (int c = 0; c < 2; c++) {
      for (int s = 0; s < nFrames; s++) {
        inputBuffers[c].push_back(inputs[c][s]);
        modulatorBuffers[c].push_back(inputs[c + 2][s]);
      }
    }
  }
}
#endif