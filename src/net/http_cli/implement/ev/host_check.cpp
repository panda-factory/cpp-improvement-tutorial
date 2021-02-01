//
// Created by admin on 2021/2/1.
//

#include "host_check.h"
#include <string.h>

/* Portable, consistent toupper (remember EBCDIC). Do not use toupper() because
   its behavior is altered by the current locale. */
static char Curl_raw_toupper(char in)
{
    switch (in) {
        case 'a':
            return 'A';
        case 'b':
            return 'B';
        case 'c':
            return 'C';
        case 'd':
            return 'D';
        case 'e':
            return 'E';
        case 'f':
            return 'F';
        case 'g':
            return 'G';
        case 'h':
            return 'H';
        case 'i':
            return 'I';
        case 'j':
            return 'J';
        case 'k':
            return 'K';
        case 'l':
            return 'L';
        case 'm':
            return 'M';
        case 'n':
            return 'N';
        case 'o':
            return 'O';
        case 'p':
            return 'P';
        case 'q':
            return 'Q';
        case 'r':
            return 'R';
        case 's':
            return 'S';
        case 't':
            return 'T';
        case 'u':
            return 'U';
        case 'v':
            return 'V';
        case 'w':
            return 'W';
        case 'x':
            return 'X';
        case 'y':
            return 'Y';
        case 'z':
            return 'Z';
    }
    return in;
}

/*
 * Curl_raw_equal() is for doing "raw" case insensitive strings. This is meant
 * to be locale independent and only compare strings we know are safe for
 * this.  See http://daniel.haxx.se/blog/2008/10/15/strcasecmp-in-turkish/ for
 * some further explanation to why this function is necessary.
 *
 * The function is capable of comparing a-z case insensitively even for
 * non-ascii.
 */

static int Curl_raw_equal(const char *first, const char *second)
{
    while(*first && *second) {
        if(Curl_raw_toupper(*first) != Curl_raw_toupper(*second))
            /* get out of the loop as soon as they don't match */
            break;
        first++;
        second++;
    }
    /* we do the comparison here (possibly again), just to make sure that if the
       loop above is skipped because one of the strings reached zero, we must not
       return this as a successful match */
    return (Curl_raw_toupper(*first) == Curl_raw_toupper(*second));
}

static int Curl_raw_nequal(const char *first, const char *second, size_t max)
{
    while(*first && *second && max) {
        if(Curl_raw_toupper(*first) != Curl_raw_toupper(*second)) {
            break;
        }
        max--;
        first++;
        second++;
    }
    if(0 == max)
        return 1; /* they are equal this far */

    return Curl_raw_toupper(*first) == Curl_raw_toupper(*second);
}

/*
 * Match a hostname against a wildcard pattern.
 * E.g.
 *  "foo.host.com" matches "*.host.com".
 *
 * We use the matching rule described in RFC6125, section 6.4.3.
 * http://tools.ietf.org/html/rfc6125#section-6.4.3
 */

static int hostmatch(const char *hostname, const char *pattern)
{
    const char *pattern_label_end, *pattern_wildcard, *hostname_label_end;
    int wildcard_enabled;
    size_t prefixlen, suffixlen;
    pattern_wildcard = strchr(pattern, '*');
    if(pattern_wildcard == NULL)
        return Curl_raw_equal(pattern, hostname) ?
               CURL_HOST_MATCH : CURL_HOST_NOMATCH;

    /* We require at least 2 dots in pattern to avoid too wide wildcard
       match. */
    wildcard_enabled = 1;
    pattern_label_end = strchr(pattern, '.');
    if(pattern_label_end == NULL || strchr(pattern_label_end+1, '.') == NULL ||
       pattern_wildcard > pattern_label_end ||
       Curl_raw_nequal(pattern, "xn--", 4)) {
        wildcard_enabled = 0;
    }
    if(!wildcard_enabled)
        return Curl_raw_equal(pattern, hostname) ?
               CURL_HOST_MATCH : CURL_HOST_NOMATCH;

    hostname_label_end = strchr(hostname, '.');
    if(hostname_label_end == NULL ||
       !Curl_raw_equal(pattern_label_end, hostname_label_end))
        return CURL_HOST_NOMATCH;

    /* The wildcard must match at least one character, so the left-most
       label of the hostname is at least as large as the left-most label
       of the pattern. */
    if(hostname_label_end - hostname < pattern_label_end - pattern)
        return CURL_HOST_NOMATCH;

    prefixlen = pattern_wildcard - pattern;
    suffixlen = pattern_label_end - (pattern_wildcard+1);
    return Curl_raw_nequal(pattern, hostname, prefixlen) &&
           Curl_raw_nequal(pattern_wildcard+1, hostname_label_end - suffixlen,
                           suffixlen) ?
           CURL_HOST_MATCH : CURL_HOST_NOMATCH;
}

int Curl_cert_hostcheck(const char *match_pattern, const char *hostname)
{
    if(!match_pattern || !*match_pattern ||
       !hostname || !*hostname) /* sanity check */
        return 0;

    if(Curl_raw_equal(hostname, match_pattern)) /* trivial case */
        return 1;

    if(hostmatch(hostname,match_pattern) == CURL_HOST_MATCH)
        return 1;
    return 0;
}