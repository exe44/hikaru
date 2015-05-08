#ifndef HIKARU_AUDIO_HANDLER_ANDROID_H
#define HIKARU_AUDIO_HANDLER_ANDROID_H

#include "audio_mgr.h"

#include <vector>

namespace hikaru {

class AudioHandlerAndroid : public AudioHandler
{
public:
  AudioHandlerAndroid();
  virtual ~AudioHandlerAndroid();

  virtual void Pause();
  virtual void Resume();
  
  virtual void LoadSound(const std::string& resource);
  virtual void UnloadSound(const std::string& resource);
  
  virtual int PlaySound(const std::string& resource, bool is_loop, float volume, float pitch);
  virtual void StopSound(int sound_id, bool wait_finish);
  virtual void StopAllSounds(bool wait_finish);

  virtual void SetSoundVolume(int sound_id, float volume);
  virtual void SetGlobalSoundVolume(float volume);

  virtual void SetSoundPitch(int sound_id, float pitch);

  virtual void PlayBgm(const std::string& resource, float volume);
  virtual void StopBgm();
  
  virtual void SetBgmVolume(float volume);
  
  virtual bool IsPlayingBgm();

private:
  struct SoundInfo
  {
    SoundInfo() : sound_id(-1), volume(1.f) {}

    int sound_id;
    float volume;
  };

  int bgm_ref_;
  float global_sound_volume_;

  std::vector<SoundInfo> sound_infos_;
};

} // namespace hikaru

#endif // HIKARU_AUDIO_HANDLER_ANDROID_H
