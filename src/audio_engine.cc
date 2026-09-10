#include "audio_engine.h"

#include <stdlib.h>
#include <string.h>

#include <SDL.h>

#include "settings.h"
#include "sound_effects_cache.h"
#include "memory.h"

namespace fallout {

#define BACKGROUND_MUSIC_MAX_COUNT (1)
#define DIALOGUE_SPEECH_MAX_COUNT (1)

typedef struct AudioEngineSoundBuffer {
    bool active;
    unsigned int size;
    int bitsPerSample;
    int channels;
    int rate;
    void* data;
    int volume;
    bool playing;
    bool looping;
    unsigned int pos;
    SDL_AudioStream* stream;
    SDL_mutex* mutex;
} AudioEngineSoundBuffer;

extern bool gProgramIsActive;

static AudioEngineSoundBuffer* gAudioEngineSoundBuffers = NULL;
static int gAudioEngineSoundBufferCount = 0;

static bool soundBufferIsValid(int soundBufferIndex);
static void audioEngineMixin(void* userData, Uint8* stream, int length);

static SDL_AudioSpec gAudioEngineSpec;
static SDL_AudioDeviceID gAudioEngineDeviceId = -1;

static bool audioEngineIsInitialized()
{
    return gAudioEngineDeviceId != -1;
}

static bool soundBufferIsValid(int soundBufferIndex)
{
    return soundBufferIndex >= 0 && soundBufferIndex < gAudioEngineSoundBufferCount;
}

static void audioEngineMixin(void* userData, Uint8* stream, int length)
{
    memset(stream, gAudioEngineSpec.silence, length);

    if (!gProgramIsActive) {
        return;
    }

    for (int index = 0; index < gAudioEngineSoundBufferCount; index++) {
        AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[index]);

        SDL_LockMutex(soundBuffer->mutex);

        if (soundBuffer->active && soundBuffer->playing) {
            int srcFrameSize = soundBuffer->bitsPerSample / 8 * soundBuffer->channels;

            unsigned char buffer[1024];
            int pos = 0;
            while (pos < length) {
                int remaining = length - pos;
                if (remaining > sizeof(buffer)) {
                    remaining = sizeof(buffer);
                }

                if (soundBuffer->pos + srcFrameSize > soundBuffer->size) {
                    if (soundBuffer->looping) {
                        soundBuffer->pos = 0;
                    } else {
                        soundBuffer->playing = false;
                        break;
                    }
                }

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

        SDL_UnlockMutex(soundBuffer->mutex);
    }
}

bool audioEngineInit()
{
    int floatAudioChannels = settings.mod_settings.float_audio_channels;
    if (floatAudioChannels < 1) {
        floatAudioChannels = 1;
    }

    gAudioEngineSoundBufferCount = BACKGROUND_MUSIC_MAX_COUNT + SOUND_EFFECTS_MAX_COUNT + DIALOGUE_SPEECH_MAX_COUNT + floatAudioChannels;

    // Allocate exact number of buffers (no cap)
    gAudioEngineSoundBuffers = (AudioEngineSoundBuffer*)internal_malloc(sizeof(AudioEngineSoundBuffer) * gAudioEngineSoundBufferCount);
    if (gAudioEngineSoundBuffers == NULL) {
        return false;
    }

    // Initialize all buffers
    for (int index = 0; index < gAudioEngineSoundBufferCount; index++) {
        AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[index]);
        soundBuffer->active = false;
        soundBuffer->size = 0;
        soundBuffer->bitsPerSample = 0;
        soundBuffer->channels = 0;
        soundBuffer->rate = 0;
        soundBuffer->data = NULL;
        soundBuffer->volume = 0;
        soundBuffer->playing = false;
        soundBuffer->looping = false;
        soundBuffer->pos = 0;
        soundBuffer->stream = NULL;
        soundBuffer->mutex = SDL_CreateMutex();
    }

    SDL_AudioSpec desiredSpec;
    desiredSpec.freq = 22050;
    desiredSpec.format = AUDIO_S16;
    desiredSpec.channels = 2;
    desiredSpec.samples = 1024;
    desiredSpec.callback = audioEngineMixin;
    const char* driver = SDL_GetCurrentAudioDriver();
    gAudioEngineDeviceId = SDL_OpenAudioDevice(NULL, 0, &desiredSpec, &gAudioEngineSpec, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE | SDL_AUDIO_ALLOW_FORMAT_CHANGE | SDL_AUDIO_ALLOW_SAMPLES_CHANGE);
    if (gAudioEngineDeviceId == -1) {
        internal_free(gAudioEngineSoundBuffers);
        gAudioEngineSoundBuffers = NULL;
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

    // Clean up all buffers
    for (int index = 0; index < gAudioEngineSoundBufferCount; index++) {
        AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[index]);
        if (soundBuffer->mutex != NULL) {
            SDL_DestroyMutex(soundBuffer->mutex);
            soundBuffer->mutex = NULL;
        }
        if (soundBuffer->data != NULL) {
            free(soundBuffer->data);
            soundBuffer->data = NULL;
        }
        if (soundBuffer->stream != NULL) {
            SDL_FreeAudioStream(soundBuffer->stream);
            soundBuffer->stream = NULL;
        }
    }

    // Free the array itself
    if (gAudioEngineSoundBuffers != NULL) {
        internal_free(gAudioEngineSoundBuffers);
        gAudioEngineSoundBuffers = NULL;
    }
    gAudioEngineSoundBufferCount = 0;
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

    for (int index = 0; index < gAudioEngineSoundBufferCount; index++) {
        AudioEngineSoundBuffer* soundBuffer = &(gAudioEngineSoundBuffers[index]);

        SDL_LockMutex(soundBuffer->mutex);

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

            SDL_UnlockMutex(soundBuffer->mutex);
            return index;
        }

        SDL_UnlockMutex(soundBuffer->mutex);
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

    SDL_LockMutex(soundBuffer->mutex);

    if (!soundBuffer->active) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    soundBuffer->active = false;

    if (soundBuffer->data != NULL) {
        free(soundBuffer->data);
        soundBuffer->data = NULL;
    }

    if (soundBuffer->stream != NULL) {
        SDL_FreeAudioStream(soundBuffer->stream);
        soundBuffer->stream = NULL;
    }

    SDL_UnlockMutex(soundBuffer->mutex);

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

    SDL_LockMutex(soundBuffer->mutex);

    if (!soundBuffer->active) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    soundBuffer->volume = volume;

    SDL_UnlockMutex(soundBuffer->mutex);

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

    SDL_LockMutex(soundBuffer->mutex);

    if (!soundBuffer->active) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    *volumePtr = soundBuffer->volume;

    SDL_UnlockMutex(soundBuffer->mutex);

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

    SDL_LockMutex(soundBuffer->mutex);

    if (!soundBuffer->active) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    // NOTE: Audio engine does not support sound panning. I'm not sure it's
    // even needed. For now this value is silently ignored.

    SDL_UnlockMutex(soundBuffer->mutex);

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

    SDL_LockMutex(soundBuffer->mutex);

    if (!soundBuffer->active) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    soundBuffer->playing = true;

    if ((flags & AUDIO_ENGINE_SOUND_BUFFER_PLAY_LOOPING) != 0) {
        soundBuffer->looping = true;
    }

    SDL_UnlockMutex(soundBuffer->mutex);

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

    SDL_LockMutex(soundBuffer->mutex);

    if (!soundBuffer->active) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    soundBuffer->playing = false;

    SDL_UnlockMutex(soundBuffer->mutex);

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

    SDL_LockMutex(soundBuffer->mutex);

    if (!soundBuffer->active) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    if (readPosPtr != NULL) {
        *readPosPtr = soundBuffer->pos;
    }

    if (writePosPtr != NULL) {
        *writePosPtr = soundBuffer->pos;

        if (soundBuffer->playing) {
            // 15 ms lead
            // See: https://docs.microsoft.com/en-us/previous-versions/windows/desktop/mt708925(v=vs.85)#remarks
            *writePosPtr += soundBuffer->rate / 150;
            *writePosPtr %= soundBuffer->size;
        }
    }

    SDL_UnlockMutex(soundBuffer->mutex);

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

    SDL_LockMutex(soundBuffer->mutex);

    if (!soundBuffer->active) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    soundBuffer->pos = pos % soundBuffer->size;

    SDL_UnlockMutex(soundBuffer->mutex);

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

    SDL_LockMutex(soundBuffer->mutex);

    if (!soundBuffer->active) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    if (audioBytes1 == NULL) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    if ((flags & AUDIO_ENGINE_SOUND_BUFFER_LOCK_FROM_WRITE_POS) != 0) {
        if (!audioEngineSoundBufferGetCurrentPosition(soundBufferIndex, NULL, &writePos)) {
            SDL_UnlockMutex(soundBuffer->mutex);
            return false;
        }
    }

    if ((flags & AUDIO_ENGINE_SOUND_BUFFER_LOCK_ENTIRE_BUFFER) != 0) {
        writeBytes = soundBuffer->size;
    }

    if (writePos + writeBytes <= soundBuffer->size) {
        *(unsigned char**)audioPtr1 = (unsigned char*)soundBuffer->data + writePos;
        *audioBytes1 = writeBytes;

        if (audioPtr2 != NULL) {
            *audioPtr2 = NULL;
        }

        if (audioBytes2 != NULL) {
            *audioBytes2 = 0;
        }
    } else {
        unsigned int remainder = writePos + writeBytes - soundBuffer->size;
        *(unsigned char**)audioPtr1 = (unsigned char*)soundBuffer->data + writePos;
        *audioBytes1 = soundBuffer->size - writePos;

        if (audioPtr2 != NULL) {
            *(unsigned char**)audioPtr2 = (unsigned char*)soundBuffer->data;
        }

        if (audioBytes2 != NULL) {
            *audioBytes2 = writeBytes - (soundBuffer->size - writePos);
        }
    }

    SDL_UnlockMutex(soundBuffer->mutex);

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

    SDL_LockMutex(soundBuffer->mutex);

    if (!soundBuffer->active) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    SDL_UnlockMutex(soundBuffer->mutex);

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

    SDL_LockMutex(soundBuffer->mutex);

    if (!soundBuffer->active) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    if (statusPtr == NULL) {
        SDL_UnlockMutex(soundBuffer->mutex);
        return false;
    }

    *statusPtr = 0;

    if (soundBuffer->playing) {
        *statusPtr |= AUDIO_ENGINE_SOUND_BUFFER_STATUS_PLAYING;

        if (soundBuffer->looping) {
            *statusPtr |= AUDIO_ENGINE_SOUND_BUFFER_STATUS_LOOPING;
        }
    }

    SDL_UnlockMutex(soundBuffer->mutex);

    return true;
}

} // namespace fallout