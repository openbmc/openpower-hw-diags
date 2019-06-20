#include <attn_pdbg.hpp>

#include <libpdbg.h>

#include <assert.h>
#include <stdlib.h>


/**
 * @brief Initialize the pdbg targets
 *
 * @param backend (cronus, fsi, host, i2c, kernel, fake)
 * @param backend_option (p8, p9, p9r, p9w, p9z, null)
 */
void pdbg_init( enum pdbg_backend backend, const char * backend_option)
{
    // sets libpdbg::pdbg_backend and libpdbg::pdbg_backend_option
    pdbg_set_backend(backend, backend_option);

    // expand dtb to fdt, sets libpdbg::pdbg_dt_root
    pdbg_targets_init(NULL);

    // probe all targets starting from root
    assert(pdbg_target_root()); // make sure root is valid
    pdbg_target_probe_all(pdbg_target_root());
}


/**
 * @brief Count number of targets of particular class
 *
 * @param classname (fsi, pib, core, thread)
 */
uint32_t count_class_target(const char *classname)
{
    pdbg_target *target;
    uint32_t count = 0;

    pdbg_for_each_class_target(classname, target)
    {
        count++;
    }

    return count;
}


/**
  * @brief Test some libpdbg interfaces
  *
  * Initialize the pdbg targets and retrieve the pib status information from
  * each available FSI CFAM register.
*/
int test_pdbg()
{
    pdbg_target * target;
    uint32_t fsi_count = 0;
    uint32_t * pib_status;

//    pdbg_init(PDBG_BACKEND_KERNEL, "p9");
    pdbg_init(PDBG_BACKEND_FAKE, NULL);

    // alloc storage for cfam pib status register data
    fsi_count = count_class_target("fsi");
    pib_status = (uint32_t *) calloc(fsi_count, sizeof(uint32_t));

    // get cfam pib status register data
    uint32_t i = 0;
    pdbg_for_each_class_target("fsi", target)
    {
        if (PDBG_TARGET_ENABLED == pdbg_target_status(target))
        {
            fsi_read(target, CFAM_PIB_STATUS_REG, &pib_status[i]);
            printf("pib_status = %04x\n", pib_status[i]);
        }
        i++;
    }
    printf("fsi_count = %u\n", fsi_count);

    // release targets when done
    pdbg_for_each_child_target(pdbg_target_root(), target)
    {
        if (PDBG_TARGET_ENABLED == pdbg_target_status(target))
        {
            pdbg_target_release(target);
        }
    }

    // free cfam pib status register storage
    free (pib_status);

    return 0;
}
