#include "game_movie.h"

#include <stdio.h>
#include <string.h>

#include "color.h"
#include "cycle.h"
#include "debug.h"
#include "game.h"
#include "game_mouse.h"
#include "game_sound.h"
#include "game_version.h"
#include "input.h"
#include "mouse.h"
#include "movie.h"
#include "movie_effect.h"
#include "palette.h"
#include "platform_compat.h"
#include "settings.h"
#include "svga.h"
#include "text_font.h"
#include "touch.h"
#include "window_manager.h"

namespace fallout {

#define GAME_MOVIE_WINDOW_WIDTH 640
#define GAME_MOVIE_WINDOW_HEIGHT 480

static char* gameMovieBuildSubtitlesFilePath(char* movieFilePath);

// 0x50352A
static const float flt_50352A = 0.032258064f;

// Defaults match Fallout 2 (FISSION's native behavior). gameMoviesInit
// overrides them for Fallout 1.
int gMovieIplogo   = 0;
int gMovieIntro    = 1;
int gMovieElder    = 2;
int gMovieVsuit    = 3;
int gMovieAfailed  = 4;
int gMovieAdestroy = 5;
int gMovieCar      = 6;
int gMovieCartucci = 7;
int gMovieTimeout  = 8;
int gMovieTanker   = 9;
int gMovieEnclave  = 10;
int gMovieDerrick  = 11;
int gMovieArtimer1 = 12;
int gMovieArtimer2 = 13;
int gMovieArtimer3 = 14;
int gMovieArtimer4 = 15;
int gMovieCredits  = 16;
int gMovieNewGameBriefing = 2;   // F2 default: elder

// 0x518DA0
// Populated by gameMoviesInit. In F1 mode these hold F1's list; in F2 mode
// F2's. Slot count is always MOVIE_COUNT so the seen bitmap in save files
// stays the same shape.
static const char* gMovieFileNames[MOVIE_COUNT];

// Subtitle palettes, same indexing as gMovieFileNames. nullptr means
// fall back to the default subtitle palette.
static const char* gMoviePaletteFilePaths[MOVIE_COUNT];

// 0x518E28
static bool gGameMovieIsPlaying = false;

// 0x518E2C
static bool gGameMovieFaded = false;

// 0x596C78
static unsigned char gGameMoviesSeen[MOVIE_COUNT];

// 0x596C89
static char gGameMovieSubtitlesFilePath[COMPAT_MAX_PATH];

// gmovie_init
// 0x44E5C0
int gameMoviesInit()
{
    int volume = 0;
    if (backgroundSoundIsEnabled()) {
        volume = backgroundSoundGetVolume();
    }

    movieSetVolume(volume);
    movieSetBuildSubtitleFilePathProc(gameMovieBuildSubtitlesFilePath);

    // Populate movie tables for the loaded game version.
    if (IS_FALLOUT_1()) {
        // Fallout 1's list, at F1's native indices.
        gMovieFileNames[0]  = "iplogo.mve";
        gMovieFileNames[1]  = "mplogo.mve";
        gMovieFileNames[2]  = "intro.mve";
        gMovieFileNames[3]  = "vexpld.mve";
        gMovieFileNames[4]  = "cathexp.mve";
        gMovieFileNames[5]  = "ovrintro.mve";
        gMovieFileNames[6]  = "boil3.mve";
        gMovieFileNames[7]  = "ovrrun.mve";
        gMovieFileNames[8]  = "walkm.mve";
        gMovieFileNames[9]  = "walkw.mve";
        gMovieFileNames[10] = "dipedv.mve";
        gMovieFileNames[11] = "boil1.mve";
        gMovieFileNames[12] = "boil2.mve";
        gMovieFileNames[13] = "raekills.mve";
        // F1 has no equivalent of F2's movies; leave slots 14-16 null.
        for (int i = 14; i < MOVIE_COUNT; i++) {
            gMovieFileNames[i] = nullptr;
        }

        // F1 subtitle palettes. Known values are filled in; unknown ones
        // fall back to the default subtitle.pal. Can be tuned later.
        for (int i = 0; i < MOVIE_COUNT; i++) {
            gMoviePaletteFilePaths[i] = nullptr;
        }
        // F1 has no elder/artimer/crdtssub palettes; those are F2-specific.

        // Re-point the semantic identifiers to F1's indices.
        gMovieIplogo   = 0;
        gMovieIntro    = 2;
        gMovieNewGameBriefing    = 5;   // F1 has overseer briefing
        gMovieVsuit    = -1;   // F1 has no vault-suit cutscene
        gMovieAfailed  = -1;
        gMovieAdestroy = -1;
        gMovieCar      = -1;
        gMovieCartucci = -1;
        gMovieTimeout  = -1;
        gMovieTanker   = -1;
        gMovieEnclave  = -1;
        gMovieDerrick  = -1;
        gMovieArtimer1 = -1;
        gMovieArtimer2 = -1;
        gMovieArtimer3 = -1;
        gMovieArtimer4 = -1;
        gMovieCredits  = -1;   // F1 credits are text-only

    } else {
        // Fallout 2 (unchanged from original).
        gMovieFileNames[0]  = "iplogo.mve";
        gMovieFileNames[1]  = "intro.mve";
        gMovieFileNames[2]  = "elder.mve";
        gMovieFileNames[3]  = "vsuit.mve";
        gMovieFileNames[4]  = "afailed.mve";
        gMovieFileNames[5]  = "adestroy.mve";
        gMovieFileNames[6]  = "car.mve";
        gMovieFileNames[7]  = "cartucci.mve";
        gMovieFileNames[8]  = "timeout.mve";
        gMovieFileNames[9]  = "tanker.mve";
        gMovieFileNames[10] = "enclave.mve";
        gMovieFileNames[11] = "derrick.mve";
        gMovieFileNames[12] = "artimer1.mve";
        gMovieFileNames[13] = "artimer2.mve";
        gMovieFileNames[14] = "artimer3.mve";
        gMovieFileNames[15] = "artimer4.mve";
        gMovieFileNames[16] = "credits.mve";

        gMoviePaletteFilePaths[0]  = nullptr;
        gMoviePaletteFilePaths[1]  = "art\\cuts\\introsub.pal";
        gMoviePaletteFilePaths[2]  = "art\\cuts\\eldersub.pal";
        gMoviePaletteFilePaths[3]  = nullptr;
        gMoviePaletteFilePaths[4]  = "art\\cuts\\artmrsub.pal";
        gMoviePaletteFilePaths[5]  = nullptr;
        gMoviePaletteFilePaths[6]  = nullptr;
        gMoviePaletteFilePaths[7]  = nullptr;
        gMoviePaletteFilePaths[8]  = "art\\cuts\\artmrsub.pal";
        gMoviePaletteFilePaths[9]  = nullptr;
        gMoviePaletteFilePaths[10] = nullptr;
        gMoviePaletteFilePaths[11] = nullptr;
        gMoviePaletteFilePaths[12] = "art\\cuts\\artmrsub.pal";
        gMoviePaletteFilePaths[13] = "art\\cuts\\artmrsub.pal";
        gMoviePaletteFilePaths[14] = "art\\cuts\\artmrsub.pal";
        gMoviePaletteFilePaths[15] = "art\\cuts\\artmrsub.pal";
        gMoviePaletteFilePaths[16] = "art\\cuts\\crdtssub.pal";

        gMovieIplogo   = 0;
        gMovieIntro    = 1;
        gMovieNewGameBriefing    = 2;
        gMovieVsuit    = 3;
        gMovieAfailed  = 4;
        gMovieAdestroy = 5;
        gMovieCar      = 6;
        gMovieCartucci = 7;
        gMovieTimeout  = 8;
        gMovieTanker   = 9;
        gMovieEnclave  = 10;
        gMovieDerrick  = 11;
        gMovieArtimer1 = 12;
        gMovieArtimer2 = 13;
        gMovieArtimer3 = 14;
        gMovieArtimer4 = 15;
        gMovieCredits  = 16;
    }

    memset(gGameMoviesSeen, 0, sizeof(gGameMoviesSeen));

    gGameMovieIsPlaying = false;
    gGameMovieFaded = false;

    return 0;
}

// 0x44E60C
void gameMoviesReset()
{
    memset(gGameMoviesSeen, 0, sizeof(gGameMoviesSeen));

    gGameMovieIsPlaying = false;
    gGameMovieFaded = false;
}

// 0x44E638
int gameMoviesLoad(File* stream)
{
    if (fileRead(gGameMoviesSeen, sizeof(*gGameMoviesSeen), MOVIE_COUNT, stream) != MOVIE_COUNT) {
        return -1;
    }

    return 0;
}

// 0x44E664
int gameMoviesSave(File* stream)
{
    if (fileWrite(gGameMoviesSeen, sizeof(*gGameMoviesSeen), MOVIE_COUNT, stream) != MOVIE_COUNT) {
        return -1;
    }

    return 0;
}

// gmovie_play
// 0x44E690
int gameMoviePlay(int movie, int flags)
{
    if (movie < 0 || movie >= MOVIE_COUNT) {
        return 0;   // Not present in this game.
    }

    const char* movieFileName = gMovieFileNames[movie];
    if (movieFileName == nullptr) {
        return 0;   // Same.
    }

    gGameMovieIsPlaying = true;

    debugPrint("\nPlaying movie: %s\n", movieFileName);

    const char* language = settings.system.language.c_str();
    char movieFilePath[COMPAT_MAX_PATH];
    int movieFileSize;
    bool movieFound = false;

    if (compat_stricmp(language, ENGLISH) != 0) {
        snprintf(movieFilePath, sizeof(movieFilePath), "art\\%s\\cuts\\%s", language, gMovieFileNames[movie]);
        movieFound = dbGetFileSize(movieFilePath, &movieFileSize) == 0;
    }

    if (!movieFound) {
        snprintf(movieFilePath, sizeof(movieFilePath), "art\\cuts\\%s", gMovieFileNames[movie]);
        movieFound = dbGetFileSize(movieFilePath, &movieFileSize) == 0;
    }

    if (!movieFound) {
        debugPrint("\ngmovie_play() - Error: Unable to open %s\n", gMovieFileNames[movie]);
        gGameMovieIsPlaying = false;
        return -1;
    }

    if ((flags & GAME_MOVIE_FADE_IN) != 0) {
        paletteFadeTo(gPaletteBlack);
        gGameMovieFaded = true;
    }

    // Must restore preference in case coming from Game area
    restoreUserAspectPreference();
    resizeContent(640, 480);

    int gameMovieWindowX = (screenGetWidth() - GAME_MOVIE_WINDOW_WIDTH) / 2;
    int gameMovieWindowY = (screenGetHeight() - GAME_MOVIE_WINDOW_HEIGHT) / 2;
    int win = windowCreate(gameMovieWindowX,
        gameMovieWindowY,
        GAME_MOVIE_WINDOW_WIDTH,
        GAME_MOVIE_WINDOW_HEIGHT,
        0,
        WINDOW_MODAL);
    if (win == -1) {
        gGameMovieIsPlaying = false;
        return -1;
    }

    if ((flags & GAME_MOVIE_STOP_MUSIC) != 0) {
        backgroundSoundDelete();
    } else if ((flags & GAME_MOVIE_PAUSE_MUSIC) != 0) {
        backgroundSoundPause();
    }

    windowRefresh(win);

    bool subtitlesEnabled = settings.preferences.subtitles;
    int movieFlags = 4;
    if (subtitlesEnabled) {
        char* subtitlesFilePath = gameMovieBuildSubtitlesFilePath(movieFilePath);

        int subtitlesFileSize;
        if (dbGetFileSize(subtitlesFilePath, &subtitlesFileSize) == 0) {
            movieFlags = 12;
        } else {
            subtitlesEnabled = false;
        }
    }

    movieSetFlags(movieFlags);

    int oldTextColor;
    int oldFont;
    if (subtitlesEnabled) {
        const char* subtitlesPaletteFilePath;
        if (gMoviePaletteFilePaths[movie] != nullptr) {
            subtitlesPaletteFilePath = gMoviePaletteFilePaths[movie];
        } else {
            subtitlesPaletteFilePath = "art\\cuts\\subtitle.pal";
        }

        colorPaletteLoad(subtitlesPaletteFilePath);

        oldTextColor = scriptWindowGetTextColor();
        scriptWindowSetTextColor(1.0, 1.0, 1.0);

        oldFont = fontGetCurrent();
        windowSetFont(101);
    }

    bool cursorWasHidden = cursorIsHidden();
    if (cursorWasHidden) {
        gameMouseSetCursor(MOUSE_CURSOR_NONE);
        mouseShowCursor();
    }

    while (mouseGetEvent() != 0) {
        _mouse_info();
    }

    mouseHideCursor();
    colorCycleDisable();

    movieEffectsLoad(movieFilePath);

    _zero_vid_mem();
    _movieRun(win, movieFilePath);

    int pressed = 0;
    int buttons;
    do {
        if (!_moviePlaying() || _game_user_wants_to_quit || inputGetInput() != -1) {
            break;
        }

        Gesture gesture;
        if (touch_get_gesture(&gesture) && gesture.state == kEnded) {
            break;
        }

        int x;
        int y;
        _mouse_get_raw_state(&x, &y, &buttons);

        pressed |= buttons;
        // Exit on mouse only after a click cycle: observe left/right down at
        // least once, then wait until both are released.
    } while (((pressed & 1) == 0 && (pressed & 2) == 0) || (buttons & 1) != 0 || (buttons & 2) != 0);

    _movieStop();
    _moviefx_stop();
    _movieUpdate();
    paletteSetEntries(gPaletteBlack);

    gGameMoviesSeen[movie] = 1;

    colorCycleEnable();

    gameMouseSetCursor(MOUSE_CURSOR_ARROW);

    if (!cursorWasHidden) {
        mouseShowCursor();
    }

    if (subtitlesEnabled) {
        colorPaletteLoad("color.pal");

        windowSetFont(oldFont);

        float r = (float)((Color2RGB(oldTextColor) & 0x7C00) >> 10) * flt_50352A;
        float g = (float)((Color2RGB(oldTextColor) & 0x3E0) >> 5) * flt_50352A;
        float b = (float)(Color2RGB(oldTextColor) & 0x1F) * flt_50352A;
        scriptWindowSetTextColor(r, g, b);
    }

    // movies in the pipboy or played from a map (temple of trials) need to be set to the actual maps size
    if (GameMode::isInGameMode(GameMode::kPipboy) || GameMode::isInGameMode(GameMode::kMap)) {
        resizeContent(screenGetWidth(), screenGetHeight(), true);
        // other movies play from outside game area - main, scripts(worldmap)
    } else {
        if (gameIsWidescreen()) {
            resizeContent(800, 500);
        } else {
            resizeContent(640, 480);
        }
    }
    windowDestroy(win);

    // CE: Destroying a window redraws only content it was covering (centered
    // 640x480). This leads to everything outside this rect to remain black.
    windowRefreshAll(&_scr_size);

    if ((flags & GAME_MOVIE_PAUSE_MUSIC) != 0) {
        backgroundSoundResume();
    }

    if ((flags & GAME_MOVIE_FADE_OUT) != 0) {
        if (!subtitlesEnabled) {
            colorPaletteLoad("color.pal");
        }

        if (!GameMode::isInGameMode(GameMode::kMap)) // Need to exclude the fade from the worldmap, otherwise we fade into it - the fade is handled at town entry.
        {
            paletteFadeTo(_cmap);
        }
        gGameMovieFaded = false;
    }

    gGameMovieIsPlaying = false;
    return 0;
}

// 0x44EAE4
void gameMovieFadeOut()
{
    if (gGameMovieFaded) {
        paletteFadeTo(_cmap);
        gGameMovieFaded = false;
    }
}

// 0x44EB04
bool gameMovieIsSeen(int movie)
{
    if (movie < 0 || movie >= MOVIE_COUNT) {
        return false;
    }
    return gGameMoviesSeen[movie] == 1;
}

// 0x44EB14
bool gameMovieIsPlaying()
{
    return gGameMovieIsPlaying;
}

// 0x44EB1C
static char* gameMovieBuildSubtitlesFilePath(char* movieFilePath)
{
    char* path = movieFilePath;

    char* separator = strrchr(path, '\\');
    if (separator != nullptr) {
        path = separator + 1;
    }

    snprintf(gGameMovieSubtitlesFilePath, sizeof(gGameMovieSubtitlesFilePath), "text\\%s\\cuts\\%s", settings.system.language.c_str(), path);

    char* pch = strrchr(gGameMovieSubtitlesFilePath, '.');
    if (*pch != '\0') {
        *pch = '\0';
    }

    strcpy(gGameMovieSubtitlesFilePath + strlen(gGameMovieSubtitlesFilePath), ".SVE");

    return gGameMovieSubtitlesFilePath;
}

} // namespace fallout
