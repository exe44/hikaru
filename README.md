hikaru
======
a c++ audio manager which can play sound / bgm

platform
--------
#### iOS
* __sound :__ [OpenAL](http://www.openal.org/)
* __bgm :__ [AVAudioPlayer](http://developer.apple.com/library/ios/#DOCUMENTATION/AVFoundation/Reference/AVAudioPlayerClassReference/Reference/Reference.html)
* __audio file format :__ support by [Extended Audio File Services](https://developer.apple.com/library/mac/#documentation/MusicAudio/Reference/ExtendedAudioFileServicesReference/Reference/reference.html)

#### Mac
* __sound / bgm :__ OpenAL
* __audio file format :__ support by Extended Audio File Services

#### Windows
* __sound / bgm :__ OpenAL
* __audio file format :__ wav / ogg

#### Android (ndk native activity only)
* __sound :__ [SoundPool](http://developer.android.com/reference/android/media/SoundPool.html)
* __bgm :__ [MediaPlayer](http://developer.android.com/reference/android/media/MediaPlayer.html)
* __audio file format :__ see [Here](http://developer.android.com/guide/appendix/media-formats.html#core)

install
-------
#### iOS
add `audio_mgr.h & cpp`, `audio_handler_al.h & cpp`, `apple/`, `ios/` into your project
#### Mac
add `audio_mgr.h & cpp`, `audio_handler_al.h & cpp`, `apple/` into your project
#### Windows
add `audio_mgr.h & cpp`, `audio_handler_al.h & cpp`, `win/` into your project
#### Android
1. add `audio_mgr.h & cpp`, `android/` into your ndk project (Android.mk)
2. add [mana/android/android_helper.h](https://github.com/exe44/mana/blob/master/android/android_helper.h) into your ndk project
3. import `android/hikaru.java` into your java project

usage
-----
1. `#include "audio_mgr.h"`
2. use `hikaru::AudioMgr::Ins().Init()` to init
3. use `hikaru::AudioMgr::Ins().Update()` in your program's main loop (for bgm fading effect)
4. use `hikaru::AudioMgr`'s interfaces to control audio
5. use `hikaru::AudioMgr::KillIns()` to clean up
