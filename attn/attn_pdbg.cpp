#include <libpdbg.h>

// return count of targets in class
static int count_class_target(const char *classname)
{
    struct pdbg_target *target;
    int n = 0;

    pdbg_for_each_class_target(classname, target) {
        n++;
    }

    return n;
}

// simple test of libpdbg interface
int testLibPdbg()
{
    int count;

    pdbg_set_backend(PDBG_BACKEND_FSI, "p9w");
    printf("using backend FSI, p9w\n");

    pdbg_targets_init(NULL);

    count = count_class_target("fsi");
    printf("   fsi count: %i\n", count);

    count = count_class_target("pib");
    printf("   pib count: %i\n", count);

    count = count_class_target("core");
    printf("  core count: %i\n", count);

    count = count_class_target("thread");
    printf("thread count: %i\n", count);

    return 0;
}
