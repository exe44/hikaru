package com.exe;

import java.util.HashMap;
import java.util.LinkedList;
import java.util.ListIterator;
import java.util.Map;

import android.app.Activity;
import android.content.res.AssetFileDescriptor;
import android.media.AudioManager;
import android.media.MediaPlayer;
import android.media.SoundPool;
import android.util.Log;
import android.util.SparseIntArray;

public class hikaru {

  private static final String TAG = "hikari";

  private static class PlaySoundEntry {
    int sound_id, wait_stream_id;
    boolean is_loop;
    float volume, pitch;
  }
  
  private static SoundPool sound_pool;
  private static Map<String, Integer> sound_id_map;
  private static LinkedList<PlaySoundEntry> wait_play_sounds;
  private static SparseIntArray wait_stream_id_map;
  private static int wait_stream_id_counter;
  
  private static MediaPlayer media_player;

  private static MediaPlayer prepare_media_player;
  private static String prepare_bgm_resource;

  public static void AudioInit() {
    sound_pool = new SoundPool(16, AudioManager.STREAM_MUSIC, 0);
    sound_id_map = new HashMap<>();
    wait_play_sounds = new LinkedList<>();
    wait_stream_id_map = new SparseIntArray();
    wait_stream_id_counter = 10000;
    
    sound_pool.setOnLoadCompleteListener(new SoundPool.OnLoadCompleteListener() {
      @Override
      public void onLoadComplete(SoundPool soundPool, int sampleId, int status) {
        if (0 != status) {
          Log.w(TAG, "on load complete sound id " + sampleId + " failed");
          return;
        }

        ListIterator<PlaySoundEntry> it = wait_play_sounds.listIterator();
        while (it.hasNext()) {
          PlaySoundEntry entry = it.next();
          if (entry.sound_id == sampleId) {
            int stream_id = sound_pool.play(sampleId, entry.volume, entry.volume, 0, entry.is_loop ? -1 : 0, entry.pitch);
            // Log.i(TAG, "PlaySound succ wait sound id " + sampleId + " wait id " + entry.wait_stream_id + " stream id " + stream_id);
            
            wait_stream_id_map.put(entry.wait_stream_id, stream_id);
            it.remove();
          }
        }
      }
    });
    
    media_player = null;
    prepare_media_player = null;
    prepare_bgm_resource = "";

    Log.i(TAG, "audio inited");
  }
  
  public static void AudioClear() {
    sound_pool.release();
    sound_pool = null;

    if (null != media_player) {
      media_player.release();
      media_player = null;
    }

    if (null != prepare_media_player) {
      prepare_media_player.release();
      prepare_media_player = null;
    }

    Log.i(TAG, "audio cleared");
  }
  
  public static void AudioPause() {
    sound_pool.autoPause();
    if (null != media_player)
      media_player.pause();
  }
  
  public static void AudioResume() {
    sound_pool.autoResume();
    if (null != media_player)
      media_player.start();
  }
  
  // public static void LoadSound(String resource)
  public static void LoadSound(Activity activity, String resource) {
    try
    {
      Integer sound_id_obj = sound_id_map.get(resource);
      if (null == sound_id_obj) {
        // sound_id_obj = sound_pool.load(resource, 1);
        sound_id_obj = sound_pool.load(activity.getAssets().openFd(resource), 1);
        sound_id_map.put(resource, sound_id_obj);
      }
    }
    catch(Exception e) {
      Log.e(TAG, "LoadSound", e);
    }
  }
  
  public static void UnloadSound(String resource) {
    Integer sound_id_obj = sound_id_map.get(resource);
    if (null != sound_id_obj) {
      sound_pool.unload(sound_id_obj);
      sound_id_map.remove(resource);
    }
  }
  
