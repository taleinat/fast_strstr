#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

/* Endian detection, taken from: http://esr.ibiblio.org/?p=5095 */
/*
   __BIG_ENDIAN__ and __LITTLE_ENDIAN__ are define in some gcc versions
  only, probably depending on the architecture. Try to use endian.h if
  the gcc way fails - endian.h also doesn not seem to be available on all
  platforms.
*/

#if defined(__APPLE__)
    /* See: https://gist.github.com/yinyin/2027912
       Or search for: OSSwapHostToBigInt64 */
    #include <libkern/OSByteOrder.h>
    #define htobe64(x) OSSwapHostToBigInt64(x)
    #define htonl(x) OSSwapHostToBigInt32(x)
#elif defined(BSD)
    #if defined(__OpenBSD__)
        #include <sys/types.h>
    #else
        #include <sys/endian.h>
    #endif
#else
    #include <endian.h>
#endif

#if defined(__BIG_ENDIAN__)
    #define WORDS_BIGENDIAN 1
#elif defined(__LITTLE_ENDIAN__)
    #undef WORDS_BIGENDIAN
#elif defined(_WIN32)
    #undef WORDS_BIGENDIAN
#elif defined(__BYTE_ORDER) && defined(__BIG_ENDIAN) && defined(__LITTLE_ENDIAN)
    #if __BYTE_ORDER == __BIG_ENDIAN
        #define WORDS_BIGENDIAN 1
    #elif __BYTE_ORDER == __LITTLE_ENDIAN
        #undef WORDS_BIGENDIAN
    #else
        #define UNKNOWN_ENDIANNESS 1
    #endif /* __BYTE_ORDER */
#else
    #define UNKNOWN_ENDIANNESS 1
#endif

#if LONG_MAX == 2147483647
    #define LONG_INT_IS_4_BYTES 1
    #define LONG_INT_N_BYTES 4
#elif LONG_MAX == 9223372036854775807
    #define LONG_INT_IS_8_BYTES 1
    #define LONG_INT_N_BYTES 8
#else
    #define LONG_INT_IS_UNSUPPORTED_SIZE
    #define LONG_INT_N_BYTES sizeof(long)
#endif

/* define MAKE_BIGENDIAN */
#if !defined(UNKNOWN_ENDIANNESS) && defined(WORDS_BIGENDIAN)
    #define MAKE_ULONG_BIGENDIAN(x) (x)
#else
    #if defined(LONG_INT_IS_8_BYTES)
        #define MAKE_ULONG_BIGENDIAN(x) htobe64((x))
    #elif defined(LONG_INT_IS_4_BYTES)
        #define MAKE_ULONG_BIGENDIAN(x) htonl((x))
    #else
        #undef MAKE_ULONG_BIGENDIAN
    #endif
#endif /* !defined(UNKNOWN_ENDIANNESS) && defined(WORDS_BIGENDIAN) */


#undef UNKNOWN_ENDIANNESS
#undef WORDS_BIGENDIAN
#undef LONG_INT_IS_4_BYTES
#undef LONG_INT_IS_8_BYTES
#undef LONG_INT_IS_UNSUPPORTED_SIZE


/**
 * Finds the first occurrence of the sub-string needle in the string haystack.
 * Returns NULL if needle was not found.
 */
