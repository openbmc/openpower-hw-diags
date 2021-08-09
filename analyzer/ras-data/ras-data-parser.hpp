#pragma once

#include <analyzer/resolution.hpp>
#include <hei_main.hpp>
#include <nlohmann/json.hpp>

#include <map>

namespace analyzer
{

/**
 * @brief Manages the RAS data files and resolves service actions required for
 *        error signatures.
 */
class RasDataParser
{
  private:
    /** @brief Supported RAS data versions. */
    enum SchemaVersion
    {
        VERSION_1 = 1,
    };

  public:
    /** @brief Default constructor. */
    RasDataParser()
    {
        initDataFiles();
    }

  private:
    /** @brief The RAS data files. */
    std::map<libhei::ChipType_t, nlohmann::json> iv_dataFiles;

  public:
    /**
     * @brief Returns a resolution for all the RAS actions needed for the given
     *        signature.
     * @param i_signature The target error signature.
     */
    std::shared_ptr<Resolution>
        getResolution(const libhei::Signature& i_signature);

  private:
    /**
     * @brief Parses all of the RAS data JSON files and validates them against
     *        the associated schema.
     */
    void initDataFiles();

    /**
     * @brief  Parses a signature in the given data file and returns a string
     *         representing the target action for the signature.
     * @param  i_data      The parsed RAS data file associated with the
     *                     signature's chip type.
     * @param  i_signature The target signature.
     * @return A string representing the target action for the signature.
     */
    std::string parseSignature(const nlohmann::json& i_data,
                               const libhei::Signature& i_signature);

    /**
     * @brief  Parses an action in the given data file and returns the
     *         corresponding resolution.
     * @param  i_data   The parsed RAS data file associated with the signature's
     *                  chip type.
     * @param  i_action The target action to parse from the given RAS data.
     * @return A resolution (or nested resolutions) representing the given
     *         action.
     * @note   This function is called recursively because an action could
     *         reference another action. This function will maintain a stack of
     *         parsed actions and will assert that the same action cannot be
     *         parsed more than once in the recursion stack.
     */
    std::shared_ptr<Resolution> parseAction(const nlohmann::json& i_data,
                                            const std::string& i_action);

    /**
     * @brief  Returns a callout priority enum value for the given string.
     * @param  i_priority The priority string.
     * @return A callout priority enum value.
     */
    Callout::Priority getPriority(const std::string& i_priority);
};

} // namespace analyzer
