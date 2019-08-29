#ifndef BASE_STRING_STRING_H
#define BASE_STRING_STRING_H

#include <stdarg.h>
#include <inttypes.h>
#include <stdlib.h>

//----------------------------------------------------------------

#if 0
unsigned dm_count_chars(const char *str, size_t len, const int c);
size_t dm_escaped_len(const char *str);
char *dm_escape_double_quotes(char *out, const char *src);
void dm_unescape_double_quotes(char *src);
#endif

/* Define some portable printing types */
#define PRIsize_t "zu"
#define PRIssize_t "zd"
#define PRIptrdiff_t "td"
#define PRIpid_t PRId32

/* For convenience */
#define FMTsize_t "%" PRIsize_t
#define FMTssize_t "%" PRIssize_t
#define FMTptrdiff_t "%" PRIptrdiff_t
#define FMTpid_t "%" PRIpid_t

#define FMTd8  "%" PRId8
#define FMTd16 "%" PRId16
#define FMTd32 "%" PRId32
#define FMTd64 "%" PRId64

#define FMTi8  "%" PRIi8
#define FMTi16 "%" PRIi16
#define FMTi32 "%" PRIi32
#define FMTi64 "%" PRIi64

#define FMTo8  "%" PRIo8
#define FMTo16 "%" PRIo16
#define FMTo32 "%" PRIo32
#define FMTo64 "%" PRIo64

#define FMTu8  "%" PRIu8
#define FMTu16 "%" PRIu16
#define FMTu32 "%" PRIu32
#define FMTu64 "%" PRIu64

#define FMTx8  "%" PRIx8
#define FMTx16 "%" PRIx16
#define FMTx32 "%" PRIx32
#define FMTx64 "%" PRIx64

#define FMTVGID "%." DM_TO_STRING(ID_LEN) "s"

//----------------------------------------------------------------

#endif
