#ifndef HIKARU_AUDIO_HANDLER_AL_H
#define HIKARU_AUDIO_HANDLER_AL_H

#include "audio_mgr.h"

#ifdef HIKARU_WIN
#include <al.h>
#include <alc.h>
#else
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#endif

#include <string>
#include <vector>
#include <map>

namespace hikaru {

class AudioPlatformHelper;

struct AudioBufferInfo
{
  AudioBufferInfo() : buffer(0), reference_count(0) {}
  
  ALuint  buffer;
  int     reference_count;
};

struct AudioSourceInfo
{
  AudioSourceInfo() : source(0), buffer_ref(-1), volume(1.f) {}
  
  ALuint  source;
  int     buffer_ref;
  float   volume;
};

class AudioHandlerAl : public AudioHandler
{
public:
  AudioHandlerAl(AudioPlatformHelper* platform_helper);
  virtual ~AudioHandlerAl();

  // sound
  
  virtual void LoadSound(const std::string& resource);
  virtual void UnloadSound(const std::string& resource);
  
  virtual int PlaySound(const std::string& resource, bool is_loop, float volume, float pitch);
  virtual void StopSound(int sound_id, bool wait_finish);

  virtual void SetSoundVolume(int sound_id, float volume);
  virtual void SetGlobalSoundVolume(float volume);
  
  virtual void SetSoundPitch(int sound_id, float pitch);

  virtual void SetSoundRefDist(int sound_id, float dist);
  virtual void SetSoundRelativePos(int sound_id, float x, float y, float z);

  // bgm
  
  virtual void PlayBgm(const std::string& resource, float volume);
  virtual void StopBgm();
  
  virtual void SetBgmVolume(float volume);
  virtual void SetBgmRate(float rate);
  
  bool IsPlayingBgm();
  
private:
  void InitAl();
  void ReleaseAl();
  
  int AddBuffer(const std::string& resource);
  void RemoveBuffer(int buffer_ref);
  int AddSource(int buffer_ref, bool is_loop, float volume, float pitch);
  void RemoveSource(int source_ref);
  
  ALCcontext* context_;
  ALCdevice*  device_;
  
  std::vector<AudioBufferInfo>  buffers_;
  std::vector<AudioSourceInfo>  sources_;
  std::map<std::string, int>    buffer_resource_map_;
  
  float global_sound_volume_;
  int	bgm_source_ref_;
  
  AudioPlatformHelper* platform_helper_;
};

} // namespace hikaru

#endif
