#ifndef __NANO_TIMER_HPP_1474940229__
#define __NANO_TIMER_HPP_1474940229__

namespace sss {
namespace time {
#ifdef __WIN32__
#include <windows.h>
#else
#include <time.h>
#endif
#include <cstring>

class nano_timer {
private:
#ifdef __WIN32__
    LARGE_INTEGER StartingTime;
    LARGE_INTEGER Frequency;
#else
    struct timespec _start_time;
#endif

public:
    nano_timer() { std::memset(&_start_time, 0, sizeof(_start_time)); }
    void start()
    {
#ifdef __WIN32__
        ::QueryPerformanceFrequency(&Frequency);
        ::QueryPerformanceCounter(&StartingTime);
#else
        ::clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &_start_time);
#endif
    }
    long get_elapsed_time() const
    {
#ifdef __WIN32__
        LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;

        ::QueryPerformanceCounter(&EndingTime);
        ElapsedMicroseconds.QuadPart =
            EndingTime.QuadPart - StartingTime.QuadPart;

        ElapsedMicroseconds.QuadPart *= 1000000;
        ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

        return ElapsedMicroseconds.QuadPart;
#else
        struct timespec end_time;
        clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
        return end_time.tv_nsec - _start_time.tv_nsec;
#endif
    }
};

// http://stackoverflow.com/questions/6749621/how-to-create-a-high-resolution-timer-in-linux-to-measure-program-performance
// call this function to start a nanosecond-resolution timer
// inline struct timespec timer_start()
// {
//     struct timespec start_time;
//     clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start_time);
//     return start_time;
// }
//
// // call this function to end a timer, returning nanoseconds elapsed as a long
// inline long timer_end(struct timespec start_time)
// {
//     struct timespec end_time;
//     clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &end_time);
//     long diffInNanos = end_time.tv_nsec - start_time.tv_nsec;
//     return diffInNanos;
// }

// struct timespec vartime = timer_start();  // begin a timer called 'vartime'
// double variance = var(input, MAXLEN);  // perform the task we want to time
// long time_elapsed_nanos = timer_end(vartime);
// printf("Variance = %f, Time taken (nanoseconds): %ld\n", variance,
// time_elapsed_nanos);
//
//! https://msdn.microsoft.com/en-us/library/windows/desktop/dn553408(v=vs.85).aspx
// LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
// LARGE_INTEGER Frequency;
//
// QueryPerformanceFrequency(&Frequency);
// QueryPerformanceCounter(&StartingTime);
//
// // Activity to be timed
//
// QueryPerformanceCounter(&EndingTime);
// ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;

//
// We now have the elapsed number of ticks, along with the
// number of ticks-per-second. We use these values
// to convert to the number of elapsed microseconds.
// To guard against loss-of-precision, we convert
// to microseconds *before* dividing by ticks-per-second.
//

// ElapsedMicroseconds.QuadPart *= 1000000;
// ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
//
//! Another solution:
//! http://stackoverflow.com/questions/1825720/c-high-precision-time-measurement-in-windows
// #include <type_traits>
// #include <chrono>
//
//
// class Stopwatch final
// {
// public:
//
//     using elapsed_resolution = std::chrono::milliseconds;
//
//     Stopwatch()
//     {
//         Reset();
//     }
//
//     void Reset()
//     {
//         reset_time = clock.now();
//     }
//
//     elapsed_resolution Elapsed()
//     {
//         return std::chrono::duration_cast<elapsed_resolution>(clock.now() -
//         reset_time);
//     }
//
// private:
//
//     std::chrono::high_resolution_clock clock;
//     std::chrono::high_resolution_clock::time_point reset_time;
// };
}  // namespace
}  // namespace sss

#endif /* __NANO_TIMER_HPP_1474940229__ */
