#ifdef __WIN32__

/* ---------------------------------------------------------------------------
 * wait_anykey()
 * ---------------------------------------------------------------------------
 * Copyright 2008 Michael Thomas Greer
 * http://www.boost.org/LICENSE_1_0.txt
 *
 * function
 *   Optionally print a message and and wait for the user to press (and
 *   release) a single key.
 *
 * arguments
 *   The message to print. If NULL, uses a default message. Specify the empty
 *   string "" to not print anything.
 *
 * returns
 *   The virtual keycode for the key that was pressed.
 *
 *   Windows #defines virtual keycode values like
 *     VK_UP
 *     VK_DOWN
 *     VK_RIGHT
 *     VK_LEFT
 *   which you can use to identify special keys.
 *
 *   Letter keys are simply the upper-case ASCII value for that letter.
 */

#include <windows.h>
#include <string>

namespace sss {
namespace debug {

int wait_anykey(const std::string & prompt_str)
{
    const char * prompt = prompt_str.c_str();
    DWORD        mode;
    HANDLE       hstdin;
    INPUT_RECORD inrec;
    DWORD        count;
    char         default_prompt[] = "Press a key to continue...";

    /* Set the console mode to no-echo, raw input, */
    /* and no window or mouse events.              */
    hstdin = GetStdHandle( STD_INPUT_HANDLE );
    if (hstdin == INVALID_HANDLE_VALUE
        || !GetConsoleMode( hstdin, &mode )
        || !SetConsoleMode( hstdin, 0 ))
        return 0;

    if (!prompt) prompt = default_prompt;

    /* Instruct the user */
    WriteConsole(
        GetStdHandle( STD_OUTPUT_HANDLE ),
        prompt,
        lstrlen( prompt ),
        &count,
        NULL
        );

    FlushConsoleInputBuffer( hstdin );

    /* Get a single key RELEASE */
    do ReadConsoleInput( hstdin, &inrec, 1, &count );
    while ((inrec.EventType != KEY_EVENT) || inrec.Event.KeyEvent.bKeyDown);

    /* Restore the original console mode */
    SetConsoleMode( hstdin, mode );

    return inrec.Event.KeyEvent.wVirtualKeyCode;
}

} // namespace debug
} // namespace sss
#else
// POSIX (Unix, Linux, Mac OSX, etc)

/* ---------------------------------------------------------------------------
 * wait_anykey()
 * ---------------------------------------------------------------------------
 * Copyright 2008 Michael Thomas Greer
 * http://www.boost.org/LICENSE_1_0.txt
 *
 * function
 *   Optionally print a message and and wait for the user to press (and
 *   release) a single key.
 *
 * arguments
 *   The message to print. If NULL, uses a default message. Specify the empty
 *   string "" to not print anything.
 *
 * returns
 *   The keycode for the key that was pressed.
 *
 *   Extended key codes (like arrow keys) are properly handled, but their
 *   keycode is not understood; they are simply returned as the last code in
 *   the sequence, negated. For example, it is likely that the arrow keys are:
 *
 *     UP_ARROW    = -'A' = -65
 *     DOWN_ARROW  = -'B' = -66
 *     RIGHT_ARROW = -'C' = -67
 *     LEFT_ARROW  = -'D' = -68
 *
 *   Exactly identifying the values for these keys requires a foray into the
 *   terminfo database, which is a subject for later. For now we'll leave it
 *   at this.
 */
#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <string>

namespace sss {
namespace debug {

int wait_anykey(const std::string & prompt_str)
{
#define MAGIC_MAX_CHARS 18
    struct termios initial_settings;
    struct termios settings;
    unsigned char  keycodes[ MAGIC_MAX_CHARS ];
    int            count;

    tcgetattr( STDIN_FILENO, &initial_settings );
    settings = initial_settings;

    /* Set the console mode to no-echo, raw input. */
    /* The exact meaning of all this jazz will be discussed later. */
    settings.c_cc[ VTIME ] = 1;
    settings.c_cc[ VMIN  ] = MAGIC_MAX_CHARS;
    settings.c_iflag &= ~(IXOFF);
    settings.c_lflag &= ~(ECHO | ICANON);
    tcsetattr( STDIN_FILENO, TCSANOW, &settings );

    const char * prompt = prompt_str.c_str();
    printf( "%s", prompt ? prompt : "Press a key to continue..." );
    count = ::read( STDIN_FILENO, (void*)keycodes, MAGIC_MAX_CHARS );
    //extern ssize_t read (int __fd, void *__buf, size_t __nbytes) __wur;

    tcsetattr( STDIN_FILENO, TCSANOW, &initial_settings );

    return (count == 1)
        ? keycodes[ 0 ]
        : -(int)(keycodes[ count -1 ]);
}
} // namespace debug
} // namespace sss
#endif
