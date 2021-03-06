#pragma once

#include <hei_main.hpp>
#include <nlohmann/json.hpp>

namespace analyzer
{

/** @brief An abstract class for service event, also known as a callout. */
class Callout
{
  public:
    /** Each callout will have a priority indicating when to issue the required
     *  service action. Details below. */
    enum Priority
    {
        /** Serivce is mandatory. */
        HIGH,

        /** Serivce medium priority callouts one at a time, in order, until the
         *  issue is resolved. */
        MED,

        MED_A, ///< Same as PRI_MED, except replace all A's as a group.
        MED_B, ///< Same as PRI_MED, except replace all B's as a group.
        MED_C, ///< Same as PRI_MED, except replace all C's as a group.

        /** If servicing all high and medium priority callouts did not resolve
         *  the issue, service low priority callouts one at a time, in order,
         *  until the issue is resolved. */
        LOW,
    };

  public:
    /** @brief Pure virtual destructor. */
    virtual ~Callout() = 0;

  protected:
    /**
     * @brief Constructor from components.
     * @param p The callout priority.
     */
    explicit Callout(Priority p) : iv_priority(p) {}

  private:
    /** The callout priority. */
    const Priority iv_priority;

  protected:
    /**
     * @brief Appends the callout priority to the end of the given json object.
     * @param j The json object for a single callout.
     */
    void addPriority(nlohmann::json& j) const
    {
        // clang-format off
        static const std::map<Priority, std::string> m =
        {
            {HIGH,  "H"},
            {MED,   "M"},
            {MED_A, "A"},
            {MED_B, "B"},
            {MED_C, "C"},
            {LOW,   "L"},
        };
        // clang-format on

        j.emplace("Priority", m.at(iv_priority));
    }

  public:
    /**
     * @brief Appends a json object representing this callout to the end of the
     *        given json object.
     * @param j The json object containing all current callouts for a PEL.
     */
    virtual void getJson(nlohmann::json&) const = 0;
};

// Pure virtual destructor must be defined.
inline Callout::~Callout() {}

/** @brief A service event requiring hardware replacement. */
class HardwareCallout : public Callout
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_locationCode The location code of the hardware callout.
     * @param i_priority     The callout priority.
     */
    HardwareCallout(const std::string& i_locationCode, Priority i_priority) :
        Callout(i_priority), iv_locationCode(i_locationCode)
    {}

  private:
    /** The hardware location code. */
    const std::string iv_locationCode;

  public:
    void getJson(nlohmann::json& j) const override
    {
        nlohmann::json c = {{"LocationCode", iv_locationCode}};
        addPriority(c);
        j.emplace_back(c);
    }
};

/**
 * @brief A service event requiring a special procedure to be handled by a
 *        service engineer.
 */
class ProcedureCallout : public Callout
{
  public:
    /** Supported service procedures. */
    enum Type
    {
        NEXTLVL, ///< Contact next level support.
    };

  public:
    /**
     * @brief Constructor from components.
     * @param i_procedure The location code of the hardware callout.
     * @param i_priority     The callout priority.
     */
    ProcedureCallout(Type i_procedure, Priority i_priority) :
        Callout(i_priority), iv_procedure(i_procedure)
    {}

  private:
    /** The callout priority. */
    const Type iv_procedure;

  public:
    void getJson(nlohmann::json& j) const override
    {
        // clang-format off
        static const std::map<Type, std::string> m =
        {
            {NEXTLVL, "NEXTLVL"},
        };
        // clang-format on

        nlohmann::json c = {{"Procedure", m.at(iv_procedure)}};
        addPriority(c);
        j.emplace_back(c);
    }
};

/**
 * @brief A service event requiring hardware to be guarded.
 */
class Guard
{
  public:
    /** Supported guard types. */
    enum Type
    {
        NONE,      ///< Do not guard
        FATAL,     ///< Guard on fatal error (cannot recover resource)
        NON_FATAL, ///< Guard on non-fatal error (can recover resource)
    };

  public:
    /**
     * @brief Constructor from components.
     * @param i_path The hardware path to guard.
     * @param i_type The guard type.
     */
    Guard(const std::string& i_path, Type i_type) :
        iv_path(i_path), iv_type(i_type)
    {}

  private:
    /** The hardware path to guard. */
    const std::string iv_path;

    /** The guard type. */
    const Type iv_type;

  public:
    void getJson(nlohmann::json& j) const
    {
        // clang-format off
        static const std::map<Type, std::string> m =
        {
            {NONE,      "NONE"},
            {FATAL,     "FATAL"},
            {NON_FATAL, "NON_FATAL"},
        };
        // clang-format on

        nlohmann::json c = {{"Path", iv_path}, {"Type", m.at(iv_type)}};
        j.emplace_back(c);
    }
};

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
     */
    explicit ServiceData(const libhei::Signature& i_rootCause) :
        iv_rootCause(i_rootCause)
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

    /** The list of callouts that will be added to a PEL. */
    std::vector<std::shared_ptr<Callout>> iv_calloutList;

    /** The list of hardware guard requests. Some information will be added to
     * the PEL, but the actual guard record will be created after submitting the
     * PEL. */
    std::vector<std::shared_ptr<Guard>> iv_guardList;

  public:
    /** @return The signature of the root cause attention. */
    const libhei::Signature& getRootCause() const
    {
        return iv_rootCause;
    }

    /** Add a callout to the list. */
    void addCallout(const std::shared_ptr<Callout>& i_callout)
    {
        iv_calloutList.push_back(i_callout);
    }

    /** Add a guard request to the list. */
    void addGuard(const std::shared_ptr<Guard>& i_guard)
    {
        iv_guardList.push_back(i_guard);
    }

    /**
     * @brief Iterates the callout list and returns the json attached to each
     *        callout in the list.
     * @param o_json The returned json data.
     */
    void getCalloutList(nlohmann::json& o_json) const
    {
        o_json.clear(); // Ensure we are starting with a clean list.

        for (const auto& c : iv_calloutList)
        {
            c->getJson(o_json);
        }
    }

    /**
     * @brief Iterates the guard list and returns the json attached to each
     *        guard request in the list.
     * @param o_json The returned json data.
     */
    void getGuardList(nlohmann::json& o_json) const
    {
        o_json.clear(); // Ensure we are starting with a clean list.

        for (const auto& g : iv_guardList)
        {
            g->getJson(o_json);
        }
    }
};

} // namespace analyzer
