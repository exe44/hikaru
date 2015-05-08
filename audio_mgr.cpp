#include "audio_mgr.h"

#include <cassert>

#if defined(HIKARU_IOS)
#include "audio_platform_helper_ios.h"
#endif

#if defined(HIKARU_ANDROID)
#include "audio_handler_android.h"
#else
#include "audio_handler_al.h"
#endif

namespace hikaru {

static std::string ReplaceResourceExtension(const std::string& resource,
                                            const std::string& extension)
{
  if (!extension.empty())
  {
    size_t pos = resource.rfind('.');
    if (pos != std::string::npos)
      return resource.substr(0, pos + 1) + extension;
  }

  return resource;
}

AudioMgr* AudioMgr::ins_ptr_ = NULL;

AudioMgr::AudioMgr()
  : bgm_volume_(1.f),
  bgm_fade_out_remain_time_(0.f),
  bgm_volume_transition_remain_time_(0.f),
  audio_handler_(NULL),
  platform_helper_(NULL)
{
}

AudioMgr::~AudioMgr()
{	
  delete platform_helper_;
  delete audio_handler_;
}

void AudioMgr::Init(bool need_record /*= false*/)
{
  assert(!platform_helper_ && !audio_handler_);

#if defined(HIKARU_IOS)
  platform_helper_ = new AudioPlatformHelperIOS(need_record);
#else
  platform_helper_ = new AudioPlatformHelper;
#endif

#if defined(HIKARU_ANDROID)
  audio_handler_ = new AudioHandlerAndroid();
#else
  audio_handler_ = new AudioHandlerAl(platform_helper_);
#endif
}

void AudioMgr::SetForceReplaceResourceExtension(const std::string& sound_ext,
                                                const std::string& bgm_ext)
{
  force_sound_ext_ = sound_ext;
  force_bgm_ext_ = bgm_ext;
}

void AudioMgr::Update(float delta_time)
{
  assert(audio_handler_);

  if (bgm_fade_out_remain_time_ > 0.0f)
  {
    bgm_fade_out_remain_time_ -= delta_time;
    if (bgm_fade_out_remain_time_ <= 0.0f)
    {
      if (wait_change_bgm_resource_.length() > 0)
      {
        audio_handler_->PlayBgm(wait_change_bgm_resource_, bgm_volume_);
        wait_change_bgm_resource_.clear();
      }
      else
      {
        audio_handler_->StopBgm();
        curr_bgm_resource_.clear();
      }
    }
    else
    {
      audio_handler_->SetBgmVolume(bgm_fade_out_remain_time_ / bgm_fade_out_period_ * bgm_volume_);
    }
  }
  
  if (bgm_volume_transition_remain_time_ > 0.f)
  {
    bgm_volume_transition_remain_time_ -= delta_time;
    if (bgm_volume_transition_remain_time_ <= 0.f)
    {
      bgm_volume_ = bgm_volume_transition_to_;
    }
    else
    {
      float progress = 1.f - bgm_volume_transition_remain_time_ / bgm_volume_transition_period_;
      bgm_volume_ = bgm_volume_transition_from_ * (1.f - progress) + bgm_volume_transition_to_ * progress;
    }
    
    audio_handler_->SetBgmVolume(bgm_volume_);
  }
}

void AudioMgr::Pause()
{
  assert(audio_handler_);

  audio_handler_->Pause();
}

void AudioMgr::Resume()
{
  assert(audio_handler_);

  audio_handler_->Resume();
}

void AudioMgr::LoadSound(const std::string& resource)
{
  assert(audio_handler_);

  audio_handler_->LoadSound(ReplaceResourceExtension(resource, force_sound_ext_));

  // TODO: sound reference id
}
  
void AudioMgr::UnloadSound(const std::string& resource)
{
  assert(audio_handler_);

  audio_handler_->UnloadSound(ReplaceResourceExtension(resource, force_sound_ext_));
  
  // TODO: sound reference id
}

int AudioMgr::PlaySound(const std::string& resource,
                        bool is_loop /*= false*/,
                        float volume /*= 1.0f*/,
                        float pitch /*= 1.0f*/)
{
  assert(audio_handler_);

  return audio_handler_->PlaySound(ReplaceResourceExtension(resource, force_sound_ext_), is_loop, volume, pitch);
}

void AudioMgr::StopSound(int sound_id, bool wait_finish /*= false*/)
{
  assert(audio_handler_);

  audio_handler_->StopSound(sound_id, wait_finish);
}

void AudioMgr::SetSoundVolume(int sound_id, float volume)
{
  assert(audio_handler_);

  audio_handler_->SetSoundVolume(sound_id, volume);
}

void AudioMgr::SetGlobalSoundVolume(float volume)
{
  assert(audio_handler_);

  audio_handler_->SetGlobalSoundVolume(volume);
}

void AudioMgr::SetSoundPitch(int sound_id, float pitch)
{
  assert(audio_handler_);

  audio_handler_->SetSoundPitch(sound_id, pitch);
}

void AudioMgr::SetSoundRefDist(int sound_id, float dist)
{
  assert(audio_handler_);

  audio_handler_->SetSoundRefDist(sound_id, dist);
}

void AudioMgr::SetSoundRelativePos(int sound_id, float x, float y, float z)
{
  assert(audio_handler_);

  audio_handler_->SetSoundRelativePos(sound_id, x, y, z);
}

void AudioMgr::PrepareBgm(const std::string& resource)
{
  assert(audio_handler_);
  
  audio_handler_->PrepareBgm(resource);
}
  
void AudioMgr::PlayBgm(const std::string& resource, float fade_out_period /*= 0.0f*/)
{
  assert(audio_handler_);
  
  if (!curr_bgm_resource_.empty() &&
      curr_bgm_resource_.compare(ReplaceResourceExtension(resource, force_bgm_ext_)) == 0)
  {
    return;
  }
  
  if (fade_out_period > 0.0f && audio_handler_->IsPlayingBgm())
  {
    if (bgm_fade_out_remain_time_ > 0.f)
    {
      if (fade_out_period < bgm_fade_out_remain_time_)
      {
        float ratio = bgm_fade_out_remain_time_ / bgm_fade_out_period_;
        bgm_fade_out_period_ = fade_out_period;
        bgm_fade_out_remain_time_ = fade_out_period * ratio;
      }
    }
    else
    {
      bgm_fade_out_period_ = bgm_fade_out_remain_time_ = fade_out_period;
    }
    
    wait_change_bgm_resource_ = ReplaceResourceExtension(resource, force_bgm_ext_);
  }
  else
  {
    curr_bgm_resource_ = ReplaceResourceExtension(resource, force_bgm_ext_);
    audio_handler_->PlayBgm(curr_bgm_resource_, bgm_volume_);
    
    wait_change_bgm_resource_.clear();
    bgm_fade_out_remain_time_ = 0.f;
  }
}

void AudioMgr::StopBgm(float fade_out_period /*= 0.0f*/)
{
  assert(audio_handler_);

  if (fade_out_period > 0.0f && audio_handler_->IsPlayingBgm())
  {
    if (bgm_fade_out_remain_time_ > 0.f)
    {
      if (fade_out_period < bgm_fade_out_remain_time_)
      {
        float ratio = bgm_fade_out_remain_time_ / bgm_fade_out_period_;
        bgm_fade_out_period_ = fade_out_period;
        bgm_fade_out_remain_time_ = fade_out_period * ratio;
      }
    }
    else
    {
      bgm_fade_out_period_ = bgm_fade_out_remain_time_ = fade_out_period;
    }
  }
  else
  {
    audio_handler_->StopBgm();
    curr_bgm_resource_.clear();
    bgm_fade_out_remain_time_ = 0.f;
  }
  
  wait_change_bgm_resource_.clear();
}

void AudioMgr::SetBgmVolume(float volume, float transition_period /*= 0.0f*/)
{
  assert(audio_handler_);

  if (transition_period <= 0.f)
  {
    bgm_volume_transition_remain_time_ = 0.f;
    bgm_volume_ = volume;
    
    audio_handler_->SetBgmVolume(bgm_volume_);
  }
  else if (volume == bgm_volume_)
  {
    bgm_volume_transition_remain_time_ = 0.f;
  }
  else
  {
    bgm_volume_transition_period_ = bgm_volume_transition_remain_time_ = transition_period;
    bgm_volume_transition_from_ = bgm_volume_;
    bgm_volume_transition_to_ = volume;
  }
}

void AudioMgr::SetBgmRate(float rate)
{
  assert(audio_handler_);

  audio_handler_->SetBgmRate(rate);
}

bool AudioMgr::IsPlayingBgm()
{
  assert(audio_handler_);

  return audio_handler_->IsPlayingBgm();
}

bool AudioMgr::IsCanPlayBgm()
{
  assert(platform_helper_);

  return platform_helper_->IsCanPlayBgm();
}

} // namespace hikaru
