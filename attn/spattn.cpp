#include <libpdbg.h>

/**
 * A special attention is active so execute a special attention chipop to
 * to retrieve the data needed to process the event. The special attention
 * chipop will take care of clearing the associated status bits.
 *
 * @param i_target FSI to target
 *
 * @return non-zero = error or unsupported
 */
bool handleSpattn(pdbg_target *i_target, bool& i_remove)
{
    bool handleNext = true;

    printf("special: ");
    ( nullptr != i_target ) ? printf("%s\n", pdbg_target_path(i_target)) :
                              printf("invalid FSI target\n");

    i_remove = true;

    return handleNext;
}
