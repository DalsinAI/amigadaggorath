/*
 * Dungeons of Daggorath on acsdl12: the SDL 2 and OpenGL names the game
 * uses, on SDL 1.2 and a small software renderer.
 *
 * The game includes <SDL2/SDL.h>, <SDL2/SDL_opengl.h>, <SDL2/SDL_mixer.h>
 * and <SDL2/SDL_mutex.h>; amiga/include/SDL2 has those four, and each
 * includes this. The window is acsdl12's 8-bit screen; the GL calls the
 * game makes (immediate-mode lines, quads and points in two dimensions,
 * a translation, glClear) are drawn by amiga/swgl.cpp into a buffer of
 * our own, which SDL_GL_SwapWindow copies to the screen. The NTSC
 * artifact-colour shader is not here (amiga/shader_amiga.cpp says no),
 * as upstream does when a GPU cannot run it.
 *
 * The AmiCradle Team. Like the game, under Douglas J. Morgan's licence.
 */
#ifndef DOD_AMIGA_H
#define DOD_AMIGA_H

#include <SDL.h>
#include <SDL_mixer.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------- SDL 2 */

typedef SDLKey SDL_Keycode;
typedef SDL_keysym SDL_Keysym;
typedef struct DOD_Window SDL_Window;
typedef void *SDL_GLContext;
typedef struct DOD_sem SDL_sem;

#define SDLK_RGUI                 SDLK_RSUPER
#define SDLK_LGUI                 SDLK_LSUPER
#define SDLK_APPLICATION          SDLK_MENU
#define SDLK_NUMLOCKCLEAR         SDLK_NUMLOCK
#define SDLK_SCROLLLOCK           SDLK_SCROLLOCK

/* a case label in the game's event switches: SDL 1.2's expose */
#define SDL_WINDOWEVENT_EXPOSED   SDL_VIDEOEXPOSE

#define SDL_WINDOWPOS_UNDEFINED   0
#define SDL_WINDOW_OPENGL         0x2
#define SDL_WINDOW_FULLSCREEN     0x1
#define SDL_TICKS_PASSED(a, b)    ((Sint32)((b) - (a)) <= 0)

#define SDL_GL_RED_SIZE               0
#define SDL_GL_GREEN_SIZE             1
#define SDL_GL_BLUE_SIZE              2
#define SDL_GL_DEPTH_SIZE             3
#define SDL_GL_DOUBLEBUFFER           4
#define SDL_GL_CONTEXT_MAJOR_VERSION  5
#define SDL_GL_CONTEXT_MINOR_VERSION  6

SDL_Window *DOD_CreateWindow(int w, int h, Uint32 flags);
void DOD_SetWindowSize(SDL_Window *window, int w, int h);
void DOD_SwapWindow(SDL_Window *window);
int  DOD_WaitEventTimeout(SDL_Event *event, int ms);
SDL_sem *DOD_CreateSemaphore(Uint32 value);
int  DOD_SemPost(SDL_sem *sem);
void DOD_AmigaInit(void);

#define SDL_CreateWindow(title, x, y, w, h, flags) DOD_CreateWindow(w, h, flags)
#define SDL_SetWindowTitle(win, title)  SDL_WM_SetCaption(title, title)
#define SDL_SetWindowSize(win, w, h)    DOD_SetWindowSize(win, w, h)
#define SDL_SetWindowFullscreen(win, f) ((void)(win), (void)(f), 0)
#define SDL_GL_CreateContext(win)       ((SDL_GLContext)(win))
#define SDL_GL_SetAttribute(a, v)       ((void)(a), (void)(v), 0)
#define SDL_GL_SetSwapInterval(i)       ((void)(i), 0)
#define SDL_GL_SwapWindow(win)          DOD_SwapWindow(win)
#define SDL_WaitEventTimeout(ev, ms)    DOD_WaitEventTimeout(ev, ms)
#define SDL_CreateSemaphore(v)          DOD_CreateSemaphore(v)
#define SDL_SemPost(s)                  DOD_SemPost(s)
#ifndef DOD_NO_REMAP
/* The screen is ours; the pointer stays hidden on it (the game is played
 * from the keyboard; Modern Controls' mouse buttons still work). */
#define SDL_ShowCursor(toggle)          ((void)(toggle), 0)
#endif

/* -------------------------------------------- OpenGL, in software */

typedef unsigned int GLenum;
typedef unsigned int GLbitfield;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned char GLboolean;
typedef float GLfloat;
typedef float GLclampf;
typedef double GLdouble;
typedef char GLchar;

#define GL_FALSE              0
#define GL_TRUE               1
#define GL_POINTS             0x0000
#define GL_LINES              0x0001
#define GL_TRIANGLES          0x0004
#define GL_QUADS              0x0007
#define GL_DEPTH_BUFFER_BIT   0x0100
#define GL_COLOR_BUFFER_BIT   0x4000
#define GL_LINE_SMOOTH        0x0B20
#define GL_MODELVIEW          0x1700
#define GL_PROJECTION         0x1701

void glBegin(GLenum mode);
void glEnd(void);
void glVertex2f(GLfloat x, GLfloat y);
void glColor3f(GLfloat r, GLfloat g, GLfloat b);
void glColor3fv(const GLfloat *v);
void glColor4fv(const GLfloat *v);
void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a);
void glClear(GLbitfield mask);
void glMatrixMode(GLenum mode);
void glLoadIdentity(void);
void glTranslatef(GLfloat x, GLfloat y, GLfloat z);
void glOrtho(GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f);
void glViewport(GLint x, GLint y, GLsizei w, GLsizei h);
void glEnable(GLenum cap);
void glDisable(GLenum cap);

#ifdef __cplusplus
}
#endif

#endif
