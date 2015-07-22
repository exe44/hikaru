#include "audio_handler_android.h"

#include "android_helper.h"

namespace hikaru {

static const char* hikaru_class = "com/exe/hikaru";

AudioHandlerAndroid::AudioHandlerAndroid() : bgm_ref_(-1), global_sound_volume_(1.f)
{
  mana::JavaCaller caller;
  caller.Set(hikaru_class, "AudioInit", "()V");
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func);
  caller.End();
}

AudioHandlerAndroid::~AudioHandlerAndroid()
{
  mana::JavaCaller caller;
  caller.Set(hikaru_class, "AudioClear", "()V");
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func);
  caller.End();
}

void AudioHandlerAndroid::Pause()
{
  mana::JavaCaller caller;
  caller.Set(hikaru_class, "AudioPause", "()V");
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func);
  caller.End();
}

void AudioHandlerAndroid::Resume()
{
  mana::JavaCaller caller;
  caller.Set(hikaru_class, "AudioResume", "()V");
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func);
  caller.End();
}

void AudioHandlerAndroid::LoadSound(const std::string& resource)
{
  mana::JavaCaller caller;
  // caller.Set(hikaru_class, "LoadSound", "(Ljava/lang/String;)V");
  caller.Set(hikaru_class, "LoadSound", "(Landroid/app/Activity;Ljava/lang/String;)V");

  // std::string full_path = g_android_app->activity->internalDataPath + std::string("/") + resource;

  jstring jstr_resource = caller.env->NewStringUTF(resource.c_str());
  // caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func, jstr_resource);
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func, g_android_app->activity->clazz, jstr_resource);
  caller.env->DeleteLocalRef(jstr_resource);

  caller.End();
}

void AudioHandlerAndroid::UnloadSound(const std::string& resource)
{
  mana::JavaCaller caller;
  caller.Set(hikaru_class, "UnloadSound", "(Ljava/lang/String;)V");

  // std::string full_path = g_android_app->activity->internalDataPath + std::string("/") + resource;

  jstring jstr_resource = caller.env->NewStringUTF(resource.c_str());
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func, jstr_resource);
  caller.env->DeleteLocalRef(jstr_resource);

  caller.End();
}

int AudioHandlerAndroid::PlaySound(const std::string& resource, bool is_loop, float volume, float pitch)
{
  if (global_sound_volume_ <= 0.f)
    return -1;

  mana::JavaCaller caller;
  // caller.Set(hikaru_class, "PlaySound", "(Ljava/lang/String;ZFF)I");
  caller.Set(hikaru_class, "PlaySound", "(Landroid/app/Activity;Ljava/lang/String;ZFF)I");

  // std::string full_path = g_android_app->activity->internalDataPath + std::string("/") + resource;

  jstring jstr_resource = caller.env->NewStringUTF(resource.c_str());
  // jint sound_id = caller.env->CallStaticIntMethod(caller.user_class, caller.user_func, jstr_resource, is_loop, volume, pitch);
  int sound_id = (int)caller.env->CallStaticIntMethod(caller.user_class, caller.user_func, g_android_app->activity->clazz, jstr_resource, is_loop, volume * global_sound_volume_, pitch);
  caller.env->DeleteLocalRef(jstr_resource);

  caller.End();

  return sound_id;
}

void AudioHandlerAndroid::StopSound(int sound_id, bool wait_finish)
{
  mana::JavaCaller caller;
  caller.Set(hikaru_class, "StopSound", "(IZ)V");
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func, sound_id, wait_finish);
  caller.End();
}

void AudioHandlerAndroid::StopAllSounds(bool wait_finish)
{
  mana::JavaCaller caller;
  caller.Set(hikaru_class, "StopAllSounds", "(Z)V");
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func, wait_finish);
  caller.End();
}

void AudioHandlerAndroid::SetSoundVolume(int sound_id, float volume)
{
  mana::JavaCaller caller;
  caller.Set(hikaru_class, "SetSoundVolume", "(IF)V");
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func, sound_id, volume * global_sound_volume_);
  caller.End();
}

void AudioHandlerAndroid::SetGlobalSoundVolume(float volume)
{
  global_sound_volume_ = volume;
}

void AudioHandlerAndroid::SetSoundPitch(int sound_id, float pitch)
{
  mana::JavaCaller caller;
  caller.Set(hikaru_class, "SetSoundPitch", "(IF)V");
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func, sound_id, pitch);
  caller.End();
}

void AudioHandlerAndroid::PrepareBgm(const std::string& resource)
{
  // TODO:
}

void AudioHandlerAndroid::PlayBgm(const std::string& resource, float volume)
{
  mana::JavaCaller caller;
  // caller.Set(hikaru_class, "PlayBgm", "(Ljava/lang/String;F)V");
  caller.Set(hikaru_class, "PlayBgm", "(Landroid/app/Activity;Ljava/lang/String;F)V");

  // std::string full_path = g_android_app->activity->internalDataPath + std::string("/") + resource;

  jstring jstr_resource = caller.env->NewStringUTF(resource.c_str());
  // caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func, jstr_resource, volume);
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func, g_android_app->activity->clazz, jstr_resource, volume);
  caller.env->DeleteLocalRef(jstr_resource);

  caller.End();

  bgm_ref_ = 0;
}

void AudioHandlerAndroid::StopBgm()
{
  if (!IsPlayingBgm())
    return;

  mana::JavaCaller caller;
  caller.Set(hikaru_class, "StopBgm", "()V");
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func);
  caller.End();

  bgm_ref_ = -1;
}

void AudioHandlerAndroid::SetBgmVolume(float volume)
{
  if (!IsPlayingBgm())
    return;

  mana::JavaCaller caller;
  caller.Set(hikaru_class, "SetBgmVolume", "(F)V");
  caller.env->CallStaticVoidMethod(caller.user_class, caller.user_func, volume);
  caller.End();
}

bool AudioHandlerAndroid::IsPlayingBgm()
{
  return bgm_ref_ != -1;
}

} // namespace hikaru
