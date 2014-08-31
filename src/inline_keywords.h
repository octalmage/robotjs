#pragma once

/* A complicated, portable model for declaring inline functions in
 * header files. */
#if !defined(H_INLINE)
    #if defined(__GNUC__)
        #define H_INLINE static __inline__ __attribute__((always_inline))
    #elif defined(__MWERKS__) || defined(__cplusplus)
        #define H_INLINE static inline
    #elif defined(_MSC_VER)
        #define H_INLINE static __inline
    #elif TARGET_OS_WIN32
        #define H_INLINE static __inline__
    #endif
#endif /* H_INLINE */
