#ifndef URG3DDETECTOS_H
#define URG3DDETECTOS_H

#if defined(_WIN32)
#define URG3D_WINDOWS_OS

#ifndef M_PI
//! pi for Visual C++ 6.0
#define M_PI 3.14159265358979323846
#endif

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define URG3D_MSC
#endif

#elif defined(__linux__)
#define URG3D_LINUX_OS

#else
//! if os type is not detected by above defines, set as Mac OS
#define URG3D_MAC_OS
#endif

#endif
