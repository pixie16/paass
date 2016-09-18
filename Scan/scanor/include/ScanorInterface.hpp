#ifndef SCANOR_INTERFACE_HPP
#define SCANOR_INTERFACE_HPP

//! DAMM initialization call
extern "C" void drrmake_();

//! DAMM declaration wrap-up call
extern "C" void endrr_();

/** Do banana gating using ban files args are the Banana number in the ban file,
 * the x-value to test, and the y-value to test.
 * \return true if the x,y pair is inside the banana gate */
extern "C" bool bantesti_(const int &, const int &, const int &);

/** create a DAMM 1D histogram
 * args are damm id, half-words per channel, param length, hist length,
 * low x-range, high x-range, and title
 */
extern "C" void hd1d_(const int &, const int &, const int &, const int &,
		      const int &, const int &, const char *, int);

/** create a DAMM 2D histogram
 * args are damm id, half-words per channel, x-param length, x-hist length
 * low x-range, high x-range, y-param length, y-hist length, low y-range
 * high y-range, and title
 */
extern "C" void hd2d_(const int &, const int &, const int &, const int &,
		      const int &, const int &, const int &, const int &,
		      const int &, const int &, const char *, int);
		      
/** Do banana gating using ban files args are the Banana number in the ban file,
 * the x-value to test, and the y-value to test.
 * \return true if the x,y pair is inside the banana gate */
extern "C" bool bantesti_(const int &, const int &, const int &);

/** Defines the DAMM function to call for 1D hists */
extern "C" void count1cc_(const int &, const int &, const int &);

/** Defines the DAMM function to call for 2D hists */
extern "C" void set2cc_(const int &, const int &, const int &, const int &);

#endif