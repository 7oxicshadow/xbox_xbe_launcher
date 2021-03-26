/* This file uses some code from the nxdk samples for SDL and winapi */

#include <stdio.h>
#include <string.h>
#include <windows.h>
#include <nxdk/mount.h>
#include <hal/debug.h>
#include <hal/video.h>
#include <hal/xbox.h>
#include <SDL.h>
#include <pbkit/pbkit.h>
#include <stdbool.h>
#include <time.h>

char namearray[10][100];
char path[255];
bool pbk_init = false;
bool sdl_init = false;
SDL_GameController *pad = NULL;
int target_xbe = 0;

void wait_then_cleanup(void)
{
    Sleep(5000);

    if (pbk_init)
        pb_kill();
    if (pad != NULL)
        SDL_GameControllerClose(pad);
    if (sdl_init)
        SDL_Quit();
    exit(1);
}

void findfiles(void)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    int i = 0;

    // Like on Windows, "*.*" and "*" will both list all files,
    // no matter whether they contain a dot or not
    hFind = FindFirstFile("D:\\*.xbe", &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        debugPrint("FindFirstHandle() failed!\n");
        Sleep(5000);
        exit(1);
    }

    do {
        if (! (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            //do not include the launcher default.xbe
            if(strcmp(findFileData.cFileName, "default.xbe") != 0){
                strcpy(&namearray[i][0], findFileData.cFileName);
                //set "_default.xbe" as the default if found
                if(strcmp(findFileData.cFileName, "_default.xbe") == 0)
                    target_xbe = i;
                i++;
            }
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    DWORD error = GetLastError();
    if (error == ERROR_NO_MORE_FILES)
        debugPrint("Done!\n");
    else
        debugPrint("error: %x\n", error);

    FindClose(hFind);
}

void init(void)
{
    BOOL ret;

    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);

    sdl_init = SDL_Init(SDL_INIT_GAMECONTROLLER) == 0;
    if (!sdl_init) {
        debugPrint("SDL_Init failed: %s\n", SDL_GetError());
        wait_then_cleanup();
    }

    if (SDL_NumJoysticks() < 1) {
        debugPrint("Please connect gamepad\n");
        wait_then_cleanup();
    }

    pad = SDL_GameControllerOpen(0);
    if (pad == NULL) {
        debugPrint("Failed to open gamecontroller 0\n");
        wait_then_cleanup();
    }

    pbk_init = pb_init() == 0;
    if (!pbk_init) {
        debugPrint("pbkit init failed\n");
        wait_then_cleanup();
    }

    memset(namearray,'\0', sizeof(namearray));
    memset(path,'\0', sizeof(path));

    // Mount D:
    if (nxIsDriveMounted('D')) {
        nxUnmountDrive('D');
    }
    ret = nxMountDrive('D', "\\Device\\CdRom0");
    if (!ret) {
        debugPrint("Failed to mount D: drive!\n");
        Sleep(5000);
        exit(1);
    }
    debugPrint("Mounted Drive D:\n");
}

int main(void)
{
    #define DEBOUNCE (5)
    #define COUNTDOWN (5)
    
    int db_up = 0;
    int db_dn = 0;
    int db_a = 0;
    int numchoices = 0;
    int timer = COUNTDOWN;
    BOOL prelaunch = false;
    BOOL startlaunch = false;
    BOOL countdown_enabled = true;
    BOOL reset_target_complete = false;
    clock_t start_clock;
    clock_t diff_clock;

    init();

    findfiles();

    pb_show_front_screen();
   
    start_clock = clock();

    while (1) {

        //process the timer
        if(countdown_enabled){
            diff_clock = ((clock() - start_clock) / CLOCKS_PER_SEC);
            /*this is crude. It works but could be done much better. 
              The int timer should not be needed*/
            if(diff_clock > 0){
                start_clock = clock();
                timer--;
            }
        }
        else{
            //reset the target_xbe to 0 once if timer has been disabled
            if(reset_target_complete == false){
                target_xbe = 0;
                reset_target_complete = true;
            
            }
        }

        pb_wait_for_vbl();
        pb_target_back_buffer();
        pb_reset();
        pb_fill(0, 0, 640, 480, 0);
        pb_erase_text_screen();

        SDL_GameControllerUpdate();
        
        pb_print("XBE Launcher\n");
        pb_print("------------\n\n");

        //print available xbe files
        int y = 0;
        numchoices = 0;
        for(y=0; y<10; y++){
            if(namearray[y][0] != '\0'){
                pb_print("%d : %s\n", y, &namearray[y][0]);
                numchoices++;
            }
        }

        //offset by one because we reference the array from 0
        if(numchoices > 0 )
            numchoices--;

        if(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP)){
            db_up++;
            if(db_up > DEBOUNCE){
                if(target_xbe < numchoices){
                    target_xbe++;
                }
                db_up = 0;
                countdown_enabled = false;
            }
        }
        else
            db_up = 0;

        if(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN)){
            db_dn++;
            if(db_dn > DEBOUNCE){
                if(target_xbe > 0){
                    target_xbe--;
                }
                db_dn = 0;
                countdown_enabled = false;
            }
        }
        else
            db_dn = 0;

        pb_print("\n");
        pb_print("(Use dpad up/down and A to manually select)\n");
        pb_print("\n");
        if(countdown_enabled)
            pb_print("Auto launch (%s) in %d seconds\n", &namearray[target_xbe][0], timer);     
        else
            pb_print("Select Required XBE Number : %d\n", target_xbe);

        if(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_A)){
            db_a++;
            if(db_a > DEBOUNCE){
                prelaunch = true;
                db_a = 0;
                countdown_enabled = false;
            }
        }
        else{
            db_a = 0;
        }
        
        // we do a pre-launch here so that the screen is updated before launch
        if((prelaunch) || (timer <= 0)){
            //note: using dos paths does not work for the cdrom drive
            strcpy(path,"\\Device\\CdRom0\\");
            strcat(path, &namearray[target_xbe][0]);
            pb_print("%s\n", path);
            pb_print("\nLaunching\n");
            startlaunch = true;
        }

        pb_draw_text_screen();
        while (pb_busy());
        while (pb_finished());

        if(startlaunch){
            Sleep(1000);
            XLaunchXBE(path);
        }
    }

    wait_then_cleanup();
}
