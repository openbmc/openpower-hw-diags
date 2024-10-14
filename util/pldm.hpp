#pragma once

#include <libpldm/instance-id.h>
#include <libpldm/transport.h>

namespace util
{
namespace pldm
{
extern struct pldm_transport* pldmTransport;

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
 * @brief setup PLDM transport for sending and receiving messages
 *
 * @param[in] eid - MCTP endpoint ID
 * @return file descriptor on success and throw
 *         exception (xyz::openbmc_project::Common::Error::NotAllowed) on
 *         failures.
 */
int openPLDM(mctp_eid_t eid);

/** @brief Opens the MCTP socket for sending and receiving messages.
 *
 * @param[in] eid - MCTP endpoint ID
 */
int openMctpDemuxTransport(mctp_eid_t eid);

/** @brief Close the PLDM file */
void pldmClose();

/**
 * @brief Get PLDM  instance ID associated with endpoint
 *
 * @param[out] pldmInstanceID - PLDM instance id
 * @param[in] Eid - MCTP enpoint ID
 *
 * @return True on success otherwise False
 */
bool getPldmInstanceID(uint8_t& pldmInstanceID, uint8_t Eid);

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
