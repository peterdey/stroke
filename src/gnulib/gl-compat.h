/* Minimal gnulib compatibility shims for vendored parse-datetime.c.
 *
 * These provide the handful of _GL_* macros that gnulib headers expect
 * without requiring a full gnulib bootstrap.
 */

#ifndef STROKE_GL_COMPAT_H
#define STROKE_GL_COMPAT_H 1

#include <string.h>

#ifndef _GL_CONFIG_H_INCLUDED
# define _GL_CONFIG_H_INCLUDED 1
#endif

#ifndef _GL_INLINE_HEADER_BEGIN
# ifdef __cplusplus
#  define _GL_INLINE_HEADER_BEGIN extern "C" {
#  define _GL_INLINE_HEADER_END }
# else
#  define _GL_INLINE_HEADER_BEGIN
#  define _GL_INLINE_HEADER_END
# endif
#endif

#ifndef _GL_INLINE
# define _GL_INLINE static inline
#endif

#ifndef _GL_ATTRIBUTE_CONST
# if defined __GNUC__ && 2 < __GNUC__ + (__GNUC_MINOR__ >= 5)
#  define _GL_ATTRIBUTE_CONST __attribute__((__const__))
# else
#  define _GL_ATTRIBUTE_CONST
# endif
#endif

#ifndef _GL_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (__GNUC_MINOR__ >= 96)
#  define _GL_ATTRIBUTE_PURE __attribute__((__pure__))
# else
#  define _GL_ATTRIBUTE_PURE
# endif
#endif

#ifndef _GL_ATTRIBUTE_FORMAT
# if defined __GNUC__ && 2 < __GNUC__ + (__GNUC_MINOR__ >= 5)
#  define _GL_ATTRIBUTE_FORMAT(spec) __attribute__((__format__ spec))
# else
#  define _GL_ATTRIBUTE_FORMAT(spec)
# endif
#endif

#ifndef _GL_UNUSED
# if defined __GNUC__ || defined __clang__
#  define _GL_UNUSED __attribute__((__unused__))
# else
#  define _GL_UNUSED
# endif
#endif

#ifndef _GL_CMP
# define _GL_CMP(a, b) (((a) > (b)) - ((a) < (b)))
#endif

#ifndef _GL_ARG_NONNULL
# define _GL_ARG_NONNULL(params)
#endif

#ifndef GNULIB_TEXT_DOMAIN
# define GNULIB_TEXT_DOMAIN PACKAGE
#endif

#ifndef static_assert
# define static_assert(cond) _Static_assert(cond, #cond)
#endif

#ifndef streq
# define streq(a, b) (strcmp((a), (b)) == 0)
#endif

#endif /* STROKE_GL_COMPAT_H */
