#ifndef GAME_MOVIE_H
#define GAME_MOVIE_H

#include "db.h"

namespace fallout {

typedef enum GameMovieFlags {
    GAME_MOVIE_FADE_IN = 0x01,
    GAME_MOVIE_FADE_OUT = 0x02,
    GAME_MOVIE_STOP_MUSIC = 0x04,
    GAME_MOVIE_PAUSE_MUSIC = 0x08,
} GameMovieFlags;

// Maximum number of movies tracked. Sized for Fallout 2's list (17);
// Fallout 1 uses only the first 14 slots. Fixed so the seen bitmap in
// save files stays the same size across versions.
#define MOVIE_COUNT 17

// Runtime movie identifiers. Values differ between Fallout 1 and Fallout 2
// because the two games number their movies differently. Populated by
// [gameMoviesInit] based on the loaded game version.
//
// A value of -1 means "no equivalent in this game." gameMoviePlay silently
// no-ops, and gameMovieIsSeen returns false.
extern int gMovieIplogo;
extern int gMovieIntro;
extern int gMovieElder;
extern int gMovieVsuit;
extern int gMovieAfailed;
extern int gMovieAdestroy;
extern int gMovieCar;
extern int gMovieCartucci;
extern int gMovieTimeout;
extern int gMovieTanker;
extern int gMovieEnclave;
extern int gMovieDerrick;
extern int gMovieArtimer1;
extern int gMovieArtimer2;
extern int gMovieArtimer3;
extern int gMovieArtimer4;
extern int gMovieCredits;

// The briefing that plays after character creation, when the player
// commits to starting a new game. Fallout 2 plays the elder; Fallout 1
// plays the overseer intro. -1 means no briefing in this game.
extern int gMovieNewGameBriefing;

// F1-only: the Vault Dweller walking away from the Overseer. -1 in F2
// mode, where F2's endgame flow uses a different set of movies.
extern int gMovieWalkm;
extern int gMovieWalkw;

// F1-only: the three Vault 13 water-supply status movies. Triggered by the
// midnight tick as the water level crosses 100/50/0. -1 in F2 mode, where
// the water-chip plot does not exist.
extern int gMovieBoil1;
extern int gMovieBoil2;
extern int gMovieBoil3;

int gameMoviesInit();
void gameMoviesReset();
int gameMoviesLoad(File* stream);
int gameMoviesSave(File* stream);
int gameMoviePlay(int movie, int flags);
void gameMovieFadeOut();
bool gameMovieIsSeen(int movie);
bool gameMovieIsPlaying();

} // namespace fallout

#endif /* GAME_MOVIE_H */