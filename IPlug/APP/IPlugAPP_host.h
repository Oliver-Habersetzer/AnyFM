/*
 ==============================================================================
 
 This file is part of the iPlug 2 library
 
 Oli Larkin et al. 2018 - https://www.olilarkin.co.uk
 
 iPlug 2 is an open source library subject to commercial or open-source
 licensing.
 
 The code included in this file is provided under the terms of the WDL license
 - https://www.cockos.com/wdl/
 
 ==============================================================================
 */

#pragma once

/**
 
 IPlug plug-in -> Standalone app wrapper, using Cockos' SWELL
 
 Oli Larkin 2014-2018
 
 Notes:
 
 App settings are stored in a .ini (text) file. The location is as follows:
 
 Windows7: C:\Users\USERNAME\AppData\Local\BUNDLE_NAME\settings.ini
 Windows XP/Vista: C:\Documents and Settings\USERNAME\Local Settings\Application Data\BUNDLE_NAME\settings.ini
 macOS: /Users/USERNAME/Library/Application\ Support/BUNDLE_NAME/settings.ini
 OR
 /Users/USERNAME/Library/Containers/BUNDLE_ID/Data/Library/Application Support/BUNDLE_NAME/settings.ini
 
 */

#include <cstdlib>
#include <string>
#include <vector>
#include <limits>

#include "RtAudio.h"
#include "RtMidi.h"

#include "wdltypes.h"
#include "wdlstring.h"

#include "IPlugPlatform.h"
#include "IPlugConstants.h"

#include "IPlugAPP.h"

#include "config.h"

#ifdef OS_WIN
  #include <WindowsX.h>
  #include <commctrl.h>
  #include <shlobj.h>
  #define DEFAULT_INPUT_DEV "Default Device"
  #define DEFAULT_OUTPUT_DEV "Default Device"
#elif defined(OS_MAC)
  #include "swell.h"
  #define SLEEP( milliseconds ) usleep( (unsigned long) (milliseconds * 1000.0) )
  #define DEFAULT_INPUT_DEV "Built-in Input"
  #define DEFAULT_OUTPUT_DEV "Built-in Output"
#elif defined(OS_LINUX)
  #include "swell.h"
#endif

const int kNumBufferSizeOptions = 11;
const std::string kBufferSizeOptions[kNumBufferSizeOptions] = {"32", "64", "96", "128", "192", "256", "512", "1024", "2048", "4096", "8192" };
const int kDeviceDS = 0; const int kDeviceCoreAudio = 0; const int kDeviceAlsa = 0;
const int kDeviceASIO = 0; const int kDeviceJack = 1;
extern HWND gHWND;
extern HINSTANCE gHINSTANCE;
extern UINT gSCROLLMSG;
extern WDL_DLGRET MainDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

class IPlugAPP;

class IPlugAPPHost
{
public:
  struct AppState
  {
    WDL_String mAudioInDev = WDL_String(DEFAULT_INPUT_DEV);
    WDL_String mAudioOutDev = WDL_String(DEFAULT_OUTPUT_DEV);
    WDL_String mMidiInDev = WDL_String("off");
    WDL_String mMidiOutDev = WDL_String("off");
    uint32_t mAudioDriverType = 0;
    uint32_t mAudioSR = 44100;
    uint32_t mBufferSize = 512;
    uint32_t mMidiInChan = 0;
    uint32_t mMidiOutChan = 0;
    
    AppState()
    : mAudioInDev(DEFAULT_INPUT_DEV)
    , mAudioOutDev(DEFAULT_OUTPUT_DEV)
    , mMidiInDev("off")
    , mMidiOutDev("off")
    , mAudioDriverType(0) // DirectSound / CoreAudio by default
    , mBufferSize(512)
    , mAudioSR(44100)
    , mMidiInChan(0)
    , mMidiOutChan(0)
    {
    }
    
    AppState (const AppState& obj)
    : mAudioInDev(obj.mAudioInDev.Get())
    , mAudioOutDev(obj.mAudioOutDev.Get())
    , mMidiInDev(obj.mMidiInDev.Get())
    , mMidiOutDev(obj.mMidiOutDev.Get())
    , mAudioDriverType(obj.mAudioDriverType)
    , mBufferSize(obj.mBufferSize)
    , mAudioSR(obj.mAudioSR)
    , mMidiInChan(obj.mMidiInChan)
    , mMidiOutChan(obj.mMidiOutChan)
    {
    }
    
