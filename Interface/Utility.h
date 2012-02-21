// a few miscellaneous functions

#ifndef __UTILITY_H_
#define __UTILITY_H_ 1

/** get time in microseconds relative to par */
double usGetTime(double par);
/** get time in microseconds since last call to this function */
double usGetDTime(void);

/** test if a value t falls in the closed range [low, high) */
template<typename T>
bool inRange(const T& t, const T& low, const T& high) {
    return (t >= low && t < high);
};


#endif // __UTILITY_H_
