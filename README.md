hikaru
======

an audio manager can play sound / bgm

platform
--------
### iOS
* __sound :__ [OpenAL](http://www.openal.org/)
* __bgm :__ [AVAudioPlayer](http://developer.apple.com/library/ios/#DOCUMENTATION/AVFoundation/Reference/AVAudioPlayerClassReference/Reference/Reference.html)
* __audio file format :__ support by [Extended Audio File Services](https://developer.apple.com/library/mac/#documentation/MusicAudio/Reference/ExtendedAudioFileServicesReference/Reference/reference.html)

### Mac
* __sound / bgm :__ OpenAL
* __audio file format :__ support by Extended Audio File Services

### Windows
* __sound / bgm :__ OpenAL
* __audio file format :__ wav / ogg

### Android (ndk native activity only)
* __sound :__ [SoundPool](http://developer.android.com/reference/android/media/SoundPool.html)
* __bgm :__ [MediaPlayer](http://developer.android.com/reference/android/media/MediaPlayer.html)
* __audio file format :__ see [Here](http://developer.android.com/guide/appendix/media-formats.html#core)

install
-------
to be done ...

usage
-----
1. `#include "audio_mgr.h"`
2. use `hikaru::AudioMgr::Ins().Init()` to init
3. use `hikaru::AudioMgr::Ins().Update()` in your program's main loop (for bgm fading effect)
4. use `hikaru::AudioMgr`'s interfaces to control audio
5. use `hikaru::AudioMgr::KillIns()` to clean up
