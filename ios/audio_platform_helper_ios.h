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
  virtual bool PlayBgm(const std::string& resource, float volume);
  virtual bool StopBgm();
  virtual bool SetBgmVolume(float volume);
  virtual bool SetBgmRate(float rate);

private:
  UInt32          is_ipod_playing_;

#ifdef __OBJC__
  AVAudioPlayer*  bgm_player_;
#else
  void*           dummy_;
#endif
};

} // namespace hikaru

#endif // HIKARU_AUDIO_PLATFORM_HELPER_IOS_H
