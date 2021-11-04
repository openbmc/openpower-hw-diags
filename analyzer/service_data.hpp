#pragma once

#include <analyzer/callout.hpp>
#include <hei_main.hpp>
#include <nlohmann/json.hpp>

namespace analyzer
{

/**
 * @brief Data regarding required service actions based on the hardware error
 *        analysis.
 */
class ServiceData
{
  public:
    /**
     * @brief Constructor from components.
     * @param The signature of the root cause attention.
     * @param True if the signature list contained a system checkstop attention.
     *        False, otherwise.
     */
    ServiceData(const libhei::Signature& i_rootCause, bool i_isCheckstop) :
        iv_rootCause(i_rootCause), iv_isCheckstop(i_isCheckstop)
    {}

    /** @brief Destructor. */
    ~ServiceData() = default;

    /** @brief Copy constructor. */
    ServiceData(const ServiceData&) = default;

    /** @brief Assignment operator. */
    ServiceData& operator=(const ServiceData&) = default;

  private:
    /** The signature of the root cause attention. */
    const libhei::Signature iv_rootCause;

    /** True if the signature list contained a system checkstop attention.
     *  False, otherwise. */
    const bool iv_isCheckstop;

    /** The list of callouts that will be added to a PEL. */
    nlohmann::json iv_calloutList = nlohmann::json::array();

    /** FFDC for callouts that would otherwise not be available in the
     *  callout list (unit paths, bus types, etc.). */
    nlohmann::json iv_calloutFFDC = nlohmann::json::array();

  public:
    /** @return The signature of the root cause attention. */
    const libhei::Signature& getRootCause() const
    {
        return iv_rootCause;
    }

    /** @return True if the signature list contained a system checkstop
     *          attention. False, otherwise. */
    bool queryCheckstop() const
    {
        return iv_isCheckstop;
    }

    /**
     * @brief Add callout information to the callout list.
     * @param The JSON object for this callout.
     */
    void addCallout(const nlohmann::json& i_callout);

    /**
     * @brief Add FFDC for a callout that would otherwise not be available in
     *        the callout list (unit paths, bus types, etc.).
     * @param The JSON object for this callout.
     */
    void addCalloutFFDC(const nlohmann::json& i_ffdc)
    {
        iv_calloutFFDC.push_back(i_ffdc);
    }

    /** @brief Accessor to iv_calloutList. */
    const nlohmann::json& getCalloutList() const
    {
        return iv_calloutList;
    }

    /** @brief Accessor to iv_calloutFFDC. */
    const nlohmann::json& getCalloutFFDC() const
    {
        return iv_calloutFFDC;
    }
};

} // namespace analyzer
