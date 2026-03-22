#include "endgame.h"

#include <ctype.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "art.h"
#include "color.h"
#include "credits.h"
#include "cycle.h"
#include "db.h"
#include "dbox.h"
#include "debug.h"
#include "draw.h"
#include "game.h"
#include "game_mouse.h"
#include "game_movie.h"
#include "game_sound.h"
#include "input.h"
#include "map.h"
#include "memory.h"
#include "mouse.h"
#include "object.h"
#include "palette.h"
#include "pipboy.h"
#include "platform_compat.h"
#include "proto.h"
#include "random.h"
#include "settings.h"
#include "stat.h"
#include "svga.h"
#include "text_font.h"
#include "window_manager.h"
#include "word_wrap.h"
#include "worldmap.h"

namespace fallout {

#define DIR_SEPARATOR '/'

// The maximum number of subtitle lines per slide.
#define ENDGAME_ENDING_MAX_SUBTITLES (50)

#define ENDGAME_ENDING_WINDOW_WIDTH 640
#define ENDGAME_ENDING_WINDOW_HEIGHT 480

typedef struct EndgameDeathEnding {
    int gvar;
    int value;
    int worldAreaKnown;
    int worldAreaNotKnown;
    int min_level;
    int percentage;
    char voiceOverBaseName[16];

    // This flag denotes that the conditions for this ending is met and it was
    // selected as a candidate for final random selection.
    bool enabled;
} EndgameDeathEnding;

typedef struct EndgameEnding {
    int gvar;
    int value;
    int art_num;
    char voiceOverBaseName[12];
    int direction;
} EndgameEnding;

static void endgameEndingRenderPanningScene(int direction, const char* narratorFileName, int art_num);
static void endgameEndingRenderStaticScene(int art_num, const char* narratorFileName);
static int endgameEndingHandleContinuePlaying();
static int endgameEndingSlideshowWindowInit();
static void endgameEndingSlideshowWindowFree();
static void endgameEndingVoiceOverInit(const char* fname);
static void endgameEndingVoiceOverReset();
static void endgameEndingVoiceOverFree();
static void endgameEndingLoadPalette(int type, int id);
static void _endgame_voiceover_callback();
static int endgameEndingSubtitlesLoad(const char* filePath);
static void endgameEndingRefreshSubtitles();
static void endgameEndingSubtitlesFree();
static void _endgame_movie_callback();
static void _endgame_movie_bk_process();
static int endgameEndingInit();
static void endgameEndingFree();
static int endgameDeathEndingValidate(int* percentage);
static void endgameEndingUpdateOverlay();

static void generateEndgameReport();
static void generateEnddeathReport();

// The number of lines in current subtitles file.
//
// It's used as a length for two arrays:
// - [gEndgameEndingSubtitles]
// - [gEndgameEndingSubtitlesTimings]
//
// This value does not exceed [ENDGAME_ENDING_SUBTITLES_CAPACITY].
//
// 0x518668
static int gEndgameEndingSubtitlesLength = 0;

// The number of characters in current subtitles file.
//
// This value is used to determine
//
// 0x51866C
static int gEndgameEndingSubtitlesCharactersCount = 0;

// 0x518670
static int gEndgameEndingSubtitlesCurrentLine = 0;

// 0x518674
static int _endgame_maybe_done = 0;

// enddeath.txt
//
// 0x518678
static EndgameDeathEnding* gEndgameDeathEndings = nullptr;

// The number of death endings in [gEndgameDeathEndings] array.
//
// 0x51867C
static int gEndgameDeathEndingsLength = 0;

// Base file name for death ending.
//
// This value does not include extension.
//
// 0x570A90
static char gEndgameDeathEndingFileName[40];

// This flag denotes whether speech sound was successfully loaded for
// the current slide.
//
// 0x570AB8
static bool gEndgameEndingVoiceOverSpeechLoaded;

// 0x570ABC
static char gEndgameEndingSubtitlesLocalizedPath[COMPAT_MAX_PATH];

// The flag used to denote voice over speech for current slide has ended.
//
// 0x570BC0
static bool gEndgameEndingSpeechEnded;

// endgame.txt
//
// 0x570BC4
static EndgameEnding* gEndgameEndings;

// The array of text lines in current subtitles file.
//
// The length is specified in [gEndgameEndingSubtitlesLength]. It's capacity
// is [ENDGAME_ENDING_SUBTITLES_CAPACITY].
//
// 0x570BC8
static char** gEndgameEndingSubtitles;

// 0x570BCC
static bool gEndgameEndingSubtitlesEnabled;

// The flag used to denote voice over subtitles for current slide has ended.
//
// 0x570BD0
static bool gEndgameEndingSubtitlesEnded;

// 0x570BD4
static bool _endgame_map_enabled;

// 0x570BD8
static bool _endgame_mouse_state;

// The number of endings in [gEndgameEndings] array.
//
// 0x570BDC
static int gEndgameEndingsLength = 0;

// This flag denotes whether subtitles was successfully loaded for
// the current slide.
//
// 0x570BE0
static bool gEndgameEndingVoiceOverSubtitlesLoaded;

// Reference time is a timestamp when subtitle is first displayed.
//
// This value is used together with [gEndgameEndingSubtitlesTimings] array to
// determine when next line needs to be displayed.
//
// 0x570BE4
static unsigned int gEndgameEndingSubtitlesReferenceTime;

// The array of timings for each line in current subtitles file.
//
// The length is specified in [gEndgameEndingSubtitlesLength]. It's capacity
// is [ENDGAME_ENDING_SUBTITLES_CAPACITY].
//
// 0x570BE8
static unsigned int* gEndgameEndingSubtitlesTimings;

// Font that was current before endgame slideshow window was created.
//
// 0x570BEC
static int gEndgameEndingSlideshowOldFont;

// 0x570BF0
static unsigned char* gEndgameEndingSlideshowWindowBuffer;

// 0x570BF4
static int gEndgameEndingSlideshowWindow;

static int gEndgameEndingOverlay;

static void getScreenDimensions(int* width, int* height)
{
    if (gameIsWidescreen()) {
        *width = 800;
        *height = 500;
    } else {
        *width = 640;
        *height = 480;
    }
}

// 0x43F788
void endgamePlaySlideshow()
{
    if (endgameEndingSlideshowWindowInit() == -1) {
        return;
    }

    restoreUserAspectPreference();
    resizeContent(gameIsWidescreen() ? 800 : 640, gameIsWidescreen() ? 500 : 480);

    for (int index = 0; index < gEndgameEndingsLength; index++) {
        EndgameEnding* ending = &(gEndgameEndings[index]);
        int value = gameGetGlobalVar(ending->gvar);
        if (value == ending->value) {
            // Changed this to allow mods to set panning end-slides, but still plays original pan slide.
            if (ending->direction == 1 || ending->direction == -1) {
                endgameEndingRenderPanningScene(ending->direction, ending->voiceOverBaseName, ending->art_num);
            } else {
                endgameEndingRenderStaticScene(ending->art_num, ending->voiceOverBaseName);
            }
        }
    }

    resizeContent(screenGetWidth(), screenGetHeight(), true);

    endgameEndingSlideshowWindowFree();
}

// 0x43F810
void endgamePlayMovie()
{
    backgroundSoundDelete();
    isoDisable();
    paletteFadeTo(gPaletteBlack);
    _endgame_maybe_done = 0;
    tickersAdd(_endgame_movie_bk_process);
    backgroundSoundSetEndCallback(_endgame_movie_callback);
    backgroundSoundLoad("akiss", 12, 14, 15);
    inputPauseForTocks(3000);

    // NOTE: Result is ignored. I guess there was some kind of switch for male
    // vs. female ending, but it was not implemented.
    critterGetStat(gDude, STAT_GENDER);

    creditsOpen("credits.txt", -1, false);
    backgroundSoundDelete();
    backgroundSoundSetEndCallback(nullptr);
    tickersRemove(_endgame_movie_bk_process);
    backgroundSoundDelete();
    colorPaletteLoad("color.pal");
    paletteFadeTo(_cmap);
    isoEnable();
    endgameEndingHandleContinuePlaying();
}

// 0x43F8C4
static int endgameEndingHandleContinuePlaying()
{
    bool isoWasEnabled = isoDisable();

    bool gameMouseWasVisible;
    if (isoWasEnabled) {
        gameMouseWasVisible = gameMouseObjectsIsVisible();
    } else {
        gameMouseWasVisible = false;
    }

    if (gameMouseWasVisible) {
        gameMouseObjectsHide();
    }

    bool oldCursorIsHidden = cursorIsHidden();
    if (oldCursorIsHidden) {
        mouseShowCursor();
    }

    int oldCursor = gameMouseGetCursor();
    gameMouseSetCursor(MOUSE_CURSOR_ARROW);

    int rc;

    MessageListItem messageListItem;
    messageListItem.num = 30;
    if (messageListGetItem(&gMiscMessageList, &messageListItem)) {
        rc = showDialogBox(messageListItem.text, nullptr, 0, 169, 117, _colorTable[32328], nullptr, _colorTable[32328], DIALOG_BOX_YES_NO);
        if (rc == 0) {
            _game_user_wants_to_quit = 2;
        }
    } else {
        rc = -1;
    }

    gameMouseSetCursor(oldCursor);
    if (oldCursorIsHidden) {
        mouseHideCursor();
    }

    if (gameMouseWasVisible) {
        gameMouseObjectsShow();
    }

    if (isoWasEnabled) {
        isoEnable();
    }

    return rc;
}

// 0x43FBDC
static void endgameEndingRenderPanningScene(int direction, const char* narratorFileName, int art_num)
{
    // Get the appropriate art file ID, using widescreen variant if needed
    int fid = artGetFidWithVariant(OBJ_TYPE_INTERFACE, art_num, gameIsWidescreen());

    int screenWidth, screenHeight;
    getScreenDimensions(&screenWidth, &screenHeight);

    CacheEntry* backgroundHandle;
    Art* background = artLock(fid, &backgroundHandle);
    if (background != nullptr) {
        int artWidth = artGetWidth(background, 0, 0);
        int artHeight = artGetHeight(background, 0, 0);
        unsigned char* backgroundData = artGetFrameData(background, 0, 0);

        // Clear the screen buffer
        bufferFill(gEndgameEndingSlideshowWindowBuffer, screenWidth, screenHeight, screenWidth, _colorTable[0]);

        // Load the appropriate palette for this art
        endgameEndingLoadPalette(6, art_num);

        // Update overlay elements
        endgameEndingUpdateOverlay();

        // Store the current palette for fade effects
        unsigned char palette[768];
        memcpy(palette, _cmap, 768);

        // Set to black palette and initialize voiceover
        paletteSetEntries(gPaletteBlack);
        endgameEndingVoiceOverInit(narratorFileName);

        // Calculate panning parameters
        int totalPanDistance = artWidth - screenWidth;
        int fadeStartDistance = totalPanDistance / 4; // Distance where fade effects start
        unsigned int baseFrameDelay = 16; // Base delay between frame updates (ms)
        unsigned int estimatedTotalPanTime = baseFrameDelay * totalPanDistance;

        // Adjust timing if voiceover is longer than estimated pan time
        if (gEndgameEndingVoiceOverSpeechLoaded) {
            unsigned int voiceoverDurationMs = 1000 * speechGetDuration();
            if (voiceoverDurationMs > estimatedTotalPanTime / 2) {
                baseFrameDelay = (voiceoverDurationMs + baseFrameDelay * (totalPanDistance / 2)) / totalPanDistance;
            }
        }

        int currentPanPosition;
        int targetPanPosition;

        // Set panning direction (left to right or right to left)
        if (direction == -1) {
            currentPanPosition = artWidth - screenWidth; // Start from right
            targetPanPosition = 0; // End at left
        } else {
            currentPanPosition = 0; // Start from left
            targetPanPosition = artWidth - screenWidth; // End at right
        }

        tickersDisable();

        bool subtitlesLoaded = false;
        unsigned int lastFrameTime = 0;

        // Main panning loop
        while (currentPanPosition != targetPanPosition) {
            sharedFpsLimiter.mark();

            int subtitleTriggerPosition = screenWidth - fadeStartDistance;

            // Check if it's time to update the frame
            if (getTicksSince(lastFrameTime) >= baseFrameDelay) {
                // Render the current panning position
                blitBufferToBuffer(backgroundData + currentPanPosition, screenWidth, screenHeight, artWidth,
                    gEndgameEndingSlideshowWindowBuffer, screenWidth);

                // Update subtitles if they're loaded
                if (subtitlesLoaded) {
                    endgameEndingRefreshSubtitles();
                }

                windowRefresh(gEndgameEndingSlideshowWindow);
                lastFrameTime = getTicks();

                // Calculate palette intensity for fade effects
                bool shouldAdjustPalette;
                double intensityFactor;

                if (currentPanPosition > fadeStartDistance) {
                    if (subtitleTriggerPosition > currentPanPosition) {
                        shouldAdjustPalette = false;
                    } else {
                        int adjustedPosition = fadeStartDistance - (currentPanPosition - subtitleTriggerPosition);
                        intensityFactor = (double)adjustedPosition / (double)fadeStartDistance;
                        shouldAdjustPalette = true;
                    }
                } else {
                    shouldAdjustPalette = true;
                    intensityFactor = (double)currentPanPosition / (double)fadeStartDistance;
                }

                // Apply palette fade effect if needed
                if (shouldAdjustPalette) {
                    unsigned char darkenedPalette[768];
                    for (int index = 0; index < 768; index++) {
                        darkenedPalette[index] = (unsigned char)trunc(palette[index] * intensityFactor);
                    }
                    paletteSetEntries(darkenedPalette);
                }

                // Move to next panning position
                currentPanPosition += direction;

                // Trigger subtitle loading when reaching certain positions
                if (direction == 1 && (currentPanPosition == fadeStartDistance)) {
                    endgameEndingVoiceOverReset();
                    subtitlesLoaded = true;
                } else if (direction == -1 && (currentPanPosition == subtitleTriggerPosition)) {
                    endgameEndingVoiceOverReset();
                    subtitlesLoaded = true;
                }
            }

            soundContinueAll();

            // Check for user input to skip the scene
            if (inputGetInput() != -1) {
                endgameEndingVoiceOverFree();
                break;
            }

            renderPresent();
            sharedFpsLimiter.throttle();
        }

        tickersEnable();
        artUnlock(backgroundHandle);

        // Fade to black at the end of the scene
        paletteFadeTo(gPaletteBlack);
        bufferFill(gEndgameEndingSlideshowWindowBuffer, screenWidth, screenHeight, screenWidth, _colorTable[0]);
        windowRefresh(gEndgameEndingSlideshowWindow);
    }

    // Clear any pending mouse events
    while (mouseGetEvent() != 0) {
        sharedFpsLimiter.mark();
        inputGetInput();
        renderPresent();
        sharedFpsLimiter.throttle();
    }
}

// 0x440004
static void endgameEndingRenderStaticScene(int art_num, const char* narratorFileName)
{
    int fid = artGetFidWithVariant(OBJ_TYPE_INTERFACE, art_num, gameIsWidescreen());

    CacheEntry* backgroundHandle;
    Art* background = artLock(fid, &backgroundHandle);
    if (background == nullptr) {
        return;
    }

    int game_width, game_height;
    getScreenDimensions(&game_width, &game_height);

    unsigned char* backgroundData = artGetFrameData(background, 0, 0);
    if (backgroundData != nullptr) {
        blitBufferToBuffer(backgroundData, game_width, game_height, game_width, gEndgameEndingSlideshowWindowBuffer, game_width);
        windowRefresh(gEndgameEndingSlideshowWindow);

        endgameEndingLoadPalette(FID_TYPE(fid), art_num);

        // CE: Update overlay.
        endgameEndingUpdateOverlay();

        endgameEndingVoiceOverInit(narratorFileName);

        unsigned int delay;
        if (gEndgameEndingVoiceOverSubtitlesLoaded || gEndgameEndingVoiceOverSpeechLoaded) {
            delay = UINT_MAX;
        } else {
            delay = 3000;
        }

        paletteFadeTo(_cmap);

        inputPauseForTocks(500);

        // NOTE: Uninline.
        endgameEndingVoiceOverReset();

        unsigned int referenceTime = getTicks();
        tickersDisable();

        int keyCode;
        while (true) {
            sharedFpsLimiter.mark();

            keyCode = inputGetInput();
            if (keyCode != -1) {
                break;
            }

            if (gEndgameEndingSpeechEnded) {
                break;
            }

            if (gEndgameEndingSubtitlesEnded) {
                break;
            }

            if (getTicksSince(referenceTime) > delay) {
                break;
            }

            blitBufferToBuffer(backgroundData, game_width, game_height, game_width, gEndgameEndingSlideshowWindowBuffer, game_width);
            endgameEndingRefreshSubtitles();
            windowRefresh(gEndgameEndingSlideshowWindow);
            soundContinueAll();

            renderPresent();
            sharedFpsLimiter.throttle();
        }

        tickersEnable();
        speechDelete();
        endgameEndingSubtitlesFree();

        gEndgameEndingVoiceOverSpeechLoaded = false;
        gEndgameEndingVoiceOverSubtitlesLoaded = false;

        if (keyCode == -1) {
            inputPauseForTocks(500);
        }

        paletteFadeTo(gPaletteBlack);

        while (mouseGetEvent() != 0) {
            sharedFpsLimiter.mark();

            inputGetInput();

            renderPresent();
            sharedFpsLimiter.throttle();
        }
    }

    artUnlock(backgroundHandle);
}

// 0x43F99C
static int endgameEndingSlideshowWindowInit()
{
    if (endgameEndingInit() != 0) {
        return -1;
    }

    int game_width, game_height;
    getScreenDimensions(&game_width, &game_height);

    backgroundSoundDelete();

    _endgame_map_enabled = isoDisable();

    colorCycleDisable();
    gameMouseSetCursor(MOUSE_CURSOR_NONE);

    bool oldCursorIsHidden = cursorIsHidden();
    _endgame_mouse_state = oldCursorIsHidden == 0;

    if (oldCursorIsHidden) {
        mouseShowCursor();
    }

    gEndgameEndingSlideshowOldFont = fontGetCurrent();
    fontSetCurrent(101);

    paletteFadeTo(gPaletteBlack);

    // CE: Every slide has a separate color palette which is incompatible with
    // main color palette. Setup overlay to hide everything.
    gEndgameEndingOverlay = windowCreate(0, 0, screenGetWidth(), screenGetHeight(), _colorTable[0], WINDOW_MOVE_ON_TOP);
    if (gEndgameEndingOverlay == -1) {
        return -1;
    }

    int windowEndgameEndingX = (screenGetWidth() - game_width) / 2;
    int windowEndgameEndingY = (screenGetHeight() - game_height) / 2;
    gEndgameEndingSlideshowWindow = windowCreate(windowEndgameEndingX,
        windowEndgameEndingY,
        game_width,
        game_height,
        _colorTable[0],
        WINDOW_MOVE_ON_TOP);
    if (gEndgameEndingSlideshowWindow == -1) {
        return -1;
    }

    gEndgameEndingSlideshowWindowBuffer = windowGetBuffer(gEndgameEndingSlideshowWindow);
    if (gEndgameEndingSlideshowWindowBuffer == nullptr) {
        return -1;
    }

    colorCycleDisable();

    speechSetEndCallback(_endgame_voiceover_callback);

    gEndgameEndingSubtitlesEnabled = settings.preferences.subtitles;
    if (!gEndgameEndingSubtitlesEnabled) {
        return 0;
    }

    snprintf(gEndgameEndingSubtitlesLocalizedPath, sizeof(gEndgameEndingSubtitlesLocalizedPath), "text\\%s\\cuts\\", settings.system.language.c_str());

    gEndgameEndingSubtitles = (char**)internal_malloc(sizeof(*gEndgameEndingSubtitles) * ENDGAME_ENDING_MAX_SUBTITLES);
    if (gEndgameEndingSubtitles == nullptr) {
        gEndgameEndingSubtitlesEnabled = false;
        return 0;
    }

    for (int index = 0; index < ENDGAME_ENDING_MAX_SUBTITLES; index++) {
        gEndgameEndingSubtitles[index] = nullptr;
    }

    gEndgameEndingSubtitlesTimings = (unsigned int*)internal_malloc(sizeof(*gEndgameEndingSubtitlesTimings) * ENDGAME_ENDING_MAX_SUBTITLES);
    if (gEndgameEndingSubtitlesTimings == nullptr) {
        internal_free(gEndgameEndingSubtitles);
        gEndgameEndingSubtitlesEnabled = false;
        return 0;
    }

    return 0;
}

// 0x43FB28
static void endgameEndingSlideshowWindowFree()
{
    if (gEndgameEndingSubtitlesEnabled) {
        endgameEndingSubtitlesFree();

        internal_free(gEndgameEndingSubtitlesTimings);
        internal_free(gEndgameEndingSubtitles);

        gEndgameEndingSubtitles = nullptr;
        gEndgameEndingSubtitlesEnabled = false;
    }

    // NOTE: Uninline.
    endgameEndingFree();

    fontSetCurrent(gEndgameEndingSlideshowOldFont);

    speechSetEndCallback(nullptr);
    windowDestroy(gEndgameEndingSlideshowWindow);
    windowDestroy(gEndgameEndingOverlay);

    if (!_endgame_mouse_state) {
        mouseHideCursor();
    }

    gameMouseSetCursor(MOUSE_CURSOR_ARROW);

    colorPaletteLoad("color.pal");
    // paletteFadeTo(_cmap); // cutting this prevents the momentary return to the location map

    colorCycleEnable();

    if (_endgame_map_enabled) {
        isoEnable();
    }
}

// 0x4401A0
static void endgameEndingVoiceOverInit(const char* fileBaseName)
{
    char path[COMPAT_MAX_PATH];

    // NOTE: Uninline.
    endgameEndingVoiceOverFree();

    gEndgameEndingVoiceOverSpeechLoaded = false;
    gEndgameEndingVoiceOverSubtitlesLoaded = false;

    const char* language = settings.system.language.c_str();
    int speechFileSize;
    bool speechFound = false;

    // Added localized language paths to allow localized narrations
    if (compat_stricmp(language, ENGLISH) != 0) {
        snprintf(path, sizeof(path), "narrator\\%s\\%s", language, fileBaseName);
        speechFound = true;
    }

    // If localized not found or English language
    if (!speechFound) {
        snprintf(path, sizeof(path), "narrator\\%s", fileBaseName);
    }

    if (speechLoad(path, 10, 14, 15) != -1) {
        gEndgameEndingVoiceOverSpeechLoaded = true;
    }

    if (gEndgameEndingSubtitlesEnabled) {
        // Build subtitles file path.
        snprintf(path, sizeof(path), "%s%s.txt", gEndgameEndingSubtitlesLocalizedPath, fileBaseName);

        if (endgameEndingSubtitlesLoad(path) != 0) {
            return;
        }

        double durationPerCharacter;
        if (gEndgameEndingVoiceOverSpeechLoaded) {
            durationPerCharacter = (double)speechGetDuration() / (double)gEndgameEndingSubtitlesCharactersCount;
        } else {
            durationPerCharacter = 0.08;
        }

        unsigned int timing = 0;
        for (int index = 0; index < gEndgameEndingSubtitlesLength; index++) {
            double charactersCount = static_cast<double>(strlen(gEndgameEndingSubtitles[index]));
            // NOTE: There is floating point math at 0x4402E6 used to add
            // timing.
            timing += (unsigned int)trunc(charactersCount * durationPerCharacter * 1000.0);
            gEndgameEndingSubtitlesTimings[index] = timing;
        }

        gEndgameEndingVoiceOverSubtitlesLoaded = true;
    }
}

// NOTE: This function was inlined at every call site.
//
// 0x440324
static void endgameEndingVoiceOverReset()
{
    gEndgameEndingSubtitlesEnded = false;
    gEndgameEndingSpeechEnded = false;

    if (gEndgameEndingVoiceOverSpeechLoaded) {
        _gsound_speech_play_preloaded();
    }

    if (gEndgameEndingVoiceOverSubtitlesLoaded) {
        gEndgameEndingSubtitlesReferenceTime = getTicks();
    }
}

// NOTE: This function was inlined at every call site.
//
// 0x44035C
static void endgameEndingVoiceOverFree()
{
    speechDelete();
    endgameEndingSubtitlesFree();
    gEndgameEndingVoiceOverSpeechLoaded = false;
    gEndgameEndingVoiceOverSubtitlesLoaded = false;
}

// 0x440378
static void endgameEndingLoadPalette(int type, int id)
{
    char fileName[13];
    if (artCopyFileName(type, id, fileName) != 0) {
        return;
    }

    // Remove extension from file name.
    char* pch = strrchr(fileName, '.');
    if (pch != nullptr) {
        *pch = '\0';
    }

    if (strlen(fileName) <= 8) {
        char path[COMPAT_MAX_PATH];
        snprintf(path, sizeof(path), "%s\\%s.pal", "art\\intrface", fileName);
        colorPaletteLoad(path);
    }
}

// 0x4403F0
static void _endgame_voiceover_callback()
{
    gEndgameEndingSpeechEnded = true;
}

// Loads subtitles file.
//
// 0x4403FC
static int endgameEndingSubtitlesLoad(const char* filePath)
{
    endgameEndingSubtitlesFree();

    File* stream = fileOpen(filePath, "rt");
    if (stream == nullptr) {
        return -1;
    }

    // FIXME: There is at least one subtitle for Arroyo ending (nar_ar1) that
    // does not fit into this buffer.
    char string[256];
    while (fileReadString(string, sizeof(string), stream)) {
        char* pch;

        // Find and clamp string at EOL.
        pch = strchr(string, '\n');
        if (pch != nullptr) {
            *pch = '\0';
        }

        // Find separator. The value before separator is ignored (as opposed to
        // movie subtitles, where the value before separator is a timing).
        pch = strchr(string, ':');
        if (pch != nullptr) {
            if (gEndgameEndingSubtitlesLength < ENDGAME_ENDING_MAX_SUBTITLES) {
                gEndgameEndingSubtitles[gEndgameEndingSubtitlesLength] = internal_strdup(pch + 1);
                gEndgameEndingSubtitlesLength++;
                gEndgameEndingSubtitlesCharactersCount += static_cast<int>(strlen(pch + 1));
            }
        }
    }

    fileClose(stream);

    return 0;
}

// Refreshes subtitles.
//
// 0x4404EC
static void endgameEndingRefreshSubtitles()
{
    int game_width, game_height;
    getScreenDimensions(&game_width, &game_height);

    if (gEndgameEndingSubtitlesLength <= gEndgameEndingSubtitlesCurrentLine) {
        if (gEndgameEndingVoiceOverSubtitlesLoaded) {
            gEndgameEndingSubtitlesEnded = true;
        }
        return;
    }

    if (getTicksSince(gEndgameEndingSubtitlesReferenceTime) > gEndgameEndingSubtitlesTimings[gEndgameEndingSubtitlesCurrentLine]) {
        gEndgameEndingSubtitlesCurrentLine++;
        return;
    }

    char* text = gEndgameEndingSubtitles[gEndgameEndingSubtitlesCurrentLine];
    if (text == nullptr) {
        return;
    }

    short beginnings[WORD_WRAP_MAX_COUNT];
    short count;
    if (wordWrap(text, game_width - 100, beginnings, &count) != 0) {
        return;
    }

    int height = fontGetLineHeight();
    int y = game_height - height * count;

    for (int index = 0; index < count - 1; index++) {
        char* beginning = text + beginnings[index];
        char* ending = text + beginnings[index + 1];

        if (ending[-1] == ' ') {
            ending--;
        }

        char c = *ending;
        *ending = '\0';

        int width = fontGetStringWidth(beginning);
        int x = (game_width - width) / 2;
        bufferFill(gEndgameEndingSlideshowWindowBuffer + game_width * y + x, width, height, game_width, _colorTable[0]);
        fontDrawText(gEndgameEndingSlideshowWindowBuffer + game_width * y + x, beginning, width, game_width, _colorTable[32767]);

        *ending = c;

        y += height;
    }
}

// 0x4406CC
static void endgameEndingSubtitlesFree()
{
    for (int index = 0; index < gEndgameEndingSubtitlesLength; index++) {
        if (gEndgameEndingSubtitles[index] != nullptr) {
            internal_free(gEndgameEndingSubtitles[index]);
            gEndgameEndingSubtitles[index] = nullptr;
        }
    }

    gEndgameEndingSubtitlesCurrentLine = 0;
    gEndgameEndingSubtitlesCharactersCount = 0;
    gEndgameEndingSubtitlesLength = 0;
}

// 0x440728
static void _endgame_movie_callback()
{
    _endgame_maybe_done = 1;
}

// 0x440734
static void _endgame_movie_bk_process()
{
    if (_endgame_maybe_done) {
        backgroundSoundLoad("10labone", 11, 14, 16);
        backgroundSoundSetEndCallback(nullptr);
        tickersRemove(_endgame_movie_bk_process);
    }
}

static int parseEndgameFile(const char* path, const char* sourceName)
{
    const char* delim = " \t,";
    File* stream = fileOpen(path, "rt");
    if (!stream) return -1;

    char str[256];
    int localCount = 0;
    while (fileReadString(str, sizeof(str), stream)) {
        char* ch = str;
        while (isspace(*ch))
            ch++;
        if (*ch == '#' || *ch == '\0') continue;

        char* tok = strtok(ch, delim);
        if (!tok) continue;
        int gvar = atoi(tok);

        tok = strtok(nullptr, delim);
        if (!tok) continue;
        int value = atoi(tok);

        tok = strtok(nullptr, delim);
        if (!tok) continue;
        int art_num = atoi(tok);

        tok = strtok(nullptr, delim);
        if (!tok) continue;
        char voiceOverBaseName[64]; // adjust size as needed
        strncpy(voiceOverBaseName, tok, sizeof(voiceOverBaseName) - 1);
        voiceOverBaseName[sizeof(voiceOverBaseName) - 1] = '\0';
        // Trim trailing whitespace (like original)
        size_t len = strlen(voiceOverBaseName);
        while (len > 0 && isspace((unsigned char)voiceOverBaseName[len - 1])) {
            voiceOverBaseName[--len] = '\0';
        }

        int direction = 0;
        tok = strtok(nullptr, delim);
        if (tok) {
            direction = atoi(tok);
        }

        // Reallocate global array
        EndgameEnding* entries = (EndgameEnding*)internal_realloc(gEndgameEndings,
            sizeof(*entries) * (gEndgameEndingsLength + 1));
        if (!entries) {
            fileClose(stream);
            return -1;
        }
        gEndgameEndings = entries;

        EndgameEnding* entry = &gEndgameEndings[gEndgameEndingsLength];
        entry->gvar = gvar;
        entry->value = value;
        entry->art_num = art_num;
        strcpy(entry->voiceOverBaseName, voiceOverBaseName);
        entry->direction = direction;

        gEndgameEndingsLength++;
        localCount++;
    }
    fileClose(stream);
    debugPrint("Loaded %d endgame entries from %s\n", localCount, sourceName);
    return 0;
}

// 0x440770
static int endgameEndingInit()
{
    // Free any previously loaded entries
    if (gEndgameEndings) {
        internal_free(gEndgameEndings);
        gEndgameEndings = nullptr;
        gEndgameEndingsLength = 0;
    }

    // Load vanilla (TC) endgame.txt
    parseEndgameFile("data\\endgame.txt", "vanilla");

    // Find and load endgame mod files (endgame_*.txt)
    char searchPattern[COMPAT_MAX_PATH];
    snprintf(searchPattern, sizeof(searchPattern),
        "%sdata%cendgame_*.txt",
        _cd_path_base, DIR_SEPARATOR);

    char** foundFiles = nullptr;
    int fileCount = fileNameListInit(searchPattern, &foundFiles, 0, 0);
    if (fileCount > 0) {
        // Sort alphabetically for consistent load order
        for (int i = 0; i < fileCount - 1; i++) {
            for (int j = i + 1; j < fileCount; j++) {
                if (strcmp(foundFiles[i], foundFiles[j]) > 0) {
                    char* temp = foundFiles[i];
                    foundFiles[i] = foundFiles[j];
                    foundFiles[j] = temp;
                }
            }
        }

        for (int i = 0; i < fileCount; i++) {
            // Skip the vanilla file (just in case...)
            if (strcmp(foundFiles[i], "endgame.txt") == 0)
                continue;

            char filePath[COMPAT_MAX_PATH];
            snprintf(filePath, sizeof(filePath), "%sdata%c%s",
                _cd_path_base, DIR_SEPARATOR, foundFiles[i]);

            parseEndgameFile(filePath, foundFiles[i]);
        }
        fileNameListFree(&foundFiles, fileCount);
    }

    // Not really needed, but keep for now
    generateEndgameReport();

    return 0;
}

// NOTE: There are no references to this function. It was inlined.
//
// 0x44095C
static void endgameEndingFree()
{
    if (gEndgameEndings != nullptr) {
        internal_free(gEndgameEndings);
        gEndgameEndings = nullptr;
    }

    gEndgameEndingsLength = 0;
}

static int parseEnddeathFile(const char* path, const char* sourceName)
{
    const char* delim = " \t,";
    File* stream = fileOpen(path, "rt");
    if (!stream) return -1;

    char str[256];
    int localCount = 0;
    while (fileReadString(str, sizeof(str), stream)) {
        char* ch = str;
        while (isspace(*ch))
            ch++;
        if (*ch == '#' || *ch == '\0') continue;

        char* tok = strtok(ch, delim);
        if (!tok) continue;
        int gvar = atoi(tok);

        tok = strtok(nullptr, delim);
        if (!tok) continue;
        int value = atoi(tok);

        tok = strtok(nullptr, delim);
        if (!tok) continue;
        int worldAreaKnown = atoi(tok);

        tok = strtok(nullptr, delim);
        if (!tok) continue;
        int worldAreaNotKnown = atoi(tok);

        tok = strtok(nullptr, delim);
        if (!tok) continue;
        int min_level = atoi(tok);

        tok = strtok(nullptr, delim);
        if (!tok) continue;
        int percentage = atoi(tok);

        tok = strtok(nullptr, delim);
        if (!tok) continue;

        // voiceOverBaseName
        char voiceOverBaseName[64]; // adjust size as needed
        strncpy(voiceOverBaseName, tok, sizeof(voiceOverBaseName) - 1);
        voiceOverBaseName[sizeof(voiceOverBaseName) - 1] = '\0';
        // Trim trailing whitespace (like original code)
        size_t len = strlen(voiceOverBaseName);
        while (len > 0 && isspace((unsigned char)voiceOverBaseName[len - 1])) {
            voiceOverBaseName[--len] = '\0';
        }

        // Reallocate global array
        EndgameDeathEnding* entries = (EndgameDeathEnding*)internal_realloc(gEndgameDeathEndings,
            sizeof(*entries) * (gEndgameDeathEndingsLength + 1));
        if (!entries) {
            fileClose(stream);
            return -1;
        }
        gEndgameDeathEndings = entries;

        EndgameDeathEnding* entry = &gEndgameDeathEndings[gEndgameDeathEndingsLength];
        entry->gvar = gvar;
        entry->value = value;
        entry->worldAreaKnown = worldAreaKnown;
        entry->worldAreaNotKnown = worldAreaNotKnown;
        entry->min_level = min_level;
        entry->percentage = percentage;
        strcpy(entry->voiceOverBaseName, voiceOverBaseName);
        entry->enabled = false; // matches original init

        gEndgameDeathEndingsLength++;
        localCount++;
    }
    fileClose(stream);
    debugPrint("Loaded %d death endings from %s\n", localCount, sourceName);
    return 0;
}

// endgameDeathEndingInit
// 0x440984
int endgameDeathEndingInit()
{
    strcpy(gEndgameDeathEndingFileName, "narrator\\nar_5");

    // Free any previously loaded entries
    if (gEndgameDeathEndings) {
        internal_free(gEndgameDeathEndings);
        gEndgameDeathEndings = nullptr;
        gEndgameDeathEndingsLength = 0;
    }

    // Load vanilla enddeath.txt
    parseEnddeathFile("data\\enddeath.txt", "vanilla");

    // Find and load enddeath mod files (enddeath_*.txt etc.)
    char searchPattern[COMPAT_MAX_PATH];
    snprintf(searchPattern, sizeof(searchPattern),
        "%sdata%cenddeath_*.txt",
        _cd_path_base, DIR_SEPARATOR);

    char** foundFiles = nullptr;
    int fileCount = fileNameListInit(searchPattern, &foundFiles, 0, 0);
    if (fileCount > 0) {
        // Sort alphabetically for consistent load order
        for (int i = 0; i < fileCount - 1; i++) {
            for (int j = i + 1; j < fileCount; j++) {
                if (strcmp(foundFiles[i], foundFiles[j]) > 0) {
                    char* temp = foundFiles[i];
                    foundFiles[i] = foundFiles[j];
                    foundFiles[j] = temp;
                }
            }
        }

        for (int i = 0; i < fileCount; i++) {
            // Skip the vanilla file
            if (strcmp(foundFiles[i], "enddeath.txt") == 0)
                continue;

            char filePath[COMPAT_MAX_PATH];
            snprintf(filePath, sizeof(filePath), "%sdata%c%s",
                _cd_path_base, DIR_SEPARATOR, foundFiles[i]);

            parseEnddeathFile(filePath, foundFiles[i]);
        }
        fileNameListFree(&foundFiles, fileCount);
    }

    // Generate a report
    generateEnddeathReport();

    return 0;
}

// 0x440BA8
int endgameDeathEndingExit()
{
    if (gEndgameDeathEndings != nullptr) {
        internal_free(gEndgameDeathEndings);
        gEndgameDeathEndings = nullptr;

        gEndgameDeathEndingsLength = 0;
    }

    return 0;
}

// endgameSetupDeathEnding
// 0x440BD0
void endgameSetupDeathEnding(int reason)
{
    if (!gEndgameDeathEndingsLength) {
        debugPrint("\nError: endgameSetupDeathEnding: No endgame death info!");
        return;
    }

    // Build death ending file path.
    const char* language = settings.system.language.c_str();

    if (compat_stricmp(language, ENGLISH) != 0) {
        // Localized narrator folder
        strcpy(gEndgameDeathEndingFileName, "narrator\\");
        strcat(gEndgameDeathEndingFileName, language);
        strcat(gEndgameDeathEndingFileName, "\\");
    } else {
        // Default narrator folder
        strcpy(gEndgameDeathEndingFileName, "narrator\\");
    }

    int percentage = 0;
    endgameDeathEndingValidate(&percentage);

    int selectedEnding = 0;
    bool specialEndingSelected = false;

    switch (reason) {
    case ENDGAME_DEATH_ENDING_REASON_DEATH:
        if (gameGetGlobalVar(GVAR_MODOC_SHITTY_DEATH) != 0) {
            selectedEnding = 12;
            specialEndingSelected = true;
        }
        break;
    case ENDGAME_DEATH_ENDING_REASON_TIMEOUT:
        gameMoviePlay(MOVIE_TIMEOUT, GAME_MOVIE_FADE_IN | GAME_MOVIE_FADE_OUT | GAME_MOVIE_PAUSE_MUSIC);
        break;
    }

    if (!specialEndingSelected) {
        int chance = randomBetween(0, percentage);

        int accum = 0;
        for (int index = 0; index < gEndgameDeathEndingsLength; index++) {
            EndgameDeathEnding* deathEnding = &(gEndgameDeathEndings[index]);

            if (deathEnding->enabled) {
                accum += deathEnding->percentage;
                if (accum >= chance) {
                    break;
                }
                selectedEnding++;
            }
        }
    }

    EndgameDeathEnding* deathEnding = &(gEndgameDeathEndings[selectedEnding]);

    strcat(gEndgameDeathEndingFileName, deathEnding->voiceOverBaseName);

    debugPrint("\nendgameSetupDeathEnding: Death Filename Picked: %s", gEndgameDeathEndingFileName);
}

// Validates conditions imposed by death endings.
//
// Upon return [percentage] is set as a sum of all valid endings' percentages.
// Always returns 0.
//
// 0x440CF4
static int endgameDeathEndingValidate(int* percentage)
{
    *percentage = 0;

    for (int index = 0; index < gEndgameDeathEndingsLength; index++) {
        EndgameDeathEnding* deathEnding = &(gEndgameDeathEndings[index]);

        deathEnding->enabled = false;

        if (deathEnding->gvar != -1) {
            if (gameGetGlobalVar(deathEnding->gvar) >= deathEnding->value) {
                continue;
            }
        }

        if (deathEnding->worldAreaKnown != -1) {
            if (!wmAreaIsKnown(deathEnding->worldAreaKnown)) {
                continue;
            }
        }

        if (deathEnding->worldAreaNotKnown != -1) {
            if (wmAreaIsKnown(deathEnding->worldAreaNotKnown)) {
                continue;
            }
        }

        if (pcGetStat(PC_STAT_LEVEL) < deathEnding->min_level) {
            continue;
        }

        deathEnding->enabled = true;

        *percentage += deathEnding->percentage;
    }

    return 0;
}

// Returns file name for voice over for death ending.
//
// This path does not include extension.
//
// 0x440D8C
char* endgameDeathEndingGetFileName()
{
    if (gEndgameDeathEndingsLength == 0) {
        debugPrint("\nError: endgameSetupDeathEnding: No endgame death info!");
        strcpy(gEndgameDeathEndingFileName, "narrator\\nar_4");
    }

    debugPrint("\nendgameSetupDeathEnding: Death Filename: %s", gEndgameDeathEndingFileName);

    return gEndgameDeathEndingFileName;
}

void endgameEndingUpdateOverlay()
{
    bufferFill(windowGetBuffer(gEndgameEndingOverlay),
        windowGetWidth(gEndgameEndingOverlay),
        windowGetHeight(gEndgameEndingOverlay),
        windowGetWidth(gEndgameEndingOverlay),
        intensityColorTable[_colorTable[0]][0]);
    windowRefresh(gEndgameEndingOverlay);
}

static void generateEnddeathReport()
{
    char dirPath[COMPAT_MAX_PATH];
    snprintf(dirPath, sizeof(dirPath), "%sdata%clists", _cd_path_base, DIR_SEPARATOR);
    compat_mkdir(dirPath);

    char reportPath[COMPAT_MAX_PATH];
    snprintf(reportPath, sizeof(reportPath), "%sdata%clists%cenddeath_list.txt",
        _cd_path_base, DIR_SEPARATOR, DIR_SEPARATOR);

    FILE* report = compat_fopen(reportPath, "wt");
    if (!report) {
        debugPrint("Failed to create enddeath report\n");
        return;
    }

    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    fprintf(report, "Death Endings Report\n");
    fprintf(report, "Generated: %04d-%02d-%02d %02d:%02d:%02d\n\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);
    fprintf(report, "Total entries: %d\n\n", gEndgameDeathEndingsLength);

    fprintf(report, "%-6s %-8s %-8s %-8s %-8s %-8s %-6s %-20s\n",
        "Index", "GVAR", "Value", "Known", "NotKnown", "MinLvl", "Pct", "VoiceOver");
    for (int i = 0; i < gEndgameDeathEndingsLength; i++) {
        EndgameDeathEnding* e = &gEndgameDeathEndings[i];
        fprintf(report, "%-6d %-8d %-8d %-8d %-8d %-8d %-6d %-20s\n",
            i, e->gvar, e->value, e->worldAreaKnown, e->worldAreaNotKnown,
            e->min_level, e->percentage, e->voiceOverBaseName);
    }
    fclose(report);
    debugPrint("Death endings report written to %s\n", reportPath);
}

// Just mimic karma report, but this could be cut - only runs at end game.
static void generateEndgameReport()
{
    char dirPath[COMPAT_MAX_PATH];
    snprintf(dirPath, sizeof(dirPath), "%sdata%clists", _cd_path_base, DIR_SEPARATOR);
    compat_mkdir(dirPath);

    char reportPath[COMPAT_MAX_PATH];
    snprintf(reportPath, sizeof(reportPath), "%sdata%clists%cendgame_list.txt",
        _cd_path_base, DIR_SEPARATOR, DIR_SEPARATOR);

    FILE* report = compat_fopen(reportPath, "wt");
    if (!report) {
        debugPrint("Failed to create endgame report\n");
        return;
    }

    time_t now = time(nullptr);
    struct tm* t = localtime(&now);
    fprintf(report, "Endgame Slides Report\n");
    fprintf(report, "Generated: %04d-%02d-%02d %02d:%02d:%02d\n\n",
        t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
        t->tm_hour, t->tm_min, t->tm_sec);
    fprintf(report, "Total entries: %d\n\n", gEndgameEndingsLength);

    fprintf(report, "%-6s %-8s %-8s %-8s %-20s %-8s\n",
        "Index", "GVAR", "Value", "Art", "VoiceOverBase", "Direction");
    for (int i = 0; i < gEndgameEndingsLength; i++) {
        EndgameEnding* e = &gEndgameEndings[i];
        fprintf(report, "%-6d %-8d %-8d %-8d %-20s %-8d\n",
            i, e->gvar, e->value, e->art_num,
            e->voiceOverBaseName, e->direction);
    }
    fclose(report);
    debugPrint("Endgame report written to %s\n", reportPath);
}

} // namespace fallout
