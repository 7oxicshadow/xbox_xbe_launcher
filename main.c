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
#include "main.h"

#define MAX_XBE_COUNT   ( 10 )
#define MAX_XBE_LENGTH  ( 20 )

char namearray[MAX_XBE_COUNT][MAX_XBE_LENGTH];
char listbuffer[100];
char selectbuffer[50];
char path[255];
bool pbk_init = false;
bool sdl_init = false;
SDL_GameController *pad = NULL;
int target_xbe = 0;
cert_data_un xbecert;
char ascii_title[41];
char ascii_region[30];

void read_header(void)
{    
    FILE *fptr = NULL;
    int i;
    char temp_path[30];
    header_data_un xbeheader;


    //create path string
    memset(temp_path,'\0', sizeof(temp_path));
    strcat(temp_path, "d:\\");
    strcat(temp_path, &namearray[0][0]);
                        
    //attempt to open default path first
    fptr = fopen("d:\\_default.xbe", "r");

    //check if the file failed to open
    if(fptr == NULL){

        //try again with the path of the first xbe we found
        fptr = fopen(temp_path, "r");
    
        //check for failure to open
        if(fptr == NULL){
            debugPrint("Header : failed to open xbe\n");
            Sleep(2000);
            return;
        }
    }

    //populate local copy of the header (Not a complete header)
    i = 0;
    while(i < (sizeof(xbeheader.headerraw)) )
    {
        xbeheader.headerraw[i] = fgetc(fptr);
        //debugPrint("%02X", xbeheader.headerraw[i]);
        i++;
    }

    //move file pointer to the start of the certificate.
    fseek(fptr, xbeheader.headerdata.dwCertificateAddr - xbeheader.headerdata.dwBaseAddr, SEEK_SET);
    
    //populate local copy of the certificate
    i = 0;
    while(i < sizeof(xbecert.certraw))
    {
        xbecert.certraw[i] = fgetc(fptr);
        //debugPrint("%02X", xbecert.certraw[i]);
        i++;
    }

    //convert title to ascii (16bit padded to char)
    i = 0;
    while(i < 40)
    {
        ascii_title[i] = (char)xbecert.certdata.wszTitleName[i];
        //debugPrint("%c", ascii_title[i]);
        i++;
    }
    
    //convert region to ascii string
    if((xbecert.certdata.dwGameRegion & GAME_REGION_NA) != 0)
        strcat(ascii_region, "NA, ");
    if((xbecert.certdata.dwGameRegion & GAME_REGION_JAPAN) != 0)
        strcat(ascii_region, "JAPAN, ");
    if((xbecert.certdata.dwGameRegion & GAME_REGION_RESTOFWORLD) != 0)
        strcat(ascii_region, "ROW, ");
    if((xbecert.certdata.dwGameRegion & GAME_REGION_MANUFACTURING) != 0)
        strcat(ascii_region, "MANU");
    
    //close the xbe
    fclose(fptr);
}

void cleanup(void)
{
    if (pbk_init)
        pb_kill();
    if (pad != NULL)
        SDL_GameControllerClose(pad);
    if (sdl_init)
        SDL_Quit();
}

