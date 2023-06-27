#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include "latx-perfmap.h"

static FILE *latx_perfmap;

static
void __attribute__((__constructor__)) latx_perfmap_init(void)
{
    char jit_map_file[64];
    sprintf(jit_map_file, "/tmp/perf-%d.map", getpid());
    latx_perfmap = fopen(jit_map_file, "w");
    if (latx_perfmap == NULL) {
        fprintf(stderr, "perfmap open fail. output to stderr.\n");
    }
}

void latx_perfmap_insert(void *start, long len, const char *name)
{
    if (latx_perfmap) {
        fprintf(latx_perfmap, "%p 0x%lx %s\n", start, len, name);
    } else {
        fprintf(stderr, "%p 0x%lx %s\n", start, len, name);
    }
}

void latx_perfmap_flush(void)
{
    if (latx_perfmap) {
        fflush(latx_perfmap);
    }
}
