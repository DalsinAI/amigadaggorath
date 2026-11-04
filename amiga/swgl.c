/*
 * The OpenGL that Dungeons of Daggorath draws with, in software, into an
 * 8-bit buffer for acsdl12's screen.
 *
 * The game draws in two dimensions with immediate-mode GL: GL_LINES (the
 * vector mode), GL_QUADS (its font, the CoCo-sized pixels of the normal
 * mode, filled boxes) and GL_POINTS (the hires mode), in colours given as
 * floats, through glOrtho(0, w, 0, h) and a glTranslatef for its text.
 * That is all this does:
 *
 *  - Colours: each new colour the game asks for takes the next free entry
 *    of the screen's palette (black is entry 0, the frame's border), so
 *    white on black, black on white (inverse video), the map's colours and
 *    the vector mode's fade steps are all exact. Past 256 the nearest is
 *    used. The palette is set on the screen at the next swap.
 *  - Quads are filled as GL fills them: the pixels whose centres are inside
 *    (a box straight to the axes, which nearly all of them are, takes a
 *    quick path); lines are Bresenham's between the pixels of the two ends,
 *    clipped to the screen first; a point is its pixel.
 *  - The game's y goes up, as GL's does; the buffer's rows go down.
 *
 * The AmiCradle Team. Like the game, under Douglas J. Morgan's licence.
 */
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "dod_amiga.h"

void swgl_resize(int w, int h);
void swgl_present(SDL_Surface *screen);

static Uint8 *fb;
static int fbw, fbh;

static SDL_Color pal[256];
static Uint32 palkey[256];
static int npal;
static int pal_dirty;
static Uint32 last_key = 0xffffffffUL;
static Uint8 last_index;

static Uint8 cur;        /* the current colour's index */
static Uint8 clear_index;

static GLenum matrix_mode = GL_MODELVIEW;
static float tx, ty;     /* the model-view matrix: a translation */
static double ortho_l = 0, ortho_r = 1, ortho_b = 0, ortho_t = 1;
static int vp_x, vp_y, vp_w = 1, vp_h = 1;
static float sx = 1, sy = 1, ox, oy;   /* object to window */

static GLenum prim = 0xffff;
static float vx[4], vy[4];
static int nv;

static void update_xform(void)
{
    double rw = ortho_r - ortho_l, rh = ortho_t - ortho_b;
    if (rw == 0) rw = 1;
    if (rh == 0) rh = 1;
    sx = (float)(vp_w / rw);
    sy = (float)(vp_h / rh);
    ox = (float)(vp_x - ortho_l * sx);
    oy = (float)(vp_y - ortho_b * sy);
}

static Uint8 to8(float f)
{
    if (f <= 0.0f) return 0;
    if (f >= 1.0f) return 255;
    return (Uint8)(f * 255.0f + 0.5f);
}

static Uint8 colour_index(float r, float g, float b)
{
    Uint8 R = to8(r), G = to8(g), B = to8(b);
    Uint32 key = ((Uint32)R << 16) | ((Uint32)G << 8) | B;
    int i, best = 0;
    long bestd = 0x7fffffffL;

    if (key == last_key)
        return last_index;
    if (npal == 0) {             /* entry 0 is black, the frame's border */
        pal[0].r = pal[0].g = pal[0].b = 0;
        palkey[0] = 0;
        npal = 1;
        pal_dirty = 1;
    }
    for (i = 0; i < npal; i++)
        if (palkey[i] == key)
            goto found;
    if (npal < 256) {
        i = npal++;
        pal[i].r = R;
        pal[i].g = G;
        pal[i].b = B;
        pal[i].unused = 0;
        palkey[i] = key;
        pal_dirty = 1;
        goto found;
    }
    for (i = 0; i < 256; i++) {
        long dr = (long)pal[i].r - R, dg = (long)pal[i].g - G, db = (long)pal[i].b - B;
        long d = dr * dr + dg * dg + db * db;
        if (d < bestd) {
            bestd = d;
            best = i;
        }
    }
    i = best;
found:
    last_key = key;
    last_index = (Uint8)i;
    return (Uint8)i;
}

void swgl_resize(int w, int h)
{
    free(fb);
    fbw = w;
    fbh = h;
    fb = (Uint8 *)malloc((size_t)w * h);
    if (fb)
        memset(fb, clear_index, (size_t)w * h);
    vp_x = vp_y = 0;
    vp_w = w;
    vp_h = h;
    ortho_l = 0; ortho_r = w; ortho_b = 0; ortho_t = h;
    update_xform();
}

