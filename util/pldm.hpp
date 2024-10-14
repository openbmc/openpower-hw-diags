#pragma once

#include <libpldm/instance-id.h>

namespace util
{
namespace pldm
{
class PLDMInstanceManager
{
  public:
    PLDMInstanceManager(const PLDMInstanceManager&) = delete;
    PLDMInstanceManager& operator=(const PLDMInstanceManager&) = delete;
    PLDMInstanceManager();
    ~PLDMInstanceManager();

  private:
    /**
     * @brief Instantiates an instance ID database object
     *
     * @return void
     **/
    void initPLDMInstanceIdDb();
    /**
     * @brief Destroys an instance ID database object
     *
     * @return void
     **/
    void destroyPLDMInstanceIdDb();
};

/**
 * @brief Get PLDM  instance ID associated with endpoint
 *
 * @param[out] pldmInstanceID - PLDM instance id
 * @param[in] tid - the terminus ID the instance ID is associated with
 *
 * @return True on success otherwise False
 */
bool getPldmInstanceID(uint8_t& pldmInstanceID, uint8_t tid);

/** @brief Free PLDM requester instance id */
void freePldmInstanceId();

/*
 *  @brief HRESET the SBE
 *
 *  @pre Host must be running
 *
 *  @param[in] sbeInstance - SBE to target (0 based)
 *
 *  @return true if HRESET successful
 *
 */
bool hresetSbe(unsigned int sbeInstance);

} // namespace pldm
} // namespace util