void cleanup_and_exit(void)
{
    cleanup();
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
                
                snprintf(&namearray[i][0], (MAX_XBE_LENGTH - 1), "%s", findFileData.cFileName);

                //set "_default.xbe" as the default if found
                if(strcmp(findFileData.cFileName, "_default.xbe") == 0)
                    target_xbe = i;
                i++;
            }
        }

        /* Drop out if we exceed the max storage count */
        if( i > MAX_XBE_COUNT )
            break;

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
    SDL_Event event;
    
    memset(xbecert.certraw, 0, sizeof(xbecert.certraw));
    memset(namearray,'\0', sizeof(namearray));
    memset(path,'\0', sizeof(path));
    memset(ascii_title,'\0', sizeof(ascii_title));
    memset(ascii_region,'\0', sizeof(ascii_region));

    XVideoSetMode(640, 480, 32, REFRESH_DEFAULT);
       
    sdl_init = SDL_Init(SDL_INIT_GAMECONTROLLER) == 0;
    if (!sdl_init) {
        debugPrint("SDL_Init failed: %s\n", SDL_GetError());
        cleanup_and_exit();
    }

    pbk_init = pb_init() == 0;
    if (!pbk_init) {
        debugPrint("pbkit init failed\n");
        cleanup_and_exit();
    }

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
    
    static SDL_Event e;
    int numchoices = 0;
    int timer = COUNTDOWN;
    int j, y;
    BOOL prelaunch = false;
    BOOL startlaunch = false;
    BOOL countdown_enabled = true;
    BOOL reset_target_complete = false;
    clock_t start_clock;
    clock_t diff_clock;
    int button_release = TRUE;
       
    init();

    findfiles();
    
    read_header();

    pb_show_front_screen();
   
    start_clock = clock();

    //Create a string of discovered files
    y = 0;
    j = 0;
    numchoices = 0;
    for(y=0; y<10; y++){
        if(namearray[y][0] != '\0'){
            j += sprintf(listbuffer + j, "%d : %s\n", y, &namearray[y][0]);
            numchoices++;
        }
    }

    //offset by one because we reference the array from 0
    if(numchoices > 0 )
        numchoices--;
    
    while (!startlaunch) {

        /* Clear the screen */
        pb_wait_for_vbl();
        pb_target_back_buffer();
        pb_reset();
        pb_fill(0, 0, 640, 480, 0);
        pb_erase_text_screen();

        //process the timer
        if(countdown_enabled){
            diff_clock = ((clock() - start_clock) / CLOCKS_PER_SEC);
            /*this is crude. It works but could be done much better. 
              The int timer should not be needed*/
            if(diff_clock > 0){
                start_clock = clock();
                timer--;
            }

            /* Set the feedback line for the user */
            sprintf(selectbuffer, "Auto launch (%s) in %d seconds\n", &namearray[target_xbe][0], timer);
        }
        else{
            //reset the target_xbe to 0 once if timer has been disabled
            if(reset_target_complete == false){
                target_xbe = 0;
                reset_target_complete = true;
            
            }

            /* Set the feedback line for the user */
            sprintf(selectbuffer, "Select Required XBE Number : %d\n", target_xbe);
        }

        /* Print the main screen */
        pb_print("XBE Launcher\n"
                 "------------\n"
                 "Title: %s\n"
                 "Region Flags: %s\n\n"
                 "%s\n"
                 "(Use dpad up/down and A to manually select)\n"
                 "%s\n",
                 ascii_title,
                 ascii_region,
                 listbuffer,
                 selectbuffer);

        /* Poll the SDL queue to determine the connected controller */
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_CONTROLLERDEVICEADDED) {
                SDL_GameController *new_pad = SDL_GameControllerOpen(e.cdevice.which);
                if (pad == NULL) {
                pad = new_pad;
                }
            }
            else if (e.type == SDL_CONTROLLERDEVICEREMOVED) {
                if (pad == SDL_GameControllerFromInstanceID(e.cdevice.which)) {
                pad = NULL;
                }
                SDL_GameControllerClose(SDL_GameControllerFromInstanceID(e.cdevice.which));
            }
            else if (e.type == SDL_CONTROLLERBUTTONDOWN) {
                if (e.cbutton.button == SDL_CONTROLLER_BUTTON_START) {
                pad = (SDL_GameControllerFromInstanceID(e.cdevice.which));
                }
            }
        }

        /* Manually update the controller buttons. (Is this needed?) */
        SDL_GameControllerUpdate();

        /* Check if we have detected a pad */
        if (pad != NULL) {

            /* Check for dpad up press */
            if(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_UP)){

                if (TRUE == button_release )
                {
                    if(target_xbe < numchoices){
                        target_xbe++;
                    }
                    countdown_enabled = false;
                }

                button_release = FALSE;

            }
            /* Check for dpad down press */
            else if(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_DPAD_DOWN)){

                if (TRUE == button_release )
                {
                    if(target_xbe > 0){
                        target_xbe--;
                    }
                    countdown_enabled = false;
                }

                button_release = FALSE;
            }
            /* Check for button a press */
            else if(SDL_GameControllerGetButton(pad, SDL_CONTROLLER_BUTTON_A)){

                if (TRUE == button_release )
                {
                    prelaunch = true;
                    countdown_enabled = false;
                }

                button_release = FALSE;
            }
            /* No Press */
            else{
                button_release = TRUE;
            }
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

        /* Output buffer to screen */
        pb_draw_text_screen();
        while (pb_busy());
        while (pb_finished());
    }

    /* Shutdown un-needed systems */
    cleanup();
    
    /* Launch the selected xbe */
    XLaunchXBE(path);
    
    exit(1);
}
