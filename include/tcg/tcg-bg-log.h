#ifndef _TCG_BG_LOG_H_
#define _TCG_BG_LOG_H_

void qemu_set_bglog_filename(const char *filename);
void qemu_bglog_flush(void);
int qemu_bglog(const char *fmt, ...);

#endif
