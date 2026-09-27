#include "audio_engine.h"

#include <string.h>

#include <mutex>
#include <vector>

#include <SDL.h>

#include "settings.h"
#include "sound_effects_cache.h"

namespace fallout {

// Background music and dialogue speech are each a single scalar Sound*
// global (gBackgroundSound/gSpeechSound in game_sound.cc), not pools -- there
// is no allocation loop to raise, so unlike SFX and floats these aren't
// runtime-configurable.
#define BACKGROUND_MUSIC_MAX_COUNT (1)
#define DIALOGUE_SPEECH_MAX_COUNT (1)

struct AudioEngineSoundBuffer {
    bool active = false;
    unsigned int size = 0;
    int bitsPerSample = 0;
    int channels = 0;
    int rate = 0;
    void* data = nullptr;
    int volume = 0;
    bool playing = false;
    bool looping = false;
    unsigned int pos = 0;
    SDL_AudioStream* stream = nullptr;
    std::recursive_mutex mutex;
};

extern bool gProgramIsActive;

static bool soundBufferIsValid(int soundBufferIndex);
static void audioEngineMixin(void* userData, Uint8* stream, int length);

static SDL_AudioSpec gAudioEngineSpec;
static SDL_AudioDeviceID gAudioEngineDeviceId = -1;

// FISSION-VOCK FIX: was a flat #define (12, raised from 8 to fit the new
// float-speech pool on top of the old worst-case budget of background music
// (1) + SFX (SOUND_EFFECTS_MAX_COUNT, 4) + dialogue speech (1) = 6, plus 2
// spare). Now derived from every category's actual budget instead of a
// hand-maintained number, so it can't silently drift out of sync with them.
// Floats are the only category configurable at runtime (see [vock-floats]
// FloatAudioChannels in game.cfg / settings.mod_settings.float_audio_channels),
// so this is computed once in audioEngineInit(), before the SDL device is
// opened and the mixer callback thread starts -- gAudioEngineSoundBuffers is
// never resized after that.
static int audioEngineSoundBufferCount()
{
    int floatAudioChannels = settings.mod_settings.float_audio_channels;
    if (floatAudioChannels < 1) {
        floatAudioChannels = 1;
    }

    return BACKGROUND_MUSIC_MAX_COUNT + SOUND_EFFECTS_MAX_COUNT + DIALOGUE_SPEECH_MAX_COUNT + floatAudioChannels;
}

static std::vector<AudioEngineSoundBuffer> gAudioEngineSoundBuffers;

static bool audioEngineIsInitialized()
{
    return gAudioEngineDeviceId != -1;
}

static bool soundBufferIsValid(int soundBufferIndex)
{
    return soundBufferIndex >= 0 && soundBufferIndex < (int)gAudioEngineSoundBuffers.size();
}

static void audioEngineMixin(void* userData, Uint8* stream, int length)
{
    memset(stream, gAudioEngineSpec.silence, length);

    if (!gProgramIsActive) {
        return;
    }

    for (int index = 0; index < (int)gAudioEngineSoundBuffers.size(); index++) {
        AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[index]);
        std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

        if (soundBuffer->active && soundBuffer->playing) {
            int srcFrameSize = soundBuffer->bitsPerSample / 8 * soundBuffer->channels;

            unsigned char buffer[1024];
            int pos = 0;
            while (pos < length) {
                int remaining = length - pos;
                if (remaining > sizeof(buffer)) {
                    remaining = sizeof(buffer);
                }

                // FISSION-VOCK FIX: bounds-check *before* reading the next frame, not
                // after. soundBuffer->size (derived from the decoded/loaded
                // sound data) is not guaranteed to be an exact multiple of
                // srcFrameSize -- confirmed via AddressSanitizer: a 2-byte
                // (16-bit mono) frame read one byte past the end of a
                // 72769-byte buffer (odd length) via SDL_AudioStreamPut,
                // heap-buffer-overflow. The old code only checked
                // soundBuffer->pos >= soundBuffer->size *after* already
                // reading srcFrameSize bytes, so the last partial frame of
                // any non-frame-aligned buffer could read past its end.
                if (soundBuffer->pos + srcFrameSize > soundBuffer->size) {
                    if (soundBuffer->looping) {
                        soundBuffer->pos = 0;
                    } else {
                        soundBuffer->playing = false;
                        break;
                    }
                }

                // TODO: Make something better than frame-by-frame convertion.
                SDL_AudioStreamPut(soundBuffer->stream, (unsigned char*)soundBuffer->data + soundBuffer->pos, srcFrameSize);
                soundBuffer->pos += srcFrameSize;

                int bytesRead = SDL_AudioStreamGet(soundBuffer->stream, buffer, remaining);
                if (bytesRead == -1) {
                    break;
                }

                SDL_MixAudioFormat(stream + pos, buffer, gAudioEngineSpec.format, bytesRead, soundBuffer->volume);

                pos += bytesRead;
            }
        }
    }
}

