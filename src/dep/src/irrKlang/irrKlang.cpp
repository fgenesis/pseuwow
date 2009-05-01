#include "irrKlang.h"

/**
 * This file contains a null-implementation of the irrKlang library, which doesn't do anything but
 * provide stubs.
 */
namespace irrklang
  {
/*
    class DummySoundDeviceList : public ISoundDeviceList
      {
      public:
        void
        grab()
          {
          }

        bool
        drop()
          {
            return true;
          }

        ik_s32
        getDeviceCount()
          {
            return 0;
          }

        const char*
        getDeviceID(ik_s32 index)
          {
            return "NULL";
          }

        const char*
        getDeviceDescription(ik_s32 index)
          {
            return "NULL";
          }
      };

    class DummySoundSource : public ISoundSource
      {
      private:
        SAudioStreamFormat stream_format;

      public:
        void
        grab()
          {
          }

        bool
        drop()
          {
            return true;
          }

        const ik_c8*
        getName()
          {
            return "NULL";
          }

        void
        setStreamMode(E_STREAM_MODE mode)
          {
          }

        E_STREAM_MODE
        getStreamMode()
          {
            return ESM_AUTO_DETECT;
          }

        ik_u32
        getPlayLength()
          {
            return 0;
          }

        SAudioStreamFormat
        getAudioFormat()
          {
            return stream_format;
          }

        bool
        getIsSeekingSupported()
          {
            return true;
          }

        void
        setDefaultVolume(ik_f32 volume = 1.0f)
          {
          }

        ik_f32
        getDefaultVolume()
          {
            return 1.0f;
          }

        void
        setDefaultMinDistance(ik_f32 minDistance)
          {
          }

        ik_f32
        getDefaultMinDistance()
          {
            return -1;
          }

        void
        setDefaultMaxDistance(ik_f32 maxDistance)
          {
          }

        ik_f32
        getDefaultMaxDistance()
          {
            return -1;
          }

        void
        forceReloadAtNextUse()
          {
          }

        void
        setForcedStreamingThreshold(ik_s32 thresholdBytes)
          {
          }

        ik_s32
        getForcedStreamingThreshold()
          {
            return 1024 * 1024;
          }

        void*
        getSampleData()
          {
            return 0;
          }
      };

    class DummyAudioRecorder : public virtual IAudioRecorder
      {
      protected:
        static DummySoundSource sound_source;
        static SAudioStreamFormat stream_format;
        bool recording;

      public:
        DummyAudioRecorder()
          {

          }

        ISoundSource*
        addSoundSourceFromRecordedAudio(const char* soundName)
          {
            return &sound_source;
          }

        void
        clearRecordedAudioDataBuffer()
          {

          }

        bool
        isRecording()
          {
            return recording;
          }

        SAudioStreamFormat
        getAudioFormat()
          {
            return stream_format;
          }

        void*
        getRecordedAudioData()
          {
            return 0;
          }

        const char*
        getDriverName()
          {
            return "NULL";
          }
      };

    class DummyAudioStream : public IAudioStream
      {
      protected:
        SAudioStreamFormat stream_format;

      public:
        ~DummyAudioStream()
          {
          }

        SAudioStreamFormat
        getFormat()
          {
            return stream_format;
          }

        bool
        setPosition(ik_s32 pos) { return true; }

        bool
        getIsSeekingSupported()
          {
            return true;
          }

        ik_s32
        readFrames(void* target, ik_s32 frameCountToRead) { return 0; }
      };

    class DummyAudioStreamLoader : public IAudioStreamLoader
      {

      };

    class DummyFileFactory : public IFileFactory
      {

      };

    class DummyFileReader : public IFileReader
      {

      };

    class DummySound : public ISound
      {

      };

    class DummySoundEffectControl : public ISoundEffectControl
      {

      };

    class DummySoundEngine : public ISoundEngine
      {

      };
*/

    IRRKLANG_API ISoundEngine*
    IRRKLANGCALLCONV createIrrKlangDevice(E_SOUND_OUTPUT_DRIVER driver, int options,
        const char* deviceID, const char* sdk_version_do_not_use)
      {
        return 0;
      }

    IRRKLANG_API ISoundDeviceList*
    IRRKLANGCALLCONV createSoundDeviceList(E_SOUND_OUTPUT_DRIVER driver, const char* sdk_version_do_not_use)
      {
        return 0;
      }

    IRRKLANG_API IAudioRecorder*
    IRRKLANGCALLCONV createIrrKlangAudioRecorder(
        ISoundEngine* irrKlangDeviceForPlayback, E_SOUND_OUTPUT_DRIVER driver, const char* deviceID,
        const char* sdk_version_do_not_use)
      {
        return 0;
      }

    IRRKLANG_API ISoundDeviceList*
    IRRKLANGCALLCONV createAudioRecorderDeviceList(
        E_SOUND_OUTPUT_DRIVER drive, const char* sdk_version_do_not_use)
      {
        return 0;
      }

  }
