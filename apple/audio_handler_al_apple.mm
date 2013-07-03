#include "audio_handler_al.h"

#include "MyOpenALSupport.h"

namespace hikaru {

void* GetAlAudioData(const std::string& resource,
                     ALsizei& size,
                     ALenum& format,
                     ALsizei& freq)
{
  NSBundle* bundle = [NSBundle mainBundle];
  
  NSString *dir, *file_name;
  size_t pos = resource.rfind('/');
  if (pos != std::string::npos)
  {
    dir = [[NSString alloc] initWithUTF8String:resource.substr(0, pos).c_str()];
    file_name = [[NSString alloc] initWithUTF8String:resource.substr(pos + 1).c_str()];
  }
  else
  {
    dir = nil;
    file_name = [[NSString alloc] initWithUTF8String:resource.c_str()];
  }
  
  // get some audio data from a wave file
  CFURLRef fileURL = (CFURLRef)[[NSURL fileURLWithPath:[bundle pathForResource:file_name
                                                                        ofType:nil
                                                                   inDirectory:dir]] retain];
  
  [dir release];
  [file_name release];
  
  if (fileURL)
  { 
    void* data = MyGetOpenALAudioData(fileURL, &size, &format, &freq);
    CFRelease(fileURL);
    
    return data;
  }
  
  return NULL;
}

} // namespace hikaru
