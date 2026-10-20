/* POSIX calls Picori makes (for desktop features: update checks through
 * curl, shader tools, crash reports) that VitaSDK's newlib does not have.
 * Weak, so a real one in the SDK wins. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

__attribute__((weak)) FILE* popen(const char* command, const char* mode)
{
    (void)command;
    (void)mode;
    errno = ENOSYS;
    return NULL;
}

__attribute__((weak)) int pclose(FILE* f)
{
    (void)f;
    return -1;
}

/* No /proc/self/exe: Picori then looks in the current folder (ux0:data/tmc). */
__attribute__((weak)) ssize_t readlink(const char* path, char* buf, size_t size)
{
    (void)path;
    (void)buf;
    (void)size;
    errno = ENOSYS;
    return -1;
}

/* Paths are used as they are (no links to resolve on the Vita). */
__attribute__((weak)) char* realpath(const char* path, char* resolved)
{
    if (!path) {
        errno = EINVAL;
        return NULL;
    }
    if (!resolved) {
        resolved = (char*)malloc(strlen(path) + 1);
        if (!resolved)
            return NULL;
    }
    strcpy(resolved, path);
    return resolved;
}

__attribute__((weak)) int fsync(int fd)
{
    (void)fd;
    return 0;
}