bool audioEngineInit()
{
    // FISSION-VOCK ADD: sized once, here, before SDL_OpenAudioDevice() below
    // starts the mixer callback thread that iterates this vector -- never
    // resized afterward.
    gAudioEngineSoundBuffers = std::vector<AudioEngineSoundBuffer>(audioEngineSoundBufferCount());

    SDL_AudioSpec desiredSpec;
    desiredSpec.freq = 22050;
    desiredSpec.format = AUDIO_S16;
    desiredSpec.channels = 2;
    desiredSpec.samples = 1024;
    desiredSpec.callback = audioEngineMixin;
    const char* driver = SDL_GetCurrentAudioDriver();
    // Prevent overriding channels, as some audio drivers (WASAPI) don't handle > 2 correctly in this context and play no sound.
    gAudioEngineDeviceId = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &gAudioEngineSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_FORMAT_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
    if (gAudioEngineDeviceId == -1) {
        return false;
    }

    SDL_PauseAudioDevice(gAudioEngineDeviceId, 0);

    return true;
}

void audioEngineExit()
{
    if (audioEngineIsInitialized()) {
        SDL_CloseAudioDevice(gAudioEngineDeviceId);
        gAudioEngineDeviceId = -1;
    }
}

void audioEnginePause()
{
    if (audioEngineIsInitialized()) {
        SDL_PauseAudioDevice(gAudioEngineDeviceId, 1);
    }
}

void audioEngineResume()
{
    if (audioEngineIsInitialized()) {
        SDL_PauseAudioDevice(gAudioEngineDeviceId, 0);
    }
}

int audioEngineCreateSoundBuffer(unsigned int size, int bitsPerSample, int channels, int rate)
{
    if (!audioEngineIsInitialized()) {
        return -1;
    }

    for (int index = 0; index < (int)gAudioEngineSoundBuffers.size(); index++) {
        AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[index]);
        std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

        if (!soundBuffer->active) {
            soundBuffer->active = true;
            soundBuffer->size = size;
            soundBuffer->bitsPerSample = bitsPerSample;
            soundBuffer->channels = channels;
            soundBuffer->rate = rate;
            soundBuffer->volume = SDL_MIX_MAXVOLUME;
            soundBuffer->playing = false;
            soundBuffer->looping = false;
            soundBuffer->pos = 0;
            soundBuffer->data = malloc(size);
            soundBuffer->stream = SDL_NewAudioStream(bitsPerSample == 16 ? AUDIO_S16 : AUDIO_S8, channels, rate, gAudioEngineSpec.format, gAudioEngineSpec.channels, gAudioEngineSpec.freq);
            return index;
        }
    }

    return -1;
}

bool audioEngineSoundBufferRelease(int soundBufferIndex)
{
    if (!audioEngineIsInitialized()) {
        return false;
    }

    if (!soundBufferIsValid(soundBufferIndex)) {
        return false;
    }

    AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[soundBufferIndex]);
    std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

    if (!soundBuffer->active) {
        return false;
    }

    soundBuffer->active = false;

    free(soundBuffer->data);
    soundBuffer->data = nullptr;

    SDL_FreeAudioStream(soundBuffer->stream);
    soundBuffer->stream = nullptr;

    return true;
}

bool audioEngineSoundBufferSetVolume(int soundBufferIndex, int volume)
{
    if (!audioEngineIsInitialized()) {
        return false;
    }

    if (!soundBufferIsValid(soundBufferIndex)) {
        return false;
    }

    AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[soundBufferIndex]);
    std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

    if (!soundBuffer->active) {
        return false;
    }

    soundBuffer->volume = volume;

    return true;
}

bool audioEngineSoundBufferGetVolume(int soundBufferIndex, int* volumePtr)
{
    if (!audioEngineIsInitialized()) {
        return false;
    }

    if (!soundBufferIsValid(soundBufferIndex)) {
        return false;
    }

    AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[soundBufferIndex]);
    std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

    if (!soundBuffer->active) {
        return false;
    }

    *volumePtr = soundBuffer->volume;

    return true;
}

bool audioEngineSoundBufferSetPan(int soundBufferIndex, int pan)
{
    if (!audioEngineIsInitialized()) {
        return false;
    }

    if (!soundBufferIsValid(soundBufferIndex)) {
        return false;
    }

    AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[soundBufferIndex]);
    std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

    if (!soundBuffer->active) {
        return false;
    }

    // NOTE: Audio engine does not support sound panning. I'm not sure it's
    // even needed. For now this value is silently ignored.

    return true;
}

bool audioEngineSoundBufferPlay(int soundBufferIndex, unsigned int flags)
{
    if (!audioEngineIsInitialized()) {
        return false;
    }

    if (!soundBufferIsValid(soundBufferIndex)) {
        return false;
    }

    AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[soundBufferIndex]);
    std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

    if (!soundBuffer->active) {
        return false;
    }

    soundBuffer->playing = true;

    if ((flags & AUDIO_ENGINE_SOUND_BUFFER_PLAY_LOOPING) != 0) {
        soundBuffer->looping = true;
    }

    return true;
}

