Dungeons of Daggorath for AmigaOS 3.x
=====================================

Dungeons of Daggorath (DynaMicro, 1982, for the TRS-80 Color Computer),
from the C++ PC port (Richard Hunerlach and others; SDL 2 and OpenGL, by
cognitivegears), for AmigaOS 3.x on a 68040 with FPU, on acsdl12, our SDL
1.2 for game ports (ACGame's screen, AHI sound).
Upstream: https://github.com/cognitivegears/DungeonsOfDaggorath at e95bf70
(version 0.5.0).

Installing
----------

Unpack the archive anywhere (on AmiCradle: SYS:Utilities/OpenCompress x
daggorath-...lha RAM:), then double-click Install-Daggorath beside the
Daggorath drawer: it asks where to put the game (Work:Games by default)
and keeps the conf and saved drawers of an earlier install. The drawer's
Libs holds the acgame.library the game was built against, and the game
opens that copy first.

Running
-------

Double-click Daggorath, or run it from a Shell; it makes its own drawer
(PROGDIR:) the current directory, so it runs from anywhere. It opens a
512x384 screen in the middle of a 640x480 frame, in 256 colours (AGA or
RTG). Needs: 68040 + FPU, AmigaOS 3.x, AHI for sound, about 3 MB of free
memory.

The game is played by typing commands (MOVE, TURN LEFT, PULL RIGHT TORCH,
or M, T L, P R T ...) and Return. Esc opens the menu (cursor keys to move,
Return to choose): new game, load and save, graphics mode, volume, cheats,
gameplay changes. ZSAVE NAME and ZLOAD NAME save and load from the
command line as well.

Files, all in the drawer:

* conf/opts.ini: the options, written by the menu's Save Options.
* saved/*.dod: saved games (text files; a PC's load here as they are).
* sound/*.wav: the sounds.

What is different on the Amiga
------------------------------

* Graphics. Upstream draws with OpenGL: lines, filled boxes and points in
  two dimensions. amiga/swgl.c draws those same calls in software into an
  8-bit picture, which is copied to the screen when the game shows a
  frame (it draws only when something changes). Each colour the game asks
  for takes the next entry of the screen's palette, so black on white,
  inverse video, the map and the vector mode's fading lines are exact.
  The NORMAL, HIRES and VECTOR modes all work.
* No NTSC artifact colours: upstream makes them with a GLSL shader, which
  needs a GPU. As upstream does when its shader cannot start, they are
  off, and the NTSC entries of the graphics menu draw as RGB.
  amiga/shader_amiga.cpp stands in for src/shader.cpp.
* Screen size. screenWidth in opts.ini (1024 upstream) is taken as 512
  (512x384, twice the CoCo's pixels) or, if less than 512, 256 (the CoCo's
  own 256x192, in a 320x240 frame, quicker on AGA). Full screen is always.
* Sound is mixed at 22050 Hz, the WAVs' own rate, in mono: the stereo
  option's panning is accepted and not heard.
* The mouse pointer is hidden (with Modern Controls on, the mouse buttons
  still attack left and right).

Building
--------

    make -f amiga/Makefile.amigaos STOVE=... NDK=... ACSDL=... ACSDLLIB=... \
         ACGAME=... ACGAMELIB=...

or, for the release archive with its installer, package/build-amiga.sh
(STOVE and OUT set; it fetches and builds acgame, acsdl12 and zlib at
pinned commits).

Output: build/amigaos/Daggorath/ (and its Daggorath.info beside it).
amiga/include/SDL2 holds the four <SDL2/...> headers the game includes;
they bring in amiga/dod_amiga.h, which maps the SDL 2 calls it makes onto
SDL 1.2 (amiga/dod_amiga.c: the screen, SDL_WaitEventTimeout, the
semaphores the sound code posts) and declares the GL calls of
amiga/swgl.c. libstdc++'s thread hooks are amiga/pthread_stubs.c.
Changes to upstream are under `#ifdef __amigaos__`: in src/dod.cpp
(PROGDIR:), src/oslink.cpp (screen size and sound rate, a short sleep in
the main loop, SDL 2's key repeat field, a bound on the key table) and
src/dodgame.cpp (<cmath>).

The icons are amiga/icons, made by AmiCradle's port_icons.py from
amiga/icons/icons.json; the art is our own (amiga/icons/art/daggorath.svg).

Byte order
----------

The game has no binary files of its own. Saved games and opts.ini are
text, written with printf's %d and read back with >> and sscanf, so they
are the same on either byte order (a PC's CR LF lines read fine). The
WAVs are read by acsdl12, which swaps their 16-bit samples (the game's
are 8-bit). Nothing in the game takes the bytes of a larger number one by
one.

Licence
-------

The game is Douglas J. Morgan's (copyright 1982 DynaMicro). In the grant
quoted in Licence (upstream's license/license.txt), he gives a
non-exclusive, permanent, world-wide licence to anyone to develop,
produce, duplicate, emulate or distribute the game, on the sole condition
that they exercise every effort to preserve the game insofar as possible
in its original and unaltered form. The PC port names no licence of its
own. acsdl12 is MIT.

Amiga port: the AmiCradle Team.
