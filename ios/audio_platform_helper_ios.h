#ifndef HIKARU_AUDIO_PLATFORM_HELPER_IOS_H
#define HIKARU_AUDIO_PLATFORM_HELPER_IOS_H

#include "audio_mgr.h"

#ifdef __OBJC__
#include <AVFoundation/AVFoundation.h>
#endif

#include <MacTypes.h>

namespace hikaru {

class AudioPlatformHelperIOS : public AudioPlatformHelper
{
public:
  AudioPlatformHelperIOS(bool need_record);
  virtual ~AudioPlatformHelperIOS();

  virtual bool IsCanPlayBgm();
  virtual bool PrepareBgm(const std::string& resource);
  virtual bool PlayBgm(const std::string& resource, float volume);
  virtual bool PauseBgm();
  virtual bool ResumeBgm();
  virtual bool StopBgm();
  virtual bool SetBgmVolume(float volume);
  virtual bool SetBgmRate(float rate);

private:
  UInt32          is_ipod_playing_;
  
  std::string prepare_bgm_resource_;

#ifdef __OBJC__
  AVAudioPlayer*  bgm_player_;
  AVAudioPlayer*  prepare_bgm_player_;
#else
  void*           dummy_;
  void*           dummy2_;
#endif
};

} // namespace hikaru

#endif // HIKARU_AUDIO_PLATFORM_HELPER_IOS_H
