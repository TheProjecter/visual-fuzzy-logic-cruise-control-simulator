#include "Game.h"
#include <OgreException.h>

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#include <stdio.h>
#include <io.h>
#include <fcntl.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
    INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT )
	{
#ifdef _DEBUG
	   // Create a console (if one doesn't already exist)
	   AllocConsole();

	   // Adjust the console parameters (scroll buffer)
	   CONSOLE_SCREEN_BUFFER_INFO coninfo;
	   GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
	   coninfo.dwSize.Y = 1000;
	   SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

	   // Redirect stdout
	   HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	   int handleOut = _open_osfhandle((intptr_t)hStdOut, _O_TEXT);
	   FILE *fpOut = _fdopen(handleOut, "w");
	   *stdout = *fpOut;
	   setvbuf(stdout, NULL, _IONBF, 0);

	   // Redirect stdin
	   HANDLE hStdIn = GetStdHandle(STD_INPUT_HANDLE);
	   int handleIn = _open_osfhandle((intptr_t)hStdIn, _O_TEXT);
	   FILE *fpIn = _fdopen(handleIn, "r");
	   *stdin = *fpIn;
	   setvbuf(stdin, NULL, _IONBF, 0);

	   // Redirect stderr
	   HANDLE hStdErr = GetStdHandle(STD_ERROR_HANDLE);
	   int handleErr = _open_osfhandle((intptr_t)hStdErr, _O_TEXT);
	   FILE *fpErr = _fdopen(handleErr, "w");
	   *stderr = *fpErr;
	   setvbuf(stderr, NULL, _IONBF, 0);  

	   // Sync other IO streams with stdio (cout, wcout, cin, wcin, wcerr, cerr, wclog, clog)
	   ios::sync_with_stdio();
#endif
#else
    int main(int argc, char *argv[])
	{
#endif
        try
        {
            // Create application object
            Game game;
            game.loop();
        }
        catch (Ogre::Exception &e)
        {
    #if OGRE_PLATFORM == OGRE_PLATFORM_WIN32
            MessageBox( NULL, e.getFullDescription().c_str(), "An exception has occured!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
    #else
            std::cerr << "An exception has occured: " <<
                      e.getFullDescription().c_str() << std::endl;
    #endif
        }
        return 0;
    }

#ifdef __cplusplus
}
#endif
