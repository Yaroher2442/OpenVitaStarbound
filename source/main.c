#include "utils/init.h"
#include "utils/glutil.h"

#include <psp2/kernel/threadmgr.h>

#include <so_util/so_util.h>

int _newlib_heap_size_user = 256 * 1024 * 1024;

#ifdef USE_SCELIBC_IO
int sceLibcHeapSize = 4 * 1024 * 1024;
#endif

so_module so_mod;

int main() {
    soloader_init_all();
    gl_init();

    // ... do some initialization

    while (1) {
        // ... render call
        gl_swap();
    }

    sceKernelExitDeleteThread(0);
}
