#pragma once
#include <tiffio.h>

#if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
#else
    #define DLL_PUBLIC
#endif
  
#ifdef __cplusplus
extern "C"
{
#endif
    DLL_PUBLIC thandle_t remotetiff_open(const char*);
    DLL_PUBLIC tsize_t remotetiff_read(thandle_t, tdata_t, tsize_t);
    DLL_PUBLIC toff_t remotetiff_seek(thandle_t, toff_t, int);
    DLL_PUBLIC int remotetiff_close(thandle_t);
    DLL_PUBLIC toff_t remotetiff_size(thandle_t);
#ifdef __cplusplus
}
#endif