void swgl_present(SDL_Surface *screen)
{
    int y, w, h;
    Uint8 *dst;

    if (!screen || !fb)
        return;
    if (pal_dirty || npal == 0) {
        colour_index(0, 0, 0);
        SDL_SetColors(screen, pal, 0, npal);
        pal_dirty = 0;
    }
    w = screen->w < fbw ? screen->w : fbw;
    h = screen->h < fbh ? screen->h : fbh;
    if (SDL_LockSurface(screen) < 0)
        return;
    dst = (Uint8 *)screen->pixels;
    for (y = 0; y < h; y++)
        memcpy(dst + (size_t)y * screen->pitch, fb + (size_t)y * fbw, w);
    SDL_UnlockSurface(screen);
    SDL_Flip(screen);
}

/* ------------------------------------------------------- rasterising */

/* A span of pixels on window row r (counted from the foot) */
static void span(int r, int x0, int x1)
{
    if (r < 0 || r >= fbh)
        return;
    if (x0 < 0) x0 = 0;
    if (x1 > fbw - 1) x1 = fbw - 1;
    if (x0 > x1)
        return;
    memset(fb + (size_t)(fbh - 1 - r) * fbw + x0, cur, x1 - x0 + 1);
}

static void put_point(float x, float y)
{
    int px = (int)floorf(x), py = (int)floorf(y);
    if (px < 0 || py < 0 || px >= fbw || py >= fbh)
        return;
    fb[(size_t)(fbh - 1 - py) * fbw + px] = cur;
}

/* Liang-Barsky: the part of the line inside [0, w) x [0, h) */
static int clip_test(float p, float q, float *t0, float *t1)
{
    float r;
    if (p == 0.0f)
        return q >= 0.0f;
    r = q / p;
    if (p < 0.0f) {
        if (r > *t1) return 0;
        if (r > *t0) *t0 = r;
    } else {
        if (r < *t0) return 0;
        if (r < *t1) *t1 = r;
    }
    return 1;
}

static void put_line(float x0, float y0, float x1, float y1)
{
    float t0 = 0.0f, t1 = 1.0f, dx = x1 - x0, dy = y1 - y0;
    float xmax = (float)fbw - 0.001f, ymax = (float)fbh - 0.001f;
    int ix0, iy0, ix1, iy1, sxs, sys, ex, ey, err;

    if (!clip_test(-dx, x0, &t0, &t1) || !clip_test(dx, xmax - x0, &t0, &t1) ||
        !clip_test(-dy, y0, &t0, &t1) || !clip_test(dy, ymax - y0, &t0, &t1))
        return;
    ix0 = (int)(x0 + t0 * dx);
    iy0 = (int)(y0 + t0 * dy);
    ix1 = (int)(x0 + t1 * dx);
    iy1 = (int)(y0 + t1 * dy);
    ix0 = ix0 < 0 ? 0 : ix0 >= fbw ? fbw - 1 : ix0;
    ix1 = ix1 < 0 ? 0 : ix1 >= fbw ? fbw - 1 : ix1;
    iy0 = iy0 < 0 ? 0 : iy0 >= fbh ? fbh - 1 : iy0;
    iy1 = iy1 < 0 ? 0 : iy1 >= fbh ? fbh - 1 : iy1;

    ex = ix1 > ix0 ? ix1 - ix0 : ix0 - ix1;
    ey = iy1 > iy0 ? iy1 - iy0 : iy0 - iy1;
    sxs = ix0 < ix1 ? 1 : -1;
    sys = iy0 < iy1 ? 1 : -1;
    err = ex - ey;
    for (;;) {
        fb[(size_t)(fbh - 1 - iy0) * fbw + ix0] = cur;
        if (ix0 == ix1 && iy0 == iy1)
            break;
        {
            int e2 = 2 * err;
            if (e2 > -ey) { err -= ey; ix0 += sxs; }
            if (e2 < ex) { err += ex; iy0 += sys; }
        }
    }
}

/* The first pixel whose centre is at or past v */
static int first_centre(float v)
{
    return (int)ceilf(v - 0.5f);
}

