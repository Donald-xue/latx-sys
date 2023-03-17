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
}

void latx_perfmap_insert(void *start, long len, const char *name)
{
    fprintf(latx_perfmap, "%p 0x%lx %s\n",
            start, len, name);
}

void latx_perfmap_flush(void)
{
    fflush(latx_perfmap);
}