bool audioEngineSoundBufferStop(int soundBufferIndex)
{
    if (!audioEngineIsInitialized()) {
        return false;
    }

    if (!soundBufferIsValid(soundBufferIndex)) {
        return false;
    }

    AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[soundBufferIndex]);
    std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

    if (!soundBuffer->active) {
        return false;
    }

    soundBuffer->playing = false;

    return true;
}

bool audioEngineSoundBufferGetCurrentPosition(int soundBufferIndex, unsigned int* readPosPtr, unsigned int* writePosPtr)
{
    if (!audioEngineIsInitialized()) {
        return false;
    }

    if (!soundBufferIsValid(soundBufferIndex)) {
        return false;
    }

    AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[soundBufferIndex]);
    std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

    if (!soundBuffer->active) {
        return false;
    }

    if (readPosPtr != nullptr) {
        *readPosPtr = soundBuffer->pos;
    }

    if (writePosPtr != nullptr) {
        *writePosPtr = soundBuffer->pos;

        if (soundBuffer->playing) {
            // 15 ms lead
            // See: https://docs.microsoft.com/en-us/previous-versions/windows/desktop/mt708925(v=vs.85)#remarks
            *writePosPtr += soundBuffer->rate / 150;
            *writePosPtr %= soundBuffer->size;
        }
    }

    return true;
}

bool audioEngineSoundBufferSetCurrentPosition(int soundBufferIndex, unsigned int pos)
{
    if (!audioEngineIsInitialized()) {
        return false;
    }

    if (!soundBufferIsValid(soundBufferIndex)) {
        return false;
    }

    AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[soundBufferIndex]);
    std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

    if (!soundBuffer->active) {
        return false;
    }

    soundBuffer->pos = pos % soundBuffer->size;

    return true;
}

bool audioEngineSoundBufferLock(int soundBufferIndex, unsigned int writePos, unsigned int writeBytes, void** audioPtr1, unsigned int* audioBytes1, void** audioPtr2, unsigned int* audioBytes2, unsigned int flags)
{
    if (!audioEngineIsInitialized()) {
        return false;
    }

    if (!soundBufferIsValid(soundBufferIndex)) {
        return false;
    }

    AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[soundBufferIndex]);
    std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

    if (!soundBuffer->active) {
        return false;
    }

    if (audioBytes1 == nullptr) {
        return false;
    }

    if ((flags & AUDIO_ENGINE_SOUND_BUFFER_LOCK_FROM_WRITE_POS) != 0) {
        if (!audioEngineSoundBufferGetCurrentPosition(soundBufferIndex, nullptr, &writePos)) {
            return false;
        }
    }

    if ((flags & AUDIO_ENGINE_SOUND_BUFFER_LOCK_ENTIRE_BUFFER) != 0) {
        writeBytes = soundBuffer->size;
    }

    if (writePos + writeBytes <= soundBuffer->size) {
        *(unsigned char**)audioPtr1 = (unsigned char*)soundBuffer->data + writePos;
        *audioBytes1 = writeBytes;

        if (audioPtr2 != nullptr) {
            *audioPtr2 = nullptr;
        }

        if (audioBytes2 != nullptr) {
            *audioBytes2 = 0;
        }
    } else {
        unsigned int remainder = writePos + writeBytes - soundBuffer->size;
        *(unsigned char**)audioPtr1 = (unsigned char*)soundBuffer->data + writePos;
        *audioBytes1 = soundBuffer->size - writePos;

        if (audioPtr2 != nullptr) {
            *(unsigned char**)audioPtr2 = (unsigned char*)soundBuffer->data;
        }

        if (audioBytes2 != nullptr) {
            *audioBytes2 = writeBytes - (soundBuffer->size - writePos);
        }
    }

    // TODO: Mark range as locked.

    return true;
}

bool audioEngineSoundBufferUnlock(int soundBufferIndex, void* audioPtr1, unsigned int audioBytes1, void* audioPtr2, unsigned int audioBytes2)
{
    if (!audioEngineIsInitialized()) {
        return false;
    }

    if (!soundBufferIsValid(soundBufferIndex)) {
        return false;
    }

    AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[soundBufferIndex]);
    std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

    if (!soundBuffer->active) {
        return false;
    }

    // TODO: Mark range as unlocked.

    return true;
}

bool audioEngineSoundBufferGetStatus(int soundBufferIndex, unsigned int* statusPtr)
{
    if (!audioEngineIsInitialized()) {
        return false;
    }

    if (!soundBufferIsValid(soundBufferIndex)) {
        return false;
    }

    AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[soundBufferIndex]);
    std::lock_guard<std::recursive_mutex> lock(soundBuffer->mutex);

    if (!soundBuffer->active) {
        return false;
    }

    if (statusPtr == nullptr) {
        return false;
    }

    *statusPtr = 0;

    if (soundBuffer->playing) {
        *statusPtr |= AUDIO_ENGINE_SOUND_BUFFER_STATUS_PLAYING;

        if (soundBuffer->looping) {
            *statusPtr |= AUDIO_ENGINE_SOUND_BUFFER_STATUS_LOOPING;
        }
    }

    return true;
}

} // namespace fallout
