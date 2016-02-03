#ifndef _popen2_h_
#define _popen2_h_
#include <sys/types.h>

pid_t popen2(char *command, int *infp, int *outfp);
int pclose2(pid_t pid);
#endif