    bool operator==(const AppState& rhs) { return (rhs.mAudioDriverType == mAudioDriverType &&
                                                   rhs.mBufferSize == mBufferSize &&
                                                   rhs.mAudioSR == mAudioSR &&
                                                   rhs.mMidiInChan == mMidiInChan &&
                                                   rhs.mMidiOutChan == mMidiOutChan &&
                                                   (strcmp(rhs.mAudioInDev.Get(), mAudioInDev.Get()) == 0) &&
                                                   (strcmp(rhs.mAudioOutDev.Get(), mAudioOutDev.Get()) == 0) &&
                                                   (strcmp(rhs.mMidiInDev.Get(), mMidiInDev.Get()) == 0) &&
                                                   (strcmp(rhs.mMidiOutDev.Get(), mMidiOutDev.Get()) == 0));
    }
    bool operator!=(const AppState& rhs) { return !operator==(rhs); }
  };
  
  static IPlugAPPHost* Create();
  static IPlugAPPHost* sInstance;
  
  void PopulateSampleRateList(HWND hwndDlg, RtAudio::DeviceInfo* pInputDevInfo, RtAudio::DeviceInfo* pOutputDevInfo);
  void PopulateAudioInputList(HWND hwndDlg, RtAudio::DeviceInfo* pInfo);
  void PopulateAudioOutputList(HWND hwndDlg, RtAudio::DeviceInfo* pInfo);
  void PopulateDriverSpecificControls(HWND hwndDlg);
  void PopulateAudioDialogs(HWND hwndDlg);
  bool PopulateMidiDialogs(HWND hwndDlg);
  void PopulatePreferencesDialog(HWND hwndDlg);
  
  IPlugAPPHost();
  ~IPlugAPPHost();
  
  bool OpenWindow();

  bool Init();
  bool InitState();
  void UpdateINI();
  
  /** Returns the name of the audio device at idx
   * @param idx The index RTAudio has given the audio device
   * @return The device name. Core Audio device names are truncated. */
  std::string GetAudioDeviceName(int idx) const;
  // returns the rtaudio device ID, based on the (truncated) device name
  
  /** Returns the audio device index linked to a particular name
  * @param name The name of the audio device to test
  * @return The integer index RTAudio has given the audio device */
  int GetAudioDeviceIdx(const char* name) const;
  
  /** @param direction Either kInput or kOutput
   * @param name The name of the midi device
   * @return An integer specifying the output port number, where 0 means any */
  int GetMIDIPortNumber(ERoute direction, const char* name) const;
  
  /** find out which devices have input channels & which have output channels, add their ids to the lists */
  void ProbeAudioIO();
  void ProbeMidiIO();
  bool InitMidi();
  bool InitAudio(uint32_t inId, uint32_t outId, uint32_t sr, uint32_t iovs);
  bool AudioSettingsInStateAreEqual(AppState& os, AppState& ns);
  bool MIDISettingsInStateAreEqual(AppState& os, AppState& ns);

  bool TryToChangeAudioDriverType();
  bool TryToChangeAudio();
  bool SelectMIDIDevice(ERoute direction, const char* portName);
  
  static int AudioCallback(void* pOutputBuffer, void* pInputBuffer, uint32_t nFrames, double streamTime, RtAudioStreamStatus status, void* pUserData);
  static void MIDICallback(double deltatime, std::vector<uint8_t>* pMsg, void* pUserData);
  static void ErrorCallback(RtAudioError::Type type, const std::string& errorText);

  static WDL_DLGRET PreferencesDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
  IPlugAPP* mIPlug = nullptr;
  RtAudio* mDAC = nullptr;
  RtMidiIn* mMidiIn = nullptr;
  RtMidiOut* mMidiOut = nullptr;
  int mMidiOutChannel = -1;
  int mMidiInChannel = -1;

  IPlugQueue<IMidiMsg> mMidiMsgsFromCallback {32};
  
  /**  */
  AppState mState;
  /** When the preferences dialog is opened the existing state is cached here, and restored if cancel is pressed */
  AppState mTempState;
  /** When the audio driver is started the current state is copied here so that if OK is pressed after APPLY nothing is changed */
  AppState mActiveState;
  
  double mFadeMult = 0.; // Fade multiplier
  double mSampleRate = 44100.;
  uint32_t mSamplesElapsed = 0;
  uint32_t mVecElapsed = 0;
  uint32_t mBufferSize = 512;
  uint32_t mBufIndex; // index for signal vector, loops from 0 to mSigVS
  
  /** The index of the operating systems default input device, -1 if not detected */
  int32_t mDefaultInputDev = -1;
  /** The index of the operating systems default output device, -1 if not detected */
  int32_t mDefaultOutputDev = -1;
    
  WDL_String mINIPath;
  
  //TODO: replace with std::map or WDL something
  std::vector<uint32_t> mAudioInputDevs;
  std::vector<uint32_t> mAudioOutputDevs;
  std::vector<std::string> mAudioIDDevNames;
  std::vector<std::string> mMidiInputDevNames;
  std::vector<std::string> mMidiOutputDevNames;
  
  friend class IPlugAPP;
};
