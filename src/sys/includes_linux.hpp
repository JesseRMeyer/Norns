#include <stdlib.h> // free
#include <xcb/xcb.h>
#include <xcb/xcb_keysyms.h>
#include <xcb/present.h>
#include <xcb/shm.h>
#include <xcb/xcb_util.h>
#include <sys/shm.h>
#include <pthread.h>
#include <signal.h> //pthread_kill
#include <errno.h> // ESRCH
#include <linux/futex.h>
#include <sys/syscall.h>
#include <unistd.h> //NOTE(Jesse): syscall
#include <time.h> //NOTE(Jesse): clock_gettime, nanosleep
#include <sys/mman.h> //mmap
#include <errno.h>
#include <string.h> //strerror_r()