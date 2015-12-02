#include "audio_platform_helper_ios.h"

#import <AudioToolbox/AudioToolbox.h>

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
  
  NSString* routeStr = (__bridge NSString*)route;
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
  prepare_bgm_player_(nil),
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
#if !__has_feature(objc_arc)
  [prepare_bgm_player_ release];
  [bgm_player_ release];
#endif
}

bool AudioPlatformHelperIOS::IsCanPlayBgm()
{
  return !is_ipod_playing_;
}
  
bool AudioPlatformHelperIOS::PrepareBgm(const std::string& resource)
{
  if (0 == prepare_bgm_resource_.compare(resource))
    return true;
  
#if !__has_feature(objc_arc)
  [prepare_bgm_player_ release];
#endif
  
  prepare_bgm_player_ = nil;
  prepare_bgm_resource_.clear();
  
  NSString* file_name = [[NSString alloc] initWithUTF8String:resource.c_str()];
  NSURL* url = [[NSURL alloc] initFileURLWithPath: [[NSBundle mainBundle] pathForResource:file_name ofType:nil]];

#if !__has_feature(objc_arc)
  [file_name release];
#endif
  
  prepare_bgm_player_ = [[AVAudioPlayer alloc] initWithContentsOfURL:url error:nil];
  if (prepare_bgm_player_)
  {
    prepare_bgm_resource_ = resource;
    if (![prepare_bgm_player_ prepareToPlay])
    {
#if !__has_feature(objc_arc)
      [prepare_bgm_player_ release];
#endif

      prepare_bgm_player_ = nil;
      prepare_bgm_resource_.clear();
    }
  }
  
#if !__has_feature(objc_arc)
  [url release];
#endif

  return (nil != prepare_bgm_player_);
}

bool AudioPlatformHelperIOS::PlayBgm(const std::string& resource, float volume)
{
  if (0 != prepare_bgm_resource_.compare(resource))
    PrepareBgm(resource);
  
  if (nil == prepare_bgm_player_)
    return false;

  bgm_player_ = prepare_bgm_player_;

  // clear prepare info
  prepare_bgm_player_ = nil;
  prepare_bgm_resource_.clear();
  
  bgm_player_.numberOfLoops = -1;
  bgm_player_.volume = volume;
  
  if ([bgm_player_ respondsToSelector:@selector(setEnableRate:)])
    bgm_player_.enableRate = YES;
  
  return [bgm_player_ play] == YES;
}
  
bool AudioPlatformHelperIOS::PauseBgm()
{
  if (bgm_player_ != nil)
  {
    [bgm_player_ pause];
    return true;
  }
  
  return false;
}

bool AudioPlatformHelperIOS::ResumeBgm()
{
  if (bgm_player_ != nil)
    return [bgm_player_ play] == YES;
  
  return false;
}

bool AudioPlatformHelperIOS::StopBgm()
{
  if (bgm_player_ != nil)
  {
    [bgm_player_ stop];
    
#if !__has_feature(objc_arc)
    [bgm_player_ release];
#endif
    
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
