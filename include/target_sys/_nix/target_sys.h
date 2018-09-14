#ifndef _SRC__TARGET__TARGET_H_
#define _SRC__TARGET__TARGET_H_

// Несколько defines
// UPD 14_05_2018
//   пропишем в сборщике
// #define _OS_UNIX
// #define _OS_WINDOWS
// #ifdef _OS_WINDOWS

// #endif  // _OS_WINDOWS

#ifdef _OS_UNIX
  #define BOOST_LIB_ISED
#endif // _OS_UNIX


// enable some debug oportunity
// #define _DEBUG
// disable assert()
// #define NDEGUG

#endif  // ! _SRC__TARGET__TARGET_H_