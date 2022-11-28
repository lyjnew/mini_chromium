#ifndef WINLIB_WINBASE_WIN_NOMINMAX_H
#define WINLIB_WINBASE_WIN_NOMINMAX_H

#if defined(min)
#undef min
#endif

#if defined(max)
#undef max
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif  // NOMINMAX

#endif  // WINLIB_WINBASE_WIN_NOMINMAX_H