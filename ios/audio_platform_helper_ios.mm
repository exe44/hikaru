#include "audio_platform_helper_ios.h"

namespace hikaru {

static bool IsHeadsetPluggedIn()
{
  CFStringRef route;
  UInt32 routeSize = sizeof(CFStringRef);
  AudioSessionGetProperty(kAudioSessionProperty_AudioRoute, &routeSize, &route);
  
  /* Known values of route:
   "Headset"
   "Headphone"
   "Speaker"
   "SpeakerAndMicrophone"
   "HeadphonesAndMicrophone"
   "HeadsetInOut"
   "ReceiverAndMicrophone"
   "Lineout" */
  
  NSString* routeStr = (NSString*)route;
  if (routeStr)
  {
    NSRange headRange = [routeStr rangeOfString: @"Head"];
    if(headRange.location != NSNotFound)
    {
      NSLog(@"headset plugged!");
      return true;
    }
  }
  
  NSLog(@"headset not plugged!");
  return false;
}

static void MyPropertyListener(void* inClientData, AudioSessionPropertyID inID,
                               UInt32 inDataSize, const void* inData)
{
  if (!IsHeadsetPluggedIn())
  {
    UInt32 audio_route_override = kAudioSessionOverrideAudioRoute_Speaker;
    AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute, sizeof(audio_route_override), &audio_route_override);
  }
}

AudioPlatformHelperIOS::AudioPlatformHelperIOS(bool need_record)
  : bgm_player_(nil),
  is_ipod_playing_(0)
{
  if (need_record)
  {
    [[AVAudioSession sharedInstance]
     setCategory: AVAudioSessionCategoryPlayAndRecord
     error: nil];
    
    UInt32 mix = TRUE;
    AudioSessionSetProperty(kAudioSessionProperty_OverrideCategoryMixWithOthers, sizeof(mix), &mix);
    
    AudioSessionAddPropertyListener(kAudioSessionProperty_AudioRouteChange, MyPropertyListener, this);
    
    if (!IsHeadsetPluggedIn())
    {
      UInt32 audio_route_override = kAudioSessionOverrideAudioRoute_Speaker;
      AudioSessionSetProperty(kAudioSessionProperty_OverrideAudioRoute, sizeof(audio_route_override), &audio_route_override);
    }
    
    [[AVAudioSession sharedInstance] setActive:YES error:nil];
  }
  else
  {
    UInt32 propertySize = sizeof(is_ipod_playing_);
    
    AudioSessionGetProperty(kAudioSessionProperty_OtherAudioIsPlaying,
                            &propertySize, &is_ipod_playing_);
    
    if (is_ipod_playing_)
    {
      [[AVAudioSession sharedInstance]
       setCategory: AVAudioSessionCategoryAmbient
       error: nil];
    }
    else
    {
      [[AVAudioSession sharedInstance]
       setCategory: AVAudioSessionCategorySoloAmbient
       error: nil];
    }
  }
}

AudioPlatformHelperIOS::~AudioPlatformHelperIOS()
{
  [bgm_player_ release];
}

bool AudioPlatformHelperIOS::IsCanPlayBgm()
{
  return !is_ipod_playing_;
}

bool AudioPlatformHelperIOS::PlayBgm(const std::string& resource, float volume)
{
  [bgm_player_ release];
  
  NSString* file_name = [[NSString alloc] initWithUTF8String:resource.c_str()];
  NSURL* url = [[NSURL alloc] initFileURLWithPath: [[NSBundle mainBundle] pathForResource:file_name ofType:nil]];
  [file_name release];
  
  bgm_player_ = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:nil];
  [url release];
  
  bgm_player_.numberOfLoops = -1;
  bgm_player_.volume = volume;
  
  if ([bgm_player_ respondsToSelector:@selector(setEnableRate:)])
    bgm_player_.enableRate = YES;
  
  return [bgm_player_ play] == YES;
}

bool AudioPlatformHelperIOS::StopBgm()
{
  if (bgm_player_ != nil)
  {
    [bgm_player_ stop];
    [bgm_player_ release];
    bgm_player_ = nil;

    return true;
  }
  
  return false;
}

bool AudioPlatformHelperIOS::SetBgmVolume(float volume)
{
  if (bgm_player_ != nil)
  {
    bgm_player_.volume = volume;
    return true;
  }
  
  return false;
}

bool AudioPlatformHelperIOS::SetBgmRate(float rate)
{
  if (bgm_player_ != nil && [bgm_player_ respondsToSelector:@selector(setRate:)])
  {
    bgm_player_.rate = rate;
    return true;
  }

  return false;
}

} // namespace hikaru
