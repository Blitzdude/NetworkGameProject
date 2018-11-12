/*
    olcPixelGameEngine.h

    +-------------------------------------------------------------+
    |           OneLoneCoder Pixel Game Engine v1.9               |
    | "Like the command prompt console one, but not..." - javidx9 |
    +-------------------------------------------------------------+

    What is this?
    ~~~~~~~~~~~~~
    The olcConsoleGameEngine has been a surprsing and wonderful
    success for me, and I'm delighted how people have reacted so
    positively towards it, so thanks for that.

    However, there are limitations that I simply cannot avoid.
    Firstly, I need to maintain several different versions of
    it to accommodate users on Windows7, 8, 10, Linux, Mac,
    Visual Studio & Code::Blocks. Secondly, this year I've been
    pushing the console to the limits of its graphical capabilities
    and the effect is becoming underwhelming. The engine itself
    is not slow at all, but the process that Windows uses to
    draw the command prompt to the screen is, and worse still,
    it's dynamic based upon the variation of character colours
    and glyphs. Sadly I have no control over this, and recent
    videos that are extremely graphical (for a command prompt :P )
    have been dipping to unacceptable framerates. As the channel
    has been popular with aspiring game developers, I'm concerned
    that the visual appeal of the command prompt is perhaps
    limited to us oldies, and I dont want to alienate younger
    learners. Finally, I'd like to demonstrate many more
    algorithms and image processing that exist in the graphical
    domain, for which the console is insufficient.

    For this reason, I have created olcPixelGameEngine! The look
    and feel to the programmer is almost identical, so all of my
    existing code from the videos is easily portable, and the
    programmer uses this file in exactly the same way. But I've
    decided that rather than just build a command prompt emulator,
    that I would at least harness some modern(ish) portable
    technologies.

    As a result, the olcPixelGameEngine supports 32-bit colour, is
    written in a cross-platform style, uses modern(ish) C++
    conventions and most importantly, renders much much faster. I
    will use this version when my applications are predominantly
    graphics based, but use the console version when they are
    predominantly text based - Don't worry, loads more command
    prompt silliness to come yet, but evolution is important!!

    License (OLC-3)
    ~~~~~~~~~~~~~~~

    Copyright 2018 OneLoneCoder.com

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions
    are met:

    1. Redistributions or derivations of source code must retain the above
    copyright notice, this list of conditions and the following disclaimer.

    2. Redistributions or derivative works in binary form must reproduce
    the above copyright notice. This list of conditions and the following
    disclaimer must be reproduced in the documentation and/or other
    materials provided with the distribution.

    3. Neither the name of the copyright holder nor the names of its
    contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Links
    ~~~~~
    YouTube:	https://www.youtube.com/javidx9
    Discord:	https://discord.gg/WhwHUMV
    Twitter:	https://www.twitter.com/javidx9
    Twitch:		https://www.twitch.tv/javidx9
    GitHub:		https://www.github.com/onelonecoder
    Homepage:	https://www.onelonecoder.com

    Relevant Videos
    ~~~~~~~~~~~~~~~
    https://youtu.be/kRH6oJLFYxY Introducing olcPixelGameEngine

    Compiling in Linux
    ~~~~~~~~~~~~~~~~~~
    You will need a modern C++ compiler, so update yours!
    To compile use the command:

    g++ -o YourProgName YourSource.cpp -lX11 -lGL -lpthread -lpng

    On some Linux configurations, the frame rate is locked to the refresh
    rate of the monitor. This engine tries to unlock it but may not be
    able to, in which case try launching your program like this:

    vblank_mode=0 ./YourProgName


    Compiling in Code::Blocks on Windows
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Well I wont judge you, but make sure your Code::Blocks installation
    is really up to date - you may even consider updating your C++ toolchain
    to use MinGW32-W64, so google this. You will also need to enable C++14
    in your build options, and add to your linker the following libraries:
    user32 gdi32 opengl32 gdiplus

    Thanks
    ~~~~~~
    I'd like to extend thanks to Eremiell, slavka, gurkanctn, Phantim,
    JackOJC, KrossX, Huhlig, Dragoneye, Appa, JustinRichardsMusic, SliceNDice
    & MagetzUb for advice, ideas and testing, and I'd like to extend
    my appreciation to the 14K YouTube followers and 1K Discord server
    members who give me the motivation to keep going with all this :D

    Special thanks to those who bring gifts!
    GnarGnarHead.......Domina
    Gorbit99...........Bastion

    Author
    ~~~~~~
    David Barr, aka javidx9, �OneLoneCoder 2018
*/