static void put_quad(void)
{
    float minx = vx[0], maxx = vx[0], miny = vy[0], maxy = vy[0];
    int i, r, r0, r1, box = 1;

    for (i = 1; i < 4; i++) {
        if (vx[i] < minx) minx = vx[i];
        if (vx[i] > maxx) maxx = vx[i];
        if (vy[i] < miny) miny = vy[i];
        if (vy[i] > maxy) maxy = vy[i];
    }
    for (i = 0; i < 4; i++)
        if ((vx[i] != minx && vx[i] != maxx) || (vy[i] != miny && vy[i] != maxy))
            box = 0;

    r0 = first_centre(miny);
    r1 = first_centre(maxy) - 1;
    if (r0 < 0) r0 = 0;
    if (r1 > fbh - 1) r1 = fbh - 1;
    if (box) {
        int c0 = first_centre(minx), c1 = first_centre(maxx) - 1;
        for (r = r0; r <= r1; r++)
            span(r, c0, c1);
        return;
    }
    /* a convex quad: each row's centre line crosses two of its edges */
    for (r = r0; r <= r1; r++) {
        float yc = (float)r + 0.5f, xl = 1e30f, xr = -1e30f;
        for (i = 0; i < 4; i++) {
            int j = (i + 1) & 3;
            float ya = vy[i], yb = vy[j];
            if ((ya <= yc && yc < yb) || (yb <= yc && yc < ya)) {
                float x = vx[i] + (yc - ya) * (vx[j] - vx[i]) / (yb - ya);
                if (x < xl) xl = x;
                if (x > xr) xr = x;
            }
        }
        if (xl <= xr)
            span(r, first_centre(xl), first_centre(xr) - 1);
    }
}

/* --------------------------------------------------------- GL calls */

void glBegin(GLenum mode)
{
    prim = mode;
    nv = 0;
}

void glEnd(void)
{
    prim = 0xffff;
    nv = 0;
}

void glVertex2f(GLfloat x, GLfloat y)
{
    float wx = (x + tx) * sx + ox, wy = (y + ty) * sy + oy;

    if (!fb)
        return;
    switch (prim) {
    case GL_POINTS:
        put_point(wx, wy);
        break;
    case GL_LINES:
        vx[nv] = wx;
        vy[nv] = wy;
        if (++nv == 2) {
            put_line(vx[0], vy[0], vx[1], vy[1]);
            nv = 0;
        }
        break;
    case GL_QUADS:
        vx[nv] = wx;
        vy[nv] = wy;
        if (++nv == 4) {
            put_quad();
            nv = 0;
        }
        break;
    default:
        break;
    }
}

void glColor3f(GLfloat r, GLfloat g, GLfloat b) { cur = colour_index(r, g, b); }
void glColor3fv(const GLfloat *v) { cur = colour_index(v[0], v[1], v[2]); }
/* the game passes its three-float colours here too: the fourth is not read */
void glColor4fv(const GLfloat *v) { cur = colour_index(v[0], v[1], v[2]); }

void glClearColor(GLclampf r, GLclampf g, GLclampf b, GLclampf a)
{
    (void)a;
    clear_index = colour_index(r, g, b);
}

void glClear(GLbitfield mask)
{
    if ((mask & GL_COLOR_BUFFER_BIT) && fb)
        memset(fb, clear_index, (size_t)fbw * fbh);
}

void glMatrixMode(GLenum mode) { matrix_mode = mode; }

void glLoadIdentity(void)
{
    if (matrix_mode == GL_MODELVIEW) {
        tx = ty = 0.0f;
    } else {
        ortho_l = -1; ortho_r = 1; ortho_b = -1; ortho_t = 1;
        update_xform();
    }
}

void glTranslatef(GLfloat x, GLfloat y, GLfloat z)
{
    (void)z;
    if (matrix_mode == GL_MODELVIEW) {
        tx += x;
        ty += y;
    }
}

void glOrtho(GLdouble l, GLdouble r, GLdouble b, GLdouble t, GLdouble n, GLdouble f)
{
    (void)n;
    (void)f;
    ortho_l = l; ortho_r = r; ortho_b = b; ortho_t = t;
    update_xform();
}

void glViewport(GLint x, GLint y, GLsizei w, GLsizei h)
{
    vp_x = x; vp_y = y; vp_w = w; vp_h = h;
    update_xform();
}

void glEnable(GLenum cap) { (void)cap; }
void glDisable(GLenum cap) { (void)cap; }
