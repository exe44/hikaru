#include "audio_handler_al.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

namespace hikaru {

extern void* GetAlAudioData(const std::string& resource,
                            ALsizei& size,
                            ALenum& format,
                            ALsizei& freq);

AudioHandlerAl::AudioHandlerAl(AudioPlatformHelper* platform_helper)
  : global_sound_volume_(1.f),
  bgm_source_ref_(-1),
  platform_helper_(platform_helper)
{
  assert(platform_helper_);
  
  InitAl();
}

AudioHandlerAl::~AudioHandlerAl()
{
  ReleaseAl();
}

void AudioHandlerAl::LoadSound(const std::string& resource)
{
  std::map<std::string, int>::iterator itor = buffer_resource_map_.find(resource);
  if (itor == buffer_resource_map_.end())
  {
    int buffer_ref = AddBuffer(resource);
    ++buffers_[buffer_ref].reference_count;
  }
}

void AudioHandlerAl::UnloadSound(const std::string& resource)
{
  // remove all referenced source first
  
  std::map<std::string, int>::iterator itor = buffer_resource_map_.find(resource);
  if (itor != buffer_resource_map_.end())
  {
    int buffer_ref = itor->second;
    size_t source_num = sources_.size();
    for (int i = 0; i < source_num; ++i)
    {
      if (sources_[i].buffer_ref == buffer_ref)
        RemoveSource(i);
    }
  }
  
  // if still exist
  
  itor = buffer_resource_map_.find(resource);
  if (itor != buffer_resource_map_.end())
  {
    RemoveBuffer(itor->second);
  }
}

int AudioHandlerAl::PlaySound(const std::string& resource, bool is_loop, float volume, float pitch)
{
  int buffer_ref = -1;
  
  std::map<std::string, int>::iterator itor = buffer_resource_map_.find(resource);
  if (itor != buffer_resource_map_.end())
  {
    buffer_ref = itor->second;
  }
  else
  {
    buffer_ref = AddBuffer(resource);
  }
  
#ifdef AUDIO_LOG
  fprintf(stdout, "PlaySound %s buffer %d\n", resource.c_str(), buffer_ref);
#endif

  return AddSource(buffer_ref, is_loop, volume, pitch);
}

void AudioHandlerAl::StopSound(int sound_id, bool wait_finish)
{
  assert(sound_id >= 0 && sound_id < sources_.size());
  
  if (wait_finish)
  {
    alSourcei(sources_[sound_id].source, AL_LOOPING, AL_FALSE);
    return;
  }
  
  RemoveSource(sound_id);
}
  
void AudioHandlerAl::StopAllSounds(bool wait_finish)
{
  for (int i = 0; i < sources_.size(); ++i)
  {
    if (bgm_source_ref_ != -1 && i == bgm_source_ref_)
      continue;
    
    if (wait_finish)
      alSourcei(sources_[i].source, AL_LOOPING, AL_FALSE);
    else
      RemoveSource(i);
  }
}

void AudioHandlerAl::SetSoundVolume(int sound_id, float volume)
{
  assert(sound_id >= 0 && sound_id < sources_.size());

  sources_[sound_id].volume = volume;
  alSourcef(sources_[sound_id].source, AL_GAIN, global_sound_volume_ * volume);
}

void AudioHandlerAl::SetGlobalSoundVolume(float volume)
{
  global_sound_volume_ = volume;
  
  for (int i = 0; i < sources_.size(); ++i)
  {
    if (bgm_source_ref_ != -1 && i == bgm_source_ref_)
      continue;
    
    alSourcef(sources_[i].source, AL_GAIN, global_sound_volume_ * sources_[i].volume);
  }
}

void AudioHandlerAl::SetSoundPitch(int sound_id, float pitch)
{
  assert(sound_id >= 0 && sound_id < sources_.size());
  
  alSourcef(sources_[sound_id].source, AL_PITCH, pitch);
}

void AudioHandlerAl::SetSoundRefDist(int sound_id, float dist)
{
  assert(sound_id >= 0 && sound_id < sources_.size());
  
  alSourcef(sources_[sound_id].source, AL_REFERENCE_DISTANCE, dist);
}

void AudioHandlerAl::SetSoundRelativePos(int sound_id, float x, float y, float z)
{
  assert(sound_id >= 0 && sound_id < sources_.size());

  float sourcePosAL[] = { x, z, -y };
  alSourcefv(sources_[sound_id].source, AL_POSITION, sourcePosAL);
}

void AudioHandlerAl::PrepareBgm(const std::string& resource)
{
  platform_helper_->PrepareBgm(resource);
}
  
void AudioHandlerAl::PlayBgm(const std::string& resource, float volume)
{
  if (!platform_helper_->IsCanPlayBgm())
    return;
  
  if (bgm_source_ref_ != -1)
    StopBgm();
  
  if (resource.empty())
    return;
  
  if (platform_helper_->PlayBgm(resource, volume))
  {
    bgm_source_ref_ = 0;
  }
  else
  {
    bgm_source_ref_ = PlaySound(resource, true, volume, 1.0f);
    
    if (bgm_source_ref_ != -1)
    {
      sources_[bgm_source_ref_].volume = volume;
      alSourcef(sources_[bgm_source_ref_].source, AL_GAIN, volume);
    }
  }
}

void AudioHandlerAl::StopBgm()
{
  if (bgm_source_ref_ == -1)
    return;
  
  if (!platform_helper_->StopBgm())
  {
    StopSound(bgm_source_ref_, false);
  }
  
  bgm_source_ref_ = -1;
}

void AudioHandlerAl::SetBgmVolume(float volume)
{
  if (bgm_source_ref_ == -1)
    return;
  
  if (!platform_helper_->SetBgmVolume(volume))
  {
    sources_[bgm_source_ref_].volume = volume;
    alSourcef(sources_[bgm_source_ref_].source, AL_GAIN, volume);
  }
}

void AudioHandlerAl::SetBgmRate(float rate)
{
  if (bgm_source_ref_ == -1)
    return;
  
  platform_helper_->SetBgmRate(rate);
}

bool AudioHandlerAl::IsPlayingBgm()
{
  return bgm_source_ref_ != -1;
}

void AudioHandlerAl::InitAl()
{
  // Create a new OpenAL Device
  // Pass NULL to specify the system’s default output device
  device_ = alcOpenDevice(NULL);
  if (device_ != NULL)
  {
    // Create a new OpenAL Context
    // The new context will render to the OpenAL Device just created 
    context_ = alcCreateContext(device_, 0);
    if (context_ != NULL)
    {
      // Make the new context the Current OpenAL Context
      alcMakeContextCurrent(context_);
    }
    
//    float listenerOriAL[] = { 0.f, 0.f, -1.f, 0.f, 1.f, 0.f };
//    alListenerfv(AL_ORIENTATION, listenerOriAL);
  }
}

void AudioHandlerAl::ReleaseAl()
{
  for (int i = 0; i < sources_.size(); ++i)
  {
    alDeleteSources(1, &sources_[i].source);
  }
  sources_.clear();
  
  for (int i = 0; i < buffers_.size(); ++i)
  {
    alDeleteBuffers(1, &buffers_[i].buffer);
  }
  buffers_.clear();
  
  //Release context
  alcDestroyContext(context_);
  
  //Close device
  alcCloseDevice(device_);
}

int AudioHandlerAl::AddBuffer(const std::string& resource)
{
  ALenum error = AL_NO_ERROR;
  
  AudioBufferInfo buffer_info;
  
  // Create some OpenAL Buffer Objects
  
  alGenBuffers(1, &buffer_info.buffer);
  if ((error = alGetError()) != AL_NO_ERROR)
  {
    fprintf(stderr, "Error(%x) generating buffer for %s\n", error, resource.c_str());
    return -1;
  }
  
  ALenum format;
  ALsizei size;
  ALsizei freq;
  
  void* data = GetAlAudioData(resource,
                              size,
                              format,
                              freq);
  
  if (NULL == data)
  {
    fprintf(stderr, "Failed to get audio data for %s\n", resource.c_str());
    return -1;
  }
  
  // use the static buffer data API
  //alBufferDataStaticProc(buffer_info.buffer, format, data, size, freq);
  alBufferData(buffer_info.buffer, format, data, size, freq);
  
  free(data);
  
  if ((error = alGetError()) != AL_NO_ERROR) {
    fprintf(stderr, "Error(%x) attaching audio to buffer for %s\n", error, resource.c_str());
    return -1;
  }		
  
  int i = 0;
  for (; i < buffers_.size(); ++i)
  {
    if (buffers_[i].reference_count == 0)
    {
      if (buffers_[i].buffer != 0)
        RemoveBuffer(i);
      
      buffers_[i] = buffer_info;
      
#ifdef AUDIO_LOG
      fprintf(stdout, "AddBuffer replace buffer %d\n", i);
#endif
      break;
    }
  }
  
  if (i == buffers_.size())
  {
    buffers_.push_back(buffer_info);
    
#ifdef AUDIO_LOG
    fprintf(stdout, "AddBuffer push back buffer %d\n", i);
#endif
  }
  
  buffer_resource_map_.insert(std::make_pair(resource, i));
  
#ifdef AUDIO_LOG
  fprintf(stdout, "AddBuffer to map %s -> %d\n", resource.c_str(), i);
#endif
  
  return i;
}

void AudioHandlerAl::RemoveBuffer(int buffer_ref)
{
  assert(buffer_ref >= 0 && buffer_ref < buffers_.size());
  
  alDeleteBuffers(1, &buffers_[buffer_ref].buffer);
  buffers_[buffer_ref].buffer = 0;
  buffers_[buffer_ref].reference_count = 0;
  
#ifdef AUDIO_LOG
  fprintf(stdout, "RemoveBuffer %d\n", buffer_ref);
#endif
  
  std::map<std::string, int>::iterator itor = buffer_resource_map_.begin();
  for (; itor != buffer_resource_map_.end(); ++itor)
  {
    if (itor->second == buffer_ref)
    {
#ifdef AUDIO_LOG
      fprintf(stdout, "RemoveBuffer from map %s -> %d\n", itor->first.c_str(), buffer_ref);
#endif
      
      buffer_resource_map_.erase(itor);
      
      return;
    }
  }
}

int AudioHandlerAl::AddSource(int buffer_ref, bool is_loop, float volume, float pitch)
{
  assert(buffer_ref >= 0 && buffer_ref < buffers_.size());

  ALenum error = AL_NO_ERROR;
  
  AudioSourceInfo source_info;
  source_info.buffer_ref = buffer_ref;
  
  // Create some OpenAL Source Objects
  alGenSources(1, &source_info.source);
  if ((error = alGetError()) != AL_NO_ERROR) 
  {
    fprintf(stderr, "Error(%x) generating source\n", error);
    return -1;
  }
  
  // Turn Looping ON
  alSourcei(source_info.source, AL_LOOPING, is_loop ? AL_TRUE : AL_FALSE);
  
  // Set Source Position
  float sourcePosAL[] = {0, 0, 0};
  alSourcefv(source_info.source, AL_POSITION, sourcePosAL);
  
  // Set Source Reference Distance
  alSourcef(source_info.source, AL_REFERENCE_DISTANCE, 50.0f);
  
  // Attach OpenAL Buffer to OpenAL Source
  alSourcei(source_info.source, AL_BUFFER, buffers_[buffer_ref].buffer);
  
  // Set Volume
  source_info.volume = volume;
  alSourcef(source_info.source, AL_GAIN, global_sound_volume_ * volume);
  
  // Set Pitch
  alSourcef(source_info.source, AL_PITCH, pitch);
  
  if ((error = alGetError()) != AL_NO_ERROR)
  {
    fprintf(stderr, "Error(%x) setup source\n", error);
    return -1;
  }
  
  // play
  alSourcePlay(source_info.source);
  if ((error = alGetError()) != AL_NO_ERROR)
  {
    fprintf(stderr, "Error(%x) starting source\n", error);
    return -1;
  }
  
  ++buffers_[buffer_ref].reference_count;
  
#ifdef AUDIO_LOG
  fprintf(stdout, "AddSource add ref count of buffer %d: %d\n", buffer_ref, buffers_[buffer_ref].reference_count);
#endif
  
  int i = 0;
  ALint play;
  bool can_insert;
  for (; i < sources_.size(); ++i)
  {
    can_insert = false;
    if (sources_[i].buffer_ref == -1)
    {
      can_insert = true;
    }
    else
    {
      alGetSourcei(sources_[i].source, AL_SOURCE_STATE, &play);
      if (play != AL_PLAYING)
      {
        RemoveSource(i);
        can_insert = true;
      }
    }
    
    if (can_insert)
    {
      sources_[i] = source_info;
      
#ifdef AUDIO_LOG
      fprintf(stdout, "AddSource replace source %d\n", i);
#endif
      
      break;
    }
  }
  
  if (i == sources_.size())
  {
    sources_.push_back(source_info);
    
#ifdef AUDIO_LOG
    fprintf(stdout, "AddSource push back source %d\n", i);
#endif
  }
  
  return i;
}

void AudioHandlerAl::RemoveSource(int source_ref)
{
  assert(source_ref >= 0 && source_ref < sources_.size());
  
  if (-1 == sources_[source_ref].buffer_ref)
    return;
  
  ALenum error = AL_NO_ERROR;
  
  alDeleteSources(1, &sources_[source_ref].source);
  if ((error = alGetError()) != AL_NO_ERROR)
  {
    fprintf(stderr, "Error(%x) deleting source %d\n", error, source_ref);
  }
  
  --buffers_[sources_[source_ref].buffer_ref].reference_count;
  
#ifdef AUDIO_LOG
  fprintf(stdout, "RemoveSource %d, minus ref count of buffer %d: %d\n",
          source_ref, sources_[source_ref].buffer_ref,
          buffers_[sources_[source_ref].buffer_ref].reference_count);
#endif
  
  sources_[source_ref].buffer_ref = -1;
}

} // namespace hikaru
