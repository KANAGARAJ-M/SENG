
#ifndef SENG_COMMON_H
#define SENG_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>

#define SENG_VERSION "1.0.2"
#define SENG_MAGIC   "SENG"
#define SENG_SEC_VER  1

/* ── memory ──────────────────────────────────────────────────── */
static inline void *xmalloc(size_t n) {
    void *p = malloc(n);
    if (!p) { fprintf(stderr, "seng: out of memory\n"); exit(1); }
    return p;
}
static inline void *xcalloc(size_t n, size_t s) {
    void *p = calloc(n, s);
    if (!p) { fprintf(stderr, "seng: out of memory\n"); exit(1); }
    return p;
}
static inline void *xrealloc(void *p, size_t n) {
    void *q = realloc(p, n);
    if (!q) { fprintf(stderr, "seng: out of memory\n"); exit(1); }
    return q;
}
static inline char *xstrdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *r = (char *)xmalloc(n);
    memcpy(r, s, n);
    return r;
}
static inline char *xstrndup(const char *s, size_t n) {
    char *r = (char *)xmalloc(n + 1);
    memcpy(r, s, n);
    r[n] = '\0';
    return r;
}

/* ── I/O ─────────────────────────────────────────────────────── */
char *read_file(const char *path);

/* ── error ───────────────────────────────────────────────────── */
void fatal(const char *fmt, ...);

#endif /* SENG_COMMON_H */
