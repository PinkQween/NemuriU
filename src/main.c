#include "nekopara_demo.h"
#include <stdio.h>

/*
 * NemuriU - CVN-Powered Visual Novel for Wii U
 * 
 * Now using the CVN (C Visual Novel) engine for high-performance
 * dual-screen visual novel experiences!
 * 
 * Features:
 * - Native C performance (100x faster than Python-based engines)
 * - Wii U dual-screen support (TV + GamePad)
 * - Advanced sprite system with animations
 * - Layered rendering with z-ordering
 * - Audio system with BGM and SFX
 * - Native C scripting for maximum power
 */

int main(int argc, char* argv[]) {
    (void)argc;
    (void)argv;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════╗\n");
    printf("║         NemuriU - CVN Visual Novel            ║\n");
    printf("║              Powered by CVN Engine             ║\n");
    printf("╚════════════════════════════════════════════════╝\n");
    printf("\n");
    
    /* Run the Nekopara demo */
    int result = nekopara_demo_run();
    
    return result;
}