void *wordlen_strstr(const char *haystack, const char *needle)
{
    if (!*needle) // Empty needle.
        return (char *) haystack;

    const char    needle_first  = *needle;

    // Runs strchr() on the first section of the haystack as it has a lower
    // algorithmic complexity for discarding the first non-matching characters.
    haystack = strchr(haystack, needle_first);
    if (!haystack) // First character of needle is not in the haystack.
        return NULL;

    // First characters of haystack and needle are the same now. Both are
    // guaranteed to be at least one character long.
    // Now computes the sum of the first needle_len characters of haystack
    // minus the sum of characters values of needle.

    const char   *i_haystack    = haystack + 1
             ,   *i_needle      = needle   + 1;
    bool          identical     = true;

    while (*i_haystack && *i_needle) {
        identical &= *i_haystack++ == *i_needle++;
    }

    // i_haystack now references the (needle_len + 1)-th character.

    if (*i_needle) // haystack is smaller than needle.
        return NULL;
    else if (identical)
        return (char *) haystack;

    size_t        needle_len    = i_needle - needle;

    // Note: needle_len > 1, because we checked that it isn't zero, and if it
    //       is 1 then identical must be true because the first strchr() ensured
    //       that the first characters are identical

    const char   *sub_start = haystack;
    int compare_len;
    unsigned long last_needle_chars;
    unsigned long last_haystack_chars;
    unsigned long mask;

#ifdef MAKE_ULONG_BIGENDIAN
    last_needle_chars = MAKE_ULONG_BIGENDIAN(*(((unsigned long *)i_needle) - 1));
    last_haystack_chars = MAKE_ULONG_BIGENDIAN(*(((unsigned long *)i_haystack) - 1));
#else
    const char   *needle_end    = i_needle;
    size_t        min_cmp_len   = (needle_len < LONG_INT_N_BYTES) ? needle_len : LONG_INT_N_BYTES
    i_needle -= min_cmp_len;
    i_haystack -= min_cmp_len;
    last_needle_chars = 0;
    last_haystack_chars = 0;
    while (i_needle != needle_end) {
        last_needle_chars <<= 8;
        last_needle_chars ^= *i_needle++;
        last_haystack_chars <<= 8;
        last_haystack_chars ^= *i_haystack++;
    }
#endif

    // At this point:
    // * needle is at least two characters long
    // * haystack is at least needle_len characters long (also at least two)
    // * the first characters of needle and haystack are identical

    if (needle_len > LONG_INT_N_BYTES + 1)
    {
        /* we will call memcmp() only once we know that the LONG_INT_N_BYTES
           last chars are equal, so it will be enough to compare all but the
           last LONG_INT_N_BYTES characters */
        compare_len = needle_len - LONG_INT_N_BYTES;

        /* iterate through the remainder of haystack while checking for identity
           of the last LONG_INT_N_BYTES, and checking the rest with memcmp() */
        while (*i_haystack)
        {
            last_haystack_chars <<= 8;
            last_haystack_chars ^= *(unsigned char *)i_haystack++;

            if (   last_haystack_chars == last_needle_chars
                && memcmp(sub_start, needle, compare_len) == 0)
            {
                return (void *) sub_start;
            }
            sub_start++;
        }
    }
    else if (needle_len == LONG_INT_N_BYTES + 1)
    {
        /* iterate through the remainder of haystack while checking for identity
           of the last LONG_INT_N_BYTES as well as the single additional
           character, which is the first one */
        while (*i_haystack)
        {
            last_haystack_chars <<= 8;
            last_haystack_chars ^= *(unsigned char *)i_haystack++;

            if (   last_haystack_chars == last_needle_chars
                && *sub_start == needle_first)
            {
                return (void *) sub_start;
            }
            sub_start++;
        }
    }
    else if (needle_len == LONG_INT_N_BYTES)
    {
        /* iterate through the remainder of haystack while checking for identity
           of the last LONG_INT_N_BYTES characters, which should exactly match
           the entire needle */
        while (*i_haystack)
        {
            last_haystack_chars <<= 8;
            last_haystack_chars ^= *(unsigned char *)i_haystack++;

            if (last_haystack_chars == last_needle_chars)
            {
                return (void *) (i_haystack - needle_len);
            }
        }
    }
    else /* needle_len < LONG_INT_N_BYTES */
    {
        mask = (((unsigned long) 1) << (needle_len * 8)) - 1;
        last_needle_chars &= mask;

        /* iterate through the remainder of haystack, updating the sums' difference
           and checking for identity whenever the difference is zero */
        while (*i_haystack)
        {
            last_haystack_chars <<= 8;
            last_haystack_chars ^= *(unsigned char *)i_haystack++;
            last_haystack_chars &= mask;

            /* if sums_diff == 0, we know that the sums are equal, so it is enough
               to compare all but the last characters */
            if (last_haystack_chars == last_needle_chars)
            {
                return (void *) (i_haystack - needle_len);
            }
        }
    }

    return NULL;
}


#undef LONG_INT_N_BYTES
#undef MAKE_ULONG_BIGENDIAN
