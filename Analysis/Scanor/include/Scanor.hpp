///@file Scanor.hpp
///@author S. V. Paulauskas, C. R. Thornsberry
///@date September 23, 2016
#ifndef SCANOR_HPP
#define SCANOR_HPP

/// Do some startup stuff.
extern "C" void startup_();

/// Catch the exit call from scanor and clean up c++ objects CRT
extern "C" void cleanup_();

/// DAMM initialization call
extern "C" void drrmake_();

/// DAMM declaration wrap-up call
extern "C" void endrr_();

///@brief Do banana gating using ban files args are the Banana number in the ban
/// file the x-value to test, and the y-value to test.
/// @return true if the x,y pair is inside the banana gate */
extern "C" bool bantesti_(const int &, const int &, const int &);

///@brief create a DAMM 1D histogram
/// args are damm id, half-words per channel, param length, hist length, low
/// x-range, high x-range, and title
extern "C" void hd1d_(const int &, const int &, const int &, const int &,
                      const int &, const int &, const char *, int);

///@brief  create a DAMM 2D histogram
// args are damm id, half-words per channel, x-param length, x-hist length
// low x-range, high x-range, y-param length, y-hist length, low y-range high
// y-range, and title
extern "C" void hd2d_(const int &, const int &, const int &, const int &,
                      const int &, const int &, const int &, const int &,
                      const int &, const int &, const char *, int);

///@brief Defines the DAMM function to call for 1D hists */
extern "C" void count1cc_(const int &, const int &, const int &);

///@brief Defines the DAMM function to call for 2D hists */
extern "C" void set2cc_(const int &, const int &, const int &, const int &);

#endif //#ifndef SCANOR_HPP
