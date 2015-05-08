#ifndef HIKARU_AUDIO_MGR_H
#define HIKARU_AUDIO_MGR_H

#if defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#  define HIKARU_WIN
#elif defined(__APPLE_CC__)
#  if __ENVIRONMENT_IPHONE_OS_VERSION_MIN_REQUIRED__ >= 30000 || __IPHONE_OS_VERSION_MIN_REQUIRED >= 30000
#    define HIKARU_IOS
#  else
#    define HIKARU_MAC
#  endif
#elif defined(OS_ANDROID)
#	 define HIKARU_ANDROID
#elif defined(linux) || defined(__linux) || defined(__linux__)
#  define HIKARU_LINUX
#endif

#include <string>

namespace hikaru {

class AudioPlatformHelper
{
public:
  virtual ~AudioPlatformHelper() {}
    
  virtual bool IsCanPlayBgm() { return true; }
  virtual bool PlayBgm(const std::string& resource, float volume) { return false; }
  virtual bool StopBgm() { return false; }
  virtual bool SetBgmVolume(float volume) { return false; }
  virtual bool SetBgmRate(float rate) { return false; }
};

class AudioHandler
{
 public:
  virtual ~AudioHandler() {}

  virtual void Pause() {}
  virtual void Resume() {}

  // sound
  
  virtual void LoadSound(const std::string& resource) = 0;
  virtual void UnloadSound(const std::string& resource) = 0;
  
  virtual int PlaySound(const std::string& resource,
                        bool is_loop,
                        float volume,
                        float pitch) = 0;

  virtual void StopSound(int sound_id, bool wait_finish) = 0;
  virtual void StopAllSounds(bool wait_finish) = 0;
  virtual void SetSoundVolume(int sound_id, float volume) = 0;
  virtual void SetGlobalSoundVolume(float volume) = 0;

  virtual void SetSoundPitch(int sound_id, float pitch) = 0;

  virtual void SetSoundRefDist(int sound_id, float dist) {}
  virtual void SetSoundRelativePos(int sound_id, float x, float y, float z) {}

  // bgm

  virtual void PlayBgm(const std::string& resource, float volume) = 0;
  virtual void StopBgm() = 0;
  
  virtual void SetBgmVolume(float volume) = 0;
  virtual void SetBgmRate(float rate) {}
  
  virtual bool IsPlayingBgm() = 0;
};

class AudioMgr
{
public:
  void Init(bool need_record = false);

  void SetForceReplaceResourceExtension(const std::string& sound_ext,
                                        const std::string& bgm_ext);

  void Update(float delta_time);

  void Pause();
  void Resume();

  // sound
  
  void LoadSound(const std::string& resource);
  void UnloadSound(const std::string& resource);
  
  int PlaySound(const std::string& resource,
                bool is_loop = false,
                float volume = 1.0f,
                float pitch = 1.0f);

  void StopSound(int sound_id, bool wait_finish = false);

  void SetSoundVolume(int sound_id, float volume);
  void SetGlobalSoundVolume(float volume);

  void SetSoundPitch(int sound_id, float pitch);
  void SetSoundRefDist(int sound_id, float dist);
  void SetSoundRelativePos(int sound_id, float x, float y, float z);

  // bgm

  void PlayBgm(const std::string& resource, float fade_out_period = 0.0f);
  void StopBgm(float fade_out_time = 0.0f);

  void SetBgmVolume(float volume, float transition_period = 0.0f);
  void SetBgmRate(float rate);

  bool IsPlayingBgm();
  bool IsCanPlayBgm();

  //

  inline static AudioMgr& Ins()
  {
    if (!ins_ptr_) ins_ptr_ = new AudioMgr;
    return *ins_ptr_;
  }

  inline static void KillIns()
  {
    if (ins_ptr_)
    {
      delete ins_ptr_;
      ins_ptr_ = NULL;
    }
  }

 private:
  AudioMgr();
  ~AudioMgr();
  
  static AudioMgr* ins_ptr_;

  float	bgm_volume_;
  float	bgm_volume_transition_from_, bgm_volume_transition_to_;
  float	bgm_volume_transition_period_, bgm_volume_transition_remain_time_;
  
  float	bgm_fade_out_period_, bgm_fade_out_remain_time_;
  
  std::string	wait_change_bgm_resource_;

  std::string force_sound_ext_, force_bgm_ext_;

  AudioHandler* audio_handler_;
  AudioPlatformHelper* platform_helper_;
};

} // namespace hikaru

#endif // HIKARU_AUDIO_MGR_H