////////////////////////////////////////////////////////////////////////////////////////// 

/* Example Usage (main.cpp)
    #define OLC_PGE_APPLICATION
    #include "olcPixelGameEngine.h"
    // Override base class with your custom functionality
    class Example : public olc::PixelGameEngine
    {
    public:
        Example()
        {
            sAppName = "Example";
        }
    public:
        bool OnUserCreate() override
        {
            // Called once at the start, so create things here
            return true;
        }
        bool OnUserUpdate(float fElapsedTime) override
        {
            // called once per frame, draws random coloured pixels
            for (int x = 0; x < ScreenWidth(); x++)
                for (int y = 0; y < ScreenHeight(); y++)
                    Draw(x, y, olc::Pixel(rand() % 255, rand() % 255, rand()% 255));
            return true;
        }
    };
    int main()
    {
        Example demo;
        if (demo.Construct(256, 240, 4, 4))
            demo.Start();
        return 0;
    }
*/

#ifndef OLC_PGE_DEF
#define OLC_PGE_DEF

#ifdef _WIN32
// Link to libraries
#ifndef __MINGW32__
#pragma comment(lib, "user32.lib")		// Visual Studio Only
#pragma comment(lib, "gdi32.lib")		// For other Windows Compilers please add
#pragma comment(lib, "opengl32.lib")	// these libs to your linker input
#pragma comment(lib, "gdiplus.lib")
#else
    // In Code::Blocks, Select C++14 in your build options, and add the
    // following libs to your linker: user32 gdi32 opengl32 gdiplus
#endif
    // Include WinAPI
#include <windows.h>
#include <gdiplus.h>

// OpenGL Extension
#include <GL/gl.h>
typedef BOOL(WINAPI wglSwapInterval_t) (int interval);
static wglSwapInterval_t *wglSwapInterval;
#else
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <png.h>
typedef int(glSwapInterval_t)(Display *dpy, GLXDrawable drawable, int interval);
static glSwapInterval_t *glSwapIntervalEXT;
#endif


// Standard includes
#include <cmath>
#include <cstdint>
#include <string>
#include <iostream>
#include <chrono>
#include <vector>
#include <list>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <fstream>
#include <map>
#include <functional>

#undef min
#undef max

namespace olc // All OneLoneCoder stuff will now exist in the "olc" namespace
{
struct Pixel
{
    union
    {
        uint32_t n = 0xFF000000;
        struct
        {
            uint8_t r;	uint8_t g;	uint8_t b;	uint8_t a;
        };
    };

