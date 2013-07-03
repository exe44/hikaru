#include "audio_handler_al.h"

#include <cassert>
#include <algorithm>
#include <vorbisfile.h>

namespace hikaru {

struct WaveFormat
{
  unsigned short format_tag;
  short channels;
  long samples_per_sec;
  long avg_bytes_per_sec;
  short block_align;
  short bits_per_sample;
};

static void* GetAlAudioDataFromWav(const std::string& resource,
                                   ALsizei& size,
                                   ALenum& format,
                                   ALsizei& freq)
{
  FILE* f = fopen(resource.c_str(), "rb");
  if (NULL == f)
    return NULL;

  char buff[5];
  buff[4] = 0;
  int length, result;

  result = fread(buff, 1, 4, f);
  if (result != 4 || strcmp(buff, "RIFF") != 0)
  {
    fclose(f);
    return NULL;
  }

  long chunk_size;
  result = fread(&chunk_size, 4, 1, f);
  if (result != 1)
  {
    fclose(f);
    return NULL;
  }

  result = fread(buff, 1, 4, f);
  if (result != 4 || strcmp(buff, "WAVE") != 0)
  {
    fclose(f);
    return NULL;
  }

  result = fread(buff, 1, 4, f);
  if (result != 4 || strcmp(buff, "fmt ") != 0)
  {
    fclose(f);
    return NULL;
  }

  result = fread(&chunk_size, 4, 1, f);
  if (result != 1)
  {
    fclose(f);
    return NULL;
  }

  WaveFormat wave_format;
  int format_size = sizeof(wave_format);
  result = fread(&wave_format, format_size, 1, f);
  if (result != 1)
  {
    fclose(f);
    return NULL;
  }

  bool valid = false;
  if (wave_format.channels == 1)
  {
    if (wave_format.bits_per_sample == 8)
    {
      format = AL_FORMAT_MONO8;
      valid = true;
    }
    else if (wave_format.bits_per_sample == 16)
    {
      format = AL_FORMAT_MONO16;
      valid = true;
    }
  }
  if (wave_format.channels == 2)
  {
    if (wave_format.bits_per_sample == 8)
    {
      format = AL_FORMAT_STEREO8;
      valid = true;
    }
    else if (wave_format.bits_per_sample == 16)
    {
      format = AL_FORMAT_STEREO16;
      valid = true;
    }
  }

  if (!valid)
  {
    fclose(f);
    return NULL;
  }

  result = fread(buff, 1, 4, f);
  if (result != 4 || strcmp(buff, "data") != 0)
  {
    fclose(f);
    return NULL;
  }

  result = fread(&chunk_size, 4, 1, f);
  if (result != 1)
  {
    fclose(f);
    return NULL;
  }

  void* data = malloc(chunk_size);
  result = fread(data, 1, chunk_size, f);
  if (result != chunk_size)
  {
    free(data);
    fclose(f);
    return NULL;
  }

  fclose(f);

  size = chunk_size;
  freq = wave_format.samples_per_sec;

  return data;
}

#define BUFFER_SIZE 32768

static void* GetAlAudioDataFromOgg(const std::string& resource,
                                   ALsizei& size,
                                   ALenum& format,
                                   ALsizei& freq)
{
  // Open for binary reading
  FILE* f = fopen(resource.c_str(), "rb");
  if (NULL == f)
    return NULL;

  OggVorbis_File ogg_file;
  ov_open(f, &ogg_file, NULL, 0);

  vorbis_info* info = ov_info(&ogg_file, -1);

  if (info->channels == 1)
  {
    format = AL_FORMAT_MONO16;
  }
  else if (info->channels == 2)
  {
    format = AL_FORMAT_STEREO16;
  }
  else
  {
    ov_clear(&ogg_file);
    return NULL;
  }

  freq = info->rate;

  long length = static_cast<long>(ov_pcm_total(&ogg_file,-1));
  size = length * info->channels * 2;
  void* data = malloc(size);
  char* ptr = static_cast<char*>(data);

  int endian = 0;             // 0 for Little-Endian, 1 for Big-Endian
  int bit_stream;
  long bytes;
  char array[BUFFER_SIZE];    // Local fixed size array

  do
  {
    // Read up to a buffer's worth of decoded sound data
    bytes = ov_read(&ogg_file, array, BUFFER_SIZE, endian, 2, 1, &bit_stream);
    // Append to end of buffer
    memcpy(ptr, array, bytes);
    ptr += bytes;
  }
  while (bytes > 0);

  ov_clear(&ogg_file);

  return data;
}

void* GetAlAudioData(const std::string& resource,
                     ALsizei& size,
                     ALenum& format,
                     ALsizei& freq)
{
  size_t pos = resource.rfind('.');
  if (pos == std::string::npos)
    return NULL;

  std::string ext = resource.substr(pos + 1);
  std::transform(ext.begin(), ext.end(), ext.begin(), tolower);

  if (ext.compare("wav") == 0)
  {
    return GetAlAudioDataFromWav(resource, size, format, freq);
  }
  else if (ext.compare("ogg") == 0)
  {
    return GetAlAudioDataFromOgg(resource, size, format, freq);
  }

  return NULL;
}

} // namespace hikaru
