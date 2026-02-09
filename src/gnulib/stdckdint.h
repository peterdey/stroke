/* Minimal checked-integer helpers for parse-datetime.c.  */

#ifndef STROKE_STDC_KDINT_H
#define STROKE_STDC_KDINT_H 1

#define ckd_add(r, a, b) __builtin_add_overflow ((a), (b), (r))
#define ckd_sub(r, a, b) __builtin_sub_overflow ((a), (b), (r))
#define ckd_mul(r, a, b) __builtin_mul_overflow ((a), (b), (r))

#endif /* STROKE_STDC_KDINT_H */