    Pixel();
    Pixel(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255);
    Pixel(uint32_t p);
    enum Mode { NORMAL, MASK, ALPHA, CUSTOM };
};

// Some constants for symbolic naming of Pixels
static const Pixel
WHITE(255, 255, 255),
GREY(192, 192, 192), DARK_GREY(128, 128, 128), VERY_DARK_GREY(64, 64, 64),
RED(255, 0, 0), DARK_RED(128, 0, 0), VERY_DARK_RED(64, 0, 0),
YELLOW(255, 255, 0), DARK_YELLOW(128, 128, 0), VERY_DARK_YELLOW(64, 64, 0),
GREEN(0, 255, 0), DARK_GREEN(0, 128, 0), VERY_DARK_GREEN(0, 64, 0),
CYAN(0, 255, 255), DARK_CYAN(0, 128, 128), VERY_DARK_CYAN(0, 64, 64),
BLUE(0, 0, 255), DARK_BLUE(0, 0, 128), VERY_DARK_BLUE(0, 0, 64),
MAGENTA(255, 0, 255), DARK_MAGENTA(128, 0, 128), VERY_DARK_MAGENTA(64, 0, 64),
BLACK(0, 0, 0),
BLANK(0, 0, 0, 0);

enum rcode
{
    FAIL = 0,
    OK = 1,
    NO_FILE = -1,
};

//=============================================================

struct HWButton
{
    bool bPressed = false;	// Set once during the frame the event occurs
    bool bReleased = false;	// Set once during the frame the event occurs
    bool bHeld = false;		// Set tru for all frames between pressed and released events
};

//=============================================================

// A bitmap-like structure that stores a 2D array of Pixels
class Sprite
{
public:
    Sprite();
    Sprite(std::string sImageFile);
    Sprite(int32_t w, int32_t h);
    ~Sprite();

public:
    olc::rcode LoadFromFile(std::string sImageFile);
    olc::rcode LoadFromSprFile(std::string sImageFile);

public:
    int32_t width = 0;
    int32_t height = 0;
    enum Mode { NORMAL, PERIODIC };

public:
    void SetSampleMode(olc::Sprite::Mode mode = olc::Sprite::Mode::NORMAL);
    Pixel GetPixel(int32_t x, int32_t y);
    void  SetPixel(int32_t x, int32_t y, Pixel p);
    Pixel Sample(float x, float y);
    Pixel* GetData();

private:
    Pixel *pColData = nullptr;
    Mode modeSample = Mode::NORMAL;

#ifdef OLC_DBG_OVERDRAW
public:
    static int nOverdrawCount;
#endif

};

//=============================================================

enum Key
{
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    K0, K1, K2, K3, K4, K5, K6, K7, K8, K9,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    UP, DOWN, LEFT, RIGHT,
    SPACE, TAB, SHIFT, CTRL, INS, DEL, HOME, END, PGUP, PGDN,
    BACK, ESCAPE, ENTER, PAUSE, SCROLL,
};


//=============================================================

class PixelGameEngine
{
public:
    PixelGameEngine();

public:
    olc::rcode	Construct(uint32_t screen_w, uint32_t screen_h, uint32_t pixel_w, uint32_t pixel_h);
    olc::rcode	Start();

public: // Override Interfaces
    // Called once on application startup, use to load your resources
    virtual bool OnUserCreate();
    // Called every frame, and provides you with a time per frame value
    virtual bool OnUserUpdate(float fElapsedTime);
    // Called once on application termination, so you can be a clean coder
    virtual bool OnUserDestroy();

public: // Hardware Interfaces
    // Returns true if window is currently in focus
    bool IsFocused();
    // Get the state of a specific keyboard button
    HWButton GetKey(Key k);
    // Get the state of a specific mouse button
    HWButton GetMouse(uint32_t b);
    // Get Mouse X coordinate in "pixel" space
    int32_t GetMouseX();
    // Get Mouse Y coordinate in "pixel" space
    int32_t GetMouseY();

public: // Utility
    // Returns the width of the screen in "pixels"
    int32_t ScreenWidth();
    // Returns the height of the screen in "pixels"
    int32_t ScreenHeight();
    // Returns the width of the currently selected drawing target in "pixels"
    int32_t GetDrawTargetWidth();
    // Returns the height of the currently selected drawing target in "pixels"
    int32_t GetDrawTargetHeight();
    // Returns the currently active draw target
    Sprite* GetDrawTarget();

public: // Draw Routines
    // Specify which Sprite should be the target of drawing functions, use nullptr
    // to specify the primary screen
    void SetDrawTarget(Sprite *target);
    // Change the pixel mode for different optimisations
    // olc::Pixel::NORMAL = No transparency
    // olc::Pixel::MASK   = Transparent if alpha is < 255
    // olc::Pixel::ALPHA  = Full transparency
    void SetPixelMode(Pixel::Mode m);
    // Use a custom blend function
    void SetPixelMode(std::function<olc::Pixel(const int x, const int y, const olc::Pixel& pSource, const olc::Pixel& pDest)> pixelMode);
    // Change the blend factor form between 0.0f to 1.0f;
    void SetPixelBlend(float fBlend);
    // Offset texels by sub-pixel amount (advanced, do not use)
    void SetSubPixelOffset(float ox, float oy);