  // public static int PlaySound(String resource, boolean is_loop, float volume, float pitch)
  public static int PlaySound(Activity activity, String resource, boolean is_loop, float volume, float pitch) {
    try {
      Integer sound_id_obj = sound_id_map.get(resource);
      if (null == sound_id_obj) {
        // sound_id_obj = sound_pool.load(resource, 1);
        sound_id_obj = sound_pool.load(activity.getAssets().openFd(resource), 1);
        sound_id_map.put(resource, sound_id_obj);
              
        PlaySoundEntry entry = new PlaySoundEntry();
        entry.sound_id = sound_id_obj;
        entry.wait_stream_id = wait_stream_id_counter++;
        entry.is_loop = is_loop;
        entry.volume = volume;
        entry.pitch = pitch;
        wait_play_sounds.push(entry);
        // Log.i(TAG, "PlaySound " + resource + " id " + entry.sound_id + " need wait id " + entry.wait_stream_id + "!");

        return entry.wait_stream_id;
      }
      
      int stream_id = sound_pool.play(sound_id_obj, volume, volume, 0, is_loop ? -1 : 0, pitch);
      // Log.i(TAG, "PlaySound succ stream id " + stream_id);
      
      return stream_id;
    }
    catch(Exception e) {
      Log.e(TAG, "PlaySound", e);
    }
      
    return -1;
  }
  
  public static void StopSound(int stream_id, boolean wait_finish) {
    if (stream_id >= 10000) {
      int wait_stream_id = stream_id;
      stream_id = wait_stream_id_map.get(wait_stream_id);
      // Log.i(TAG, "StopSound wait stream id " + wait_stream_id + " -> stream id " + stream_id);
    }
    
    if (wait_finish) {
      sound_pool.setLoop(stream_id, 0);
      // Log.i(TAG, "StopSound stop loop stream id " + stream_id);
    }
    else {
      sound_pool.stop(stream_id);
      // Log.i(TAG, "StopSound direct stop stream id " + stream_id);
    }
  }
  
  public static void StopAllSounds(boolean wait_finish) {
    // TODO
  }

  public static void SetSoundVolume(int stream_id, float volume) {
    if (stream_id >= 10000) {
      int wait_stream_id = stream_id;
      stream_id = wait_stream_id_map.get(wait_stream_id);
      // Log.i(TAG, "SetSoundVolume wait stream id " + wait_stream_id + " -> stream id " + stream_id);
    }

    sound_pool.setVolume(stream_id, volume, volume);
  }

  public static void SetSoundPitch(int stream_id, float pitch) {
    if (stream_id >= 10000) {
      int wait_stream_id = stream_id;
      stream_id = wait_stream_id_map.get(wait_stream_id);
      // Log.i(TAG, "SetSoundPitch wait stream id " + wait_stream_id + " -> stream id " + stream_id);
    }

    sound_pool.setRate(stream_id, pitch);
  }

  // public static void PrepareBgm(String resource)
  public static void PrepareBgm(Activity activity, String resource) {
    // Log.i(TAG, "PrepareBgm " + resource);

    try {
      if (prepare_bgm_resource.equals(resource))
        return;

      if (null == prepare_media_player)
        prepare_media_player = new MediaPlayer();

      prepare_media_player.reset();
      prepare_media_player.setAudioStreamType(AudioManager.STREAM_MUSIC);
      prepare_media_player.setLooping(true);

      // FileInputStream fis = new FileInputStream(resource);
      // media_player.setDataSource(fis.getFD());
      AssetFileDescriptor afd = activity.getAssets().openFd(resource);
      prepare_media_player.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
      prepare_media_player.prepare();
      // fis.close();

      prepare_bgm_resource = resource;
    }
    catch(Exception e) {
      Log.e(TAG, "PrepareBgm", e);
    }
  }
  
  // public static void PlayBgm(String resource, float volume)
  public static void PlayBgm(Activity activity, String resource, float volume) {
    // Log.i(TAG, "PlayBgm " + resource);
    
    try {
      if (!prepare_bgm_resource.equals(resource))
        PrepareBgm(activity, resource);

      MediaPlayer tmp = media_player;
      media_player = prepare_media_player;
      prepare_media_player = tmp;

      prepare_bgm_resource = "";

      media_player.setVolume(volume, volume);
      media_player.start();
    }
    catch(Exception e) {
        Log.e(TAG, "PlayBgm", e);
    }
  }
  
  public static void StopBgm() {
    media_player.stop();
  }
  
  public static void SetBgmVolume(float volume) {
    media_player.setVolume(volume, volume);
  }
}
