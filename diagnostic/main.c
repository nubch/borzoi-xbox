#include <hal/video.h>
#include <hal/debug.h>
#include <windows.h>

int main(void) {
    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
    debugPrint("BORZOI 2001 DIAGNOSTIC\n");
    debugPrint("======================\n\n");
    debugPrint("XBE LOADER: OK\nVIDEO MODE: OK\n\n");
    debugPrint("If you see this, EvolutionX successfully launched an nxdk homebrew.\n");
    debugPrint("Power-cycle the Xbox to return to the dashboard.\n");
    while (1) Sleep(1000);
    return 0;
}
