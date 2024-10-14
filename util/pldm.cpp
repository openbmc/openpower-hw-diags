#include "config.h"

#include <libpldm/oem/ibm/state_set.h>
#include <libpldm/platform.h>
#include <libpldm/pldm.h>
#include <libpldm/transport.h>
#include <libpldm/transport/mctp-demux.h>
#include <poll.h>

#include <util/dbus.hpp>
#include <util/pldm.hpp>
#include <util/trace.hpp>

namespace util
{
namespace pldm
{

static uint8_t pldmInstanceID;

pldm_instance_db* pldmInstanceIdDb = nullptr;
pldm_transport* pldmTransport = nullptr;
pldm_transport_mctp_demux* mctpDemux = nullptr;

PLDMInstanceManager::PLDMInstanceManager()
{
    initPLDMInstanceIdDb();
}

PLDMInstanceManager::~PLDMInstanceManager()
{
    destroyPLDMInstanceIdDb();
}
void PLDMInstanceManager::initPLDMInstanceIdDb()
{
    auto rc = pldm_instance_db_init_default(&pldmInstanceIdDb);
    if (rc)
    {
        trace::err("Error calling pldm_instance_db_init_default, rc = %d",
                   (unsigned)rc);
    }
}
void PLDMInstanceManager::destroyPLDMInstanceIdDb()
{
    auto rc = pldm_instance_db_destroy(pldmInstanceIdDb);
    if (rc)
    {
        trace::err("pldm_instance_db_destroy failed rc = %d", (unsigned)rc);
    }
}

bool getPldmInstanceID(uint8_t& pldmInstance, uint8_t tid)
{
    pldm_instance_id_t id;
    int rc = pldm_instance_id_alloc(pldmInstanceIdDb, tid, &id);
    if (rc == -EAGAIN)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        rc = pldm_instance_id_alloc(pldmInstanceIdDb, tid, &id);
    }
    if (rc)
    {
        trace::err("getPldmInstanceId: Failed to alloc ID for TID = %d, RC= %d",
                   (unsigned)tid, (unsigned)rc);
        return false;
    }
    pldmInstance = id; // Return the allocated instance ID
    trace::inf("Got instanceId: %d, for PLDM TID: %d", (unsigned)pldmInstance,
               (unsigned)tid);
    return true;
}

void freePLDMInstanceID(pldm_instance_id_t instanceID, uint8_t tid)
{
    auto rc = pldm_instance_id_free(pldmInstanceIdDb, tid, instanceID);
    if (rc)
    {
        trace::err(
            "pldm_instance_id_free failed to free id=%d of TID=%d with rc= %d",
            (unsigned)instanceID, (unsigned)tid, (unsigned)rc);
    }
}

int openPLDM(mctp_eid_t eid)
{
    auto fd = -1;
    if (pldmTransport)
    {
        trace::inf("open: pldmTransport already setup!");
        return fd;
    }
    fd = openMctpDemuxTransport(eid);
    if (fd < 0)
    {
        auto e = errno;
        trace::err("openPLDM failed, fd = %d and error= %d", (unsigned)fd, e);
    }
    return fd;
}

int openMctpDemuxTransport(mctp_eid_t eid)
{
    int rc = pldm_transport_mctp_demux_init(&mctpDemux);
    if (rc)
    {
        trace::err(
            "openMctpDemuxTransport: Failed to setup tid to eid mapping. rc = %d",
            (unsigned)rc);
        pldmClose();
        return rc;
    }

    rc = pldm_transport_mctp_demux_map_tid(mctpDemux, eid, eid);
    if (rc)
    {
        trace::err(
            "openMctpDemuxTransport: Failed to setup tid to eid mapping. rc = %d",
            (unsigned)rc);
        pldmClose();
        return rc;
    }

    pldmTransport = pldm_transport_mctp_demux_core(mctpDemux);
    struct pollfd pollfd;
    rc = pldm_transport_mctp_demux_init_pollfd(pldmTransport, &pollfd);
    if (rc)
    {
        trace::err("openMctpDemuxTransport: Failed to get pollfd. rc= %d",
                   (unsigned)rc);
        pldmClose();
        return rc;
    }
    return pollfd.fd;
}
void pldmClose()
{
    pldm_transport_mctp_demux_destroy(mctpDemux);
    mctpDemux = nullptr;
    pldmTransport = nullptr;
}

/** @brief Send PLDM request
 *
 * @param[in] request - the request data
 * @param[in] mcptEid - the mctp endpoint ID
 *
 * @pre a mctp instance must have been
 * @return true if send is successful false otherwise
 */
bool sendPldm(const std::vector<uint8_t>& request, uint8_t mctpEid)
{
    auto rc = openPLDM(mctpEid);
    if (rc)
    {
        trace::err("failed to connect to pldm");
        freePLDMInstanceID(pldmInstanceID, mctpEid);
        return false;
    }

    pldm_tid_t pldmTID = static_cast<pldm_tid_t>(mctpEid);
    // send PLDM request
    auto pldmRc = pldm_transport_send_msg(pldmTransport, pldmTID,
                                          request.data(), request.size());

    trace::inf("sent pldm request");

    return pldmRc == PLDM_REQUESTER_SUCCESS ? true : false;
}

/** @brief Prepare a request for SetStateEffecterStates
 *
 *  @param[in] effecterId - the effecter ID
 *  @param[in] effecterCount - composite effecter count
 *  @param[in] stateIdPos - position of the state set
 *  @param[in] stateSetValue - the value to set the state
 *  @param[in] mcptEid - the MCTP endpoint ID
 *
 *  @return PLDM request message to be sent to host, empty message on error
 */
std::vector<uint8_t> prepareSetEffecterReq(
    uint16_t effecterId, uint8_t effecterCount, uint8_t stateIdPos,
    uint8_t stateSetValue, uint8_t mctpEid)
{
    // get pldm instance associated with the endpoint ID
    uint8_t pldmInstanceID;
    if (!getPldmInstanceID(pldmInstanceID, mctpEid))
    {
        return std::vector<uint8_t>();
    }

    // form the request message
    std::vector<uint8_t> request(
        sizeof(pldm_msg_hdr) + sizeof(effecterId) + sizeof(effecterCount) +
        (effecterCount * sizeof(set_effecter_state_field)));

    // encode the state data with the change we want to elicit
    std::vector<set_effecter_state_field> stateField;
    for (uint8_t effecterPos = 0; effecterPos < effecterCount; effecterPos++)
    {
        if (effecterPos == stateIdPos)
        {
            stateField.emplace_back(
                set_effecter_state_field{PLDM_REQUEST_SET, stateSetValue});
        }
        else
        {
            stateField.emplace_back(
                set_effecter_state_field{PLDM_NO_CHANGE, 0});
        }
    }

    // encode the message with state data
    auto requestMsg = reinterpret_cast<pldm_msg*>(request.data());
    auto rc = encode_set_state_effecter_states_req(
        pldmInstanceID, effecterId, effecterCount, stateField.data(),
        requestMsg);

    if (rc != PLDM_SUCCESS)
    {
        trace::err("encode set effecter states request failed");
        freePLDMInstanceID(pldmInstanceID, mctpEid);
        request.clear();
    }

    return request;
}

/** @brief Return map of sensor ID to SBE instance
 *
 *  @param[in] stateSetId - the state set ID of interest
 *  @param[out] sensorInstanceMap - map of sensor to SBE instance
 *  @param[out] sensorOffset - position of sensor with state set ID within map
 *
 *  @return true if sensor info is available false otherwise
 */
bool fetchSensorInfo(uint16_t stateSetId,
                     std::map<uint16_t, unsigned int>& sensorInstanceMap,
                     uint8_t& sensorOffset)
{
    // get state sensor PDRs
    std::vector<std::vector<uint8_t>> pdrs{};
    if (!util::dbus::getStateSensorPdrs(pdrs, stateSetId))
    {
        return false;
    }

    // check for any PDRs available
    if (!pdrs.size())
    {
        trace::err("state sensor PDRs not present");
        return false;
    }

    // find the offset of specified sensor withing PDRs
    bool offsetFound = false;
    auto stateSensorPDR =
        reinterpret_cast<const pldm_state_sensor_pdr*>(pdrs.front().data());
    auto possibleStatesPtr = stateSensorPDR->possible_states;

    for (auto offset = 0; offset < stateSensorPDR->composite_sensor_count;
         offset++)
    {
        auto possibleStates =
            reinterpret_cast<const state_sensor_possible_states*>(
                possibleStatesPtr);

        if (possibleStates->state_set_id == stateSetId)
        {
            sensorOffset = offset;
            offsetFound = true;
            break;
        }
        possibleStatesPtr += sizeof(possibleStates->state_set_id) +
                             sizeof(possibleStates->possible_states_size) +
                             possibleStates->possible_states_size;
    }

    if (!offsetFound)
    {
        trace::err("state sensor not found");
        return false;
    }

    // map sensor ID to equivelent 16 bit value
    std::map<uint32_t, uint16_t> entityInstMap{};
    for (auto& pdr : pdrs)
    {
        auto pdrPtr =
            reinterpret_cast<const pldm_state_sensor_pdr*>(pdr.data());
        uint32_t key = pdrPtr->sensor_id;
        entityInstMap.emplace(key, static_cast<uint16_t>(pdrPtr->sensor_id));
    }

    // map sensor ID to zero based SBE instance
    unsigned int position = 0;
    for (const auto& pair : entityInstMap)
    {
        sensorInstanceMap.emplace(pair.second, position);
        position++;
    }

    return true;
}

/** @brief Return map of SBE instance to effecter ID
 *
 *  @param[in] stateSetId - the state set ID of interest
 *  @param[out] instanceToEffecterMap - map of sbe instance to effecter ID
 *  @param[out] effecterCount - composite effecter count
 *  @param[out] stateIdPos - position of effecter with state set ID within map
 *
 *  @return true if effector info is available false otherwise
 */
bool fetchEffecterInfo(uint16_t stateSetId,
                       std::map<unsigned int, uint16_t>& instanceToEffecterMap,
                       uint8_t& effecterCount, uint8_t& stateIdPos)
{
    // get state effecter PDRs
    std::vector<std::vector<uint8_t>> pdrs{};
    if (!util::dbus::getStateEffecterPdrs(pdrs, stateSetId))
    {
        return false;
    }

    // check for any PDRs available
    if (!pdrs.size())
    {
        trace::err("state effecter PDRs not present");
        return false;
    }

    // find the offset of specified effector within PDRs
    bool offsetFound = false;
    auto stateEffecterPDR =
        reinterpret_cast<const pldm_state_effecter_pdr*>(pdrs.front().data());
    auto possibleStatesPtr = stateEffecterPDR->possible_states;

    for (auto offset = 0; offset < stateEffecterPDR->composite_effecter_count;
         offset++)
    {
        auto possibleStates =
            reinterpret_cast<const state_effecter_possible_states*>(
                possibleStatesPtr);

        if (possibleStates->state_set_id == stateSetId)
        {
            stateIdPos = offset;
            effecterCount = stateEffecterPDR->composite_effecter_count;
            offsetFound = true;
            break;
        }
        possibleStatesPtr += sizeof(possibleStates->state_set_id) +
                             sizeof(possibleStates->possible_states_size) +
                             possibleStates->possible_states_size;
    }

    if (!offsetFound)
    {
        trace::err("state set effecter not found");
        return false;
    }

    // map effecter ID to equivelent 16 bit value
    std::map<uint32_t, uint16_t> entityInstMap{};
    for (auto& pdr : pdrs)
    {
        auto pdrPtr =
            reinterpret_cast<const pldm_state_effecter_pdr*>(pdr.data());
        uint32_t key = pdrPtr->effecter_id;
        entityInstMap.emplace(key, static_cast<uint16_t>(pdrPtr->effecter_id));
    }

    // map zero based SBE instance to effecter ID
    unsigned int position = 0;
    for (const auto& pair : entityInstMap)
    {
        instanceToEffecterMap.emplace(position, pair.second);
        position++;
    }

    return true;
}

/**  @brief Reset SBE using HBRT PLDM interface */
bool hresetSbe(unsigned int sbeInstance)
{
    trace::inf("requesting sbe hreset");

    // get effecter info
    std::map<unsigned int, uint16_t> sbeInstanceToEffecter;
    uint8_t SBEEffecterCount = 0;
    uint8_t sbeMaintenanceStatePosition = 0;

    if (!fetchEffecterInfo(PLDM_OEM_IBM_SBE_MAINTENANCE_STATE,
                           sbeInstanceToEffecter, SBEEffecterCount,
                           sbeMaintenanceStatePosition))
    {
        return false;
    }

    // find the state effecter ID for the given SBE instance
    auto effecterEntry = sbeInstanceToEffecter.find(sbeInstance);
    if (effecterEntry == sbeInstanceToEffecter.end())
    {
        trace::err("failed to find effecter for SBE");
        return false;
    }

    // create request to HRESET the SBE
    constexpr uint8_t hbrtMctpEid = 10; // HBRT MCTP EID

    auto request = prepareSetEffecterReq(
        effecterEntry->second, SBEEffecterCount, sbeMaintenanceStatePosition,
        SBE_RETRY_REQUIRED, hbrtMctpEid);

    if (request.empty())
    {
        trace::err("HRESET effecter request empty");
        return false;
    }

    // get sensor info for validating sensor change
    std::map<uint16_t, unsigned int> sensorToSbeInstance;
    uint8_t sbeSensorOffset = 0;
    if (!fetchSensorInfo(PLDM_OEM_IBM_SBE_HRESET_STATE, sensorToSbeInstance,
                         sbeSensorOffset))
    {
        return false;
    }

    // register signal change listener
    std::string hresetStatus = "requested";
    constexpr auto interface = "xyz.openbmc_project.PLDM.Event";
    constexpr auto path = "/xyz/openbmc_project/pldm";
    constexpr auto member = "StateSensorEvent";

    auto bus = sdbusplus::bus::new_default();
    std::unique_ptr<sdbusplus::bus::match_t> match =
        std::make_unique<sdbusplus::bus::match_t>(
            bus,
            sdbusplus::bus::match::rules::type::signal() +
                sdbusplus::bus::match::rules::member(member) +
                sdbusplus::bus::match::rules::path(path) +
                sdbusplus::bus::match::rules::interface(interface),
            [&](auto& msg) {
                uint8_t sensorTid{};
                uint16_t sensorId{};
                uint8_t msgSensorOffset{};
                uint8_t eventState{};
                uint8_t previousEventState{};

                // get sensor event details
                msg.read(sensorTid, sensorId, msgSensorOffset, eventState,
                         previousEventState);

                // does sensor offset match?
                if (sbeSensorOffset == msgSensorOffset)
                {
                    // does sensor ID match?
                    auto sensorEntry = sensorToSbeInstance.find(sensorId);
                    if (sensorEntry != sensorToSbeInstance.end())
                    {
                        const uint8_t instance = sensorEntry->second;

                        // if instances matche check status
                        if (instance == sbeInstance)
                        {
                            if (eventState ==
                                static_cast<uint8_t>(SBE_HRESET_READY))
                            {
                                hresetStatus = "success";
                            }
                            else if (eventState ==
                                     static_cast<uint8_t>(SBE_HRESET_FAILED))
                            {
                                hresetStatus = "fail";
                            }
                        }
                    }
                }
            });

    // send request to issue hreset of sbe
    if (!sendPldm(request, hbrtMctpEid))
    {
        trace::err("send pldm request failed");
        return false;
    }

    // keep track of elapsed time
    uint64_t timeRemaining = 60000000; // microseconds, 1 minute
    std::chrono::steady_clock::time_point begin =
        std::chrono::steady_clock::now();

    // wait for status update or timeout
    trace::inf("waiting on sbe hreset");
    while ("requested" == hresetStatus && 0 != timeRemaining)
    {
        bus.wait(timeRemaining);
        uint64_t timeElapsed =
            std::chrono::duration_cast<std::chrono::microseconds>(
                std::chrono::steady_clock::now() - begin)
                .count();

        timeRemaining =
            timeElapsed > timeRemaining ? 0 : timeRemaining - timeElapsed;

        bus.process_discard();
    }

    if (0 == timeRemaining)
    {
        trace::err("hreset timed out");
    }

    pldmClose();

    return hresetStatus == "success" ? true : false;
}

} // namespace pldm
} // namespace util
