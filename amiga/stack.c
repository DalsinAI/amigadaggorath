/* Dungeons of Daggorath keeps its state in globals, but its menus loop
 * inside one another and libstdc++'s streams want room: a Shell's 4 KB is
 * far too little, so libnix's startup sets this much up instead.
 * Started from Workbench, what the game writes (its build line, or why it
 * could not open the screen or AHI) goes to a window that opens only when
 * something is written.
 * The AmiCradle Team. Like the game, under Douglas J. Morgan's licence. */
unsigned long __stack = 128 * 1024;
const char *__stdiowin = "CON:0/12/640/200/Dungeons of Daggorath/AUTO/CLOSE/INACTIVE";
