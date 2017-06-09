///@authors D. Miller, K. Smith
// a few miscellaneous functions

#ifndef __UTILITY_H_
#define __UTILITY_H_ 1

/** get time in microseconds relative to par */
double usGetTime(double par);

/** get time in microseconds since last call to this function */
double usGetDTime(void);

/** test if a value t falls in the closed range [low, high) */
template<typename T>
bool inRange(const T &t, const T &low, const T &high) {
    // use only less than operator so that only one operator needs to be defined
    return (!(t < low) && t < high);
}

/** test if a value t falls in the range between [default value, high) */
template<typename T>
bool inRange(const T &t, const T &high) {
    // use only less than operator so that only one operator needs to be defined
    return (!(t < T()) && t < high);
}


#endif // __UTILITY_H_
