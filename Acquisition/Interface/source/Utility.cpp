///@authors D. Miller

#include <sys/time.h>

#include "Utility.h"

double usGetTime(double par) {
    timeval cur_time;

    gettimeofday(&cur_time, 0);

    return double(1.0e6 * cur_time.tv_sec) + double(cur_time.tv_usec) - par;
}

double usGetDTime() {
    static timeval last;
    static bool first = true;

    if (first) {
        gettimeofday(&last, 0);
        first = false;
        return 0.;
    }

    timeval current;
    gettimeofday(&current, 0);

    double dt = 1.0e6 * (current.tv_sec - last.tv_sec);
    dt += current.tv_usec - last.tv_usec;

    last = current;
    return dt;
}
