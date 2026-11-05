/*
 * Dungeons of Daggorath on acsdl12: its window, frames, waits and the
 * drawer it runs from.
 *
 *  - The window is acsdl12's 8-bit screen at the game's own size (512 x
 *    384, twice the CoCo's 256 x 192, or 256 x 192 itself), in the middle
 *    of a frame a quarter larger (640 x 480 or 320 x 240), with the pointer
 *    hidden and held keys repeating, as SDL 2's do. The game draws
 *    through amiga/swgl.c; a swap copies that picture to the screen and
 *    shows it.
 *  - SDL_WaitEventTimeout (SDL 2) polls, sleeping a few milliseconds
 *    between looks.
 *  - Semaphores: the game posts one when a sound channel ends and never
 *    waits on it, so a count is enough.
 *  - The game opens conf/, sound/ and saved/ by relative names: PROGDIR:
 *    is made the current directory at start-up, and the one it was is
 *    given back at exit, so it runs from Workbench or any Shell.
 *
 * The AmiCradle Team. Like the game, under Douglas J. Morgan's licence.
 */
#include <stdlib.h>
#include <proto/dos.h>
#include "dod_amiga.h"

void swgl_resize(int w, int h);
void swgl_present(SDL_Surface *screen);

struct DOD_Window { int w, h; };
struct DOD_sem { volatile Uint32 count; };

static struct DOD_Window window;
static SDL_Surface *screen;

static void open_screen(int w, int h)
{
    ACSDL_SetMinimumFrame(w + w / 4, h + h / 4);
    screen = SDL_SetVideoMode(w, h, 8, SDL_HWPALETTE | SDL_FULLSCREEN);
    if (!screen)
        return;
    SDL_ShowCursor(SDL_DISABLE);
    /* SDL 2 repeats held keys by itself; SDL 1.2 only when asked */
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    window.w = w;
    window.h = h;
    swgl_resize(w, h);
}

SDL_Window *DOD_CreateWindow(int w, int h, Uint32 flags)
{
    (void)flags;
    open_screen(w, h);
    return screen ? &window : NULL;
}

void DOD_SetWindowSize(SDL_Window *win, int w, int h)
{
    (void)win;
    if (!screen || w != window.w || h != window.h)
        open_screen(w, h);
}

void DOD_SwapWindow(SDL_Window *win)
{
    (void)win;
    swgl_present(screen);
}

int DOD_WaitEventTimeout(SDL_Event *event, int ms)
{
    Uint32 start = SDL_GetTicks();
    for (;;) {
        Uint32 gone;
        if (SDL_PollEvent(event))
            return 1;
        gone = SDL_GetTicks() - start;
        if ((int)gone >= ms)
            return 0;
        SDL_Delay(ms - (int)gone < 5 ? ms - (int)gone : 5);
    }
}

SDL_sem *DOD_CreateSemaphore(Uint32 value)
{
    SDL_sem *sem = (SDL_sem *)malloc(sizeof *sem);
    if (sem)
        sem->count = value;
    return sem;
}

int DOD_SemPost(SDL_sem *sem)
{
    if (!sem)
        return -1;
    sem->count++;
    return 0;
}

static BPTR progdir_lock;
static BPTR old_dir;

static void amiga_done(void)
{
    if (progdir_lock) {
        CurrentDir(old_dir);
        UnLock(progdir_lock);
        progdir_lock = 0;
    }
}

void DOD_AmigaInit(void)
{
    progdir_lock = Lock((CONST_STRPTR)"PROGDIR:", ACCESS_READ);
    if (progdir_lock) {
        old_dir = CurrentDir(progdir_lock);
        atexit(amiga_done);
    }
}
