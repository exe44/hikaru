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

public class hikaru
{
  private static class PlaySoundEntry
  {
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

  public static void AudioInit()
  {
    sound_pool = new SoundPool(16, AudioManager.STREAM_MUSIC, 0);
    sound_id_map = new HashMap<String, Integer>();
    wait_play_sounds = new LinkedList<PlaySoundEntry>();
    wait_stream_id_map = new SparseIntArray();
    wait_stream_id_counter = 10000;
    
    sound_pool.setOnLoadCompleteListener(new SoundPool.OnLoadCompleteListener()
    {
      @Override
      public void onLoadComplete(SoundPool soundPool, int sampleId, int status)
      {
        if (status != 0)
        {
          Log.w("PlaySound", "load sound id " + sampleId + " failed");
          return;
        }

        ListIterator<PlaySoundEntry> it = wait_play_sounds.listIterator();
        while (it.hasNext())
        {
          PlaySoundEntry entry = it.next();
          if (entry.sound_id == sampleId)
          {
            int stream_id = sound_pool.play(sampleId, entry.volume, entry.volume, 0, entry.is_loop ? -1 : 0, entry.pitch);
            // Log.i("PlaySound", "succ wait sound id " + sampleId + " wait id " + entry.wait_stream_id + " stream id " + stream_id);
            
            wait_stream_id_map.put(entry.wait_stream_id, stream_id);

            it.remove();
          }
        }
      }
    });
    
    media_player = new MediaPlayer();   
  }
  
  public static void AudioClear()
  {
    sound_pool.release();
    sound_pool = null;
    media_player.release();
    media_player = null;
  }
  
  // public static void LoadSound(String resource)
  public static void LoadSound(Activity activity, String resource)
  {
    try
    {
      Integer sound_id_obj = sound_id_map.get(resource);
      if (sound_id_obj == null)
      {
        // sound_id_obj = sound_pool.load(resource, 1);
        sound_id_obj = sound_pool.load(activity.getAssets().openFd(resource), 1);
        
        if (sound_id_obj == null)
        {
          Log.w("LoadSound", "load " + resource + "failed!");
          return;
        }
        
        sound_id_map.put(resource, sound_id_obj);
      }
    }
    catch(Exception e)
    {
      Log.e("LoadSound", "LoadSound", e);
    }
  }
  
  public static void UnloadSound(String resource)
  {
    Integer sound_id_obj = sound_id_map.get(resource);
    if (sound_id_obj != null)
    {
      sound_pool.unload(sound_id_obj.intValue());
      sound_id_map.remove(resource);
    }
  }
  
  // public static int PlaySound(String resource, boolean is_loop, float volume, float pitch)
  public static int PlaySound(Activity activity, String resource, boolean is_loop, float volume, float pitch)
  {
    try
    {
      Integer sound_id_obj = sound_id_map.get(resource);
      if (sound_id_obj == null)
      {
        // sound_id_obj = sound_pool.load(resource, 1);
        sound_id_obj = sound_pool.load(activity.getAssets().openFd(resource), 1);
        
        if (sound_id_obj == null)
        {
          Log.w("PlaySound", "load " + resource + "failed!");
          return -1;
        }
        
        sound_id_map.put(resource, sound_id_obj);
              
        PlaySoundEntry entry = new PlaySoundEntry();
        entry.sound_id = sound_id_obj.intValue();
        entry.wait_stream_id = wait_stream_id_counter++;
        entry.is_loop = is_loop;
        entry.volume = volume;
        entry.pitch = pitch;
        wait_play_sounds.push(entry);
        // Log.i("PlaySound", "sound " + resource + " id " + entry.sound_id + " need wait id " + entry.wait_stream_id + "!");

        return entry.wait_stream_id;
      }
      
      int stream_id = sound_pool.play(sound_id_obj.intValue(), volume, volume, 0, is_loop ? -1 : 0, pitch);
      // Log.i("PlaySound", "succ stream id " + stream_id);
      
      return stream_id;
    }
    catch(Exception e)
    { 
      Log.e("PlaySound", "PlaySound", e);
    }
      
    return -1;
  }
  
  public static void StopSound(int stream_id, boolean wait_finish)
  {
    if (stream_id >= 10000)
    {
      int wait_stream_id = stream_id;
      stream_id = wait_stream_id_map.get(wait_stream_id);
      // Log.i("StopSound", "wait stream id " + wait_stream_id + " -> stream id " + stream_id);
    }
    
    if (wait_finish)
    {
      sound_pool.setLoop(stream_id, 0);
      // Log.i("StopSound", "stop loop stream id " + stream_id);
    }
    else
    {
      sound_pool.stop(stream_id);
      // Log.i("StopSound", "direct stop stream id " + stream_id);
    }
  }
  
  public static void SetSoundVolume(int stream_id, float volume)
  {
    if (stream_id >= 10000)
    {
      int wait_stream_id = stream_id;
      stream_id = wait_stream_id_map.get(wait_stream_id);
      // Log.i("SetSoundVolume", "wait stream id " + wait_stream_id + " -> stream id " + stream_id);
    }

    sound_pool.setVolume(stream_id, volume, volume);
  }

  public static void SetSoundPitch(int stream_id, float pitch)
  {
    if (stream_id >= 10000)
    {
      int wait_stream_id = stream_id;
      stream_id = wait_stream_id_map.get(wait_stream_id);
      // Log.i("SetSoundPitch", "wait stream id " + wait_stream_id + " -> stream id " + stream_id);
    }

    sound_pool.setRate(stream_id, pitch);
  }
  
  // public static void PlayBgm(String resource, float volume)
  public static void PlayBgm(Activity activity, String resource, float volume)
  {
    // Log.i("PlayBgm", "play bgm " + resource);
    
    try
    {
      media_player.reset();
      media_player.setAudioStreamType(AudioManager.STREAM_MUSIC);
      media_player.setLooping(true);
      media_player.setVolume(volume, volume);

      // FileInputStream fis = new FileInputStream(resource);
      // media_player.setDataSource(fis.getFD());
      AssetFileDescriptor afd = activity.getAssets().openFd(resource);
      media_player.setDataSource(afd.getFileDescriptor(), afd.getStartOffset(), afd.getLength());
      media_player.prepare();
      media_player.start();
      // fis.close();     
    }
    catch(Exception e)
    { 
        Log.e("PlayBgm", "PlayBgm", e);
    }
  }
  
  public static void StopBgm()
  {
    media_player.stop();
  }
  
  public static void SetBgmVolume(float volume)
  {
    media_player.setVolume(volume, volume);
  }
}