    // Draws a single Pixel
    virtual void Draw(int32_t x, int32_t y, Pixel p = olc::WHITE);
    // Draws a line from (x1,y1) to (x2,y2)
    void DrawLine(int32_t x1, int32_t y1, int32_t x2, int32_t y2, Pixel p = olc::WHITE);
    // Draws a circle located at (x,y) with radius
    void DrawCircle(int32_t x, int32_t y, int32_t radius, Pixel p = olc::WHITE);
    // Fills a circle located at (x,y) with radius
    void FillCircle(int32_t x, int32_t y, int32_t radius, Pixel p = olc::WHITE);
    // Draws a rectangle at (x,y) to (x+w,y+h)
    void DrawRect(int32_t x, int32_t y, int32_t w, int32_t h, Pixel p = olc::WHITE);
    // Fills a rectangle at (x,y) to (x+w,y+h)
    void FillRect(int32_t x, int32_t y, int32_t w, int32_t h, Pixel p = olc::WHITE);
    // Draws a triangle between points (x1,y1), (x2,y2) and (x3,y3)
    void DrawTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Pixel p = olc::WHITE);
    // Flat fills a triangle between points (x1,y1), (x2,y2) and (x3,y3)
    void FillTriangle(int32_t x1, int32_t y1, int32_t x2, int32_t y2, int32_t x3, int32_t y3, Pixel p = olc::WHITE);
    // Draws an entire sprite at location (x,y)
    void DrawSprite(int32_t x, int32_t y, Sprite *sprite, uint32_t scale = 1);
    // Draws an area of a sprite at location (x,y), where the
    // selected area is (ox,oy) to (ox+w,oy+h)
    void DrawPartialSprite(int32_t x, int32_t y, Sprite *sprite, int32_t ox, int32_t oy, int32_t w, int32_t h, uint32_t scale = 1);
    // Draws a single line of text
    void DrawString(int32_t x, int32_t y, std::string sText, Pixel col = olc::WHITE, uint32_t scale = 1);
    // Clears entire draw target to Pixel
    void Clear(Pixel p);

public: // Branding
    std::string sAppName;

private: // Inner mysterious workings
    Sprite		*pDefaultDrawTarget = nullptr;
    Sprite		*pDrawTarget = nullptr;
    Pixel::Mode	nPixelMode = Pixel::NORMAL;
    float		fBlendFactor = 1.0f;
    uint32_t	nScreenWidth = 256;
    uint32_t	nScreenHeight = 240;
    uint32_t	nPixelWidth = 4;
    uint32_t	nPixelHeight = 4;
    uint32_t	nMousePosX = 0;
    uint32_t	nMousePosY = 0;
    float		fPixelX = 1.0f;
    float		fPixelY = 1.0f;
    float		fSubPixelOffsetX = 0.0f;
    float		fSubPixelOffsetY = 0.0f;
    bool		bHasInputFocus = false;
    float		fFrameTimer = 1.0f;
    int			nFrameCount = 0;
    Sprite		*fontSprite = nullptr;
    std::function<olc::Pixel(const int x, const int y, const olc::Pixel&, const olc::Pixel&)> funcPixelMode;

    static std::map<uint16_t, uint8_t> mapKeys;
    bool		pKeyNewState[256]{ 0 };
    bool		pKeyOldState[256]{ 0 };
    HWButton	pKeyboardState[256];

    bool		pMouseNewState[5]{ 0 };
    bool		pMouseOldState[5]{ 0 };
    HWButton	pMouseState[5];

#ifdef _WIN32
    HDC			glDeviceContext = nullptr;
    HGLRC		glRenderContext = nullptr;
#else
    GLXContext	glDeviceContext = nullptr;
    GLXContext	glRenderContext = nullptr;
#endif
    GLuint		glBuffer;

    void		EngineThread();

    // If anything sets this flag to false, the engine
    // "should" shut down gracefully
    static std::atomic<bool> bAtomActive;

    // Common initialisation functions
    void olc_UpdateMouse(uint32_t x, uint32_t y);
    bool olc_OpenGLCreate();
    void olc_ConstructFontSheet();

#ifdef _WIN32
    // Windows specific window handling
    HWND olc_hWnd = nullptr;
    HWND olc_WindowCreate();
    std::wstring wsAppName;
    static LRESULT CALLBACK olc_WindowEvent(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
#else
    // Non-Windows specific window handling
    Display*				olc_Display = nullptr;
    Window					olc_WindowRoot;
    Window					olc_Window;
    XVisualInfo*            olc_VisualInfo;
    Colormap                olc_ColourMap;
    XSetWindowAttributes    olc_SetWindowAttribs;
    Display*				olc_WindowCreate();
#endif

};


class PGEX
{
    friend class olc::PixelGameEngine;
protected:
    static PixelGameEngine* pge;
};

//=============================================================
}

#endif // OLC_PGE_DEF

/*
    Object Oriented Mode
    ~~~~~~~~~~~~~~~~~~~~

    If the olcPixelGameEngine.h is called from several sources it can cause
    multiple definitions of objects. To prevent this, ONLY ONE of the pathways
    to including this file must have OLC_PGE_APPLICATION defined before it. This prevents
    the definitions being duplicated.

    Consider the following project structure:

    Class1.h	- Includes olcPixelGameEngine.h, overrides olc::PixelGameEngine
    Class1.cpp	- #define OLC_PGE_APPLICATION #include "Class1.h"
    Class2.h	- Includes Class1.h, which includes olcPixelGameEngine.h
    Class2.cpp	- #define OLC_PGE_APPLICATION #include "Class2.h"
    main.cpp	- Includes Class1.h and Class2.h

    If all of this is a bit too confusing, you can split this file in two!
    Everything below this comment block can go into olcPixelGameEngineOOP.cpp
    and everything above it can go into olcPixelGameEngineOOP.h

*/
