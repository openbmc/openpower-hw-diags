#pragma once

#include <analyzer/service_data.hpp>

namespace analyzer
{

/** @brief An abstract class for service event resolutions. */
class Resolution
{
  public:
    /** @brief Pure virtual destructor. */
    virtual ~Resolution() = 0;

  public:
    /**
     * @brief Resolves the service actions required by this resolution.
     * @param io_sd An object containing the service data collected during
     *              hardware error analysis.
     */
    virtual void resolve(ServiceData& io_sd) const = 0;
};

// Pure virtual destructor must be defined.
inline Resolution::~Resolution() {}

/** @brief Resolves a hardware callout service event. */
class HardwareCalloutResolution : public Resolution
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_unitPath The devtree path of a guardable unit relative to a
     *                   chip. An empty string refers to the chip itself.
     * @param i_priority The callout priority.
     * @param i_guard    True, if guard is required. False, otherwise.
     */
    HardwareCalloutResolution(const std::string& i_unitPath,
                              callout::Priority i_priority, bool i_guard) :
        iv_unitPath(i_unitPath),
        iv_priority(i_priority), iv_guard(i_guard)
    {}

  private:
    /** The devtree path of a guardable unit relative to a chip. An empty string
     *  refers to the chip itself. */
    const std::string iv_unitPath;

    /** The callout priority. */
    const callout::Priority iv_priority;

    /** True, if guard is required. False, otherwise. */
    const bool iv_guard;

  public:
    void resolve(ServiceData& io_sd) const override;
};

/** @brief Resolution to callout a connected chip/target. */
class ConnectedCalloutResolution : public Resolution
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_busType  The bus type.
     * @param i_unitPath The path of the chip unit that is connected to the
     *                   other chip. An empty string refers to the chip itself,
     *                   which generally means this chip is a child of another.
     * @param i_priority The callout priority.
     * @param i_guard    The guard type for this callout.
     */
    ConnectedCalloutResolution(const callout::BusType& i_busType,
                               const std::string& i_unitPath,
                               callout::Priority i_priority, bool i_guard) :
        iv_busType(i_busType),
        iv_unitPath(i_unitPath), iv_priority(i_priority), iv_guard(i_guard)
    {}

  private:
    /** The bus type. */
    const callout::BusType iv_busType;

    /** The devtree path the chip unit that is connected to the other chip. An
     *  empty string refers to the chip itself, which generally means this chip
     *  is a child of the other chip. */
    const std::string iv_unitPath;

    /** The callout priority. */
    const callout::Priority iv_priority;

    /** True, if guard is required. False, otherwise. */
    const bool iv_guard;

  public:
    void resolve(ServiceData& io_sd) const override;
};

/**
 * @brief Resolution to callout all parts on a bus (RX/TX endpoints and
 *         everything else in between).
 */
class BusCalloutResolution : public Resolution
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_busType  The bus type.
     * @param i_unitPath The path of the chip unit that is connected to the
     *                   other chip. An empty string refers to the chip itself,
     *                   which generally means this chip is a child of another.
     * @param i_priority The callout priority.
     * @param i_guard    The guard type for this callout.
     */
    BusCalloutResolution(const callout::BusType& i_busType,
                         const std::string& i_unitPath,
                         callout::Priority i_priority, bool i_guard) :
        iv_busType(i_busType),
        iv_unitPath(i_unitPath), iv_priority(i_priority), iv_guard(i_guard)
    {}

  private:
    /** The bus type. */
    const callout::BusType iv_busType;

    /** The devtree path the chip unit that is connected to the other chip. An
     *  empty string refers to the chip itself, which generally means this chip
     *  is a child of the other chip. */
    const std::string iv_unitPath;

    /** The callout priority. */
    const callout::Priority iv_priority;

    /** True, if guard is required. False, otherwise. */
    const bool iv_guard;

  public:
    void resolve(ServiceData& io_sd) const override;
};

/** @brief Resolves a clock callout service event. */
class ClockCalloutResolution : public Resolution
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_clockType The clock type.
     * @param i_priority  The callout priority.
     * @param i_guard     The guard type for this callout.
     */
    ClockCalloutResolution(const callout::ClockType& i_clockType,
                           callout::Priority i_priority, bool i_guard) :
        iv_clockType(i_clockType),
        iv_priority(i_priority), iv_guard(i_guard)
    {}

  private:
    /** The clock type. */
    const callout::ClockType iv_clockType;

    /** The callout priority. */
    const callout::Priority iv_priority;

    /** True, if guard is required. False, otherwise. */
    const bool iv_guard;

  public:
    void resolve(ServiceData& io_sd) const override;
};

/** @brief Resolves a procedure callout service event. */
class ProcedureCalloutResolution : public Resolution
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_procedure The procedure callout type.
     * @param i_priority  The callout priority.
     */
    ProcedureCalloutResolution(const callout::Procedure& i_procedure,
                               callout::Priority i_priority) :
        iv_procedure(i_procedure),
        iv_priority(i_priority)
    {}

  private:
    /** The procedure callout type. */
    const callout::Procedure iv_procedure;

    /** The callout priority. */
    const callout::Priority iv_priority;

  public:
    void resolve(ServiceData& io_sd) const override;
};

/** @brief Resolves a part callout service event. */
class PartCalloutResolution : public Resolution
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_part     The part callout type.
     * @param i_priority The callout priority.
     */
    PartCalloutResolution(const callout::PartType& i_part,
                          callout::Priority i_priority) :
        iv_part(i_part),
        iv_priority(i_priority)
    {}

  private:
    /** The part callout type. */
    const callout::PartType iv_part;

    /** The callout priority. */
    const callout::Priority iv_priority;

  public:
    void resolve(ServiceData& io_sd) const override;
};

/**
 * @brief Some service actions cannot be contained within the RAS data files.
 *        This resolution class allows a predefined plugin function to be
 *        called to do additional service action work.
 */
class PluginResolution : public Resolution
{
  public:
    /**
     * @brief Constructor from components.
     * @param i_name     The name of the plugin.
     * @param i_instance A plugin could be defined for multiple chip
     *                   units/registers.
     */
    PluginResolution(const std::string& i_name, unsigned int i_instance) :
        iv_name(i_name), iv_instance(i_instance)
    {}

  private:
    /** The name of the plugin. */
    const std::string iv_name;

    /** Some plugins will define the same action for multiple instances of a
     *  register (i.e. for each core on a processor). */
    const unsigned int iv_instance;

  public:
    void resolve(ServiceData& io_sd) const override;
};

/**
 * @brief Contains a list of resolutions. This resolutions will be resolved the
 *        list in the order in which they were inputted into the constructor.
 */
class ResolutionList : public Resolution
{
  public:
    /** @brief Default constructor. */
    ResolutionList() = default;

  private:
    /** The resolution list. */
    std::vector<std::shared_ptr<Resolution>> iv_list;

  public:
    /**
     * @brief Adds a new resolution to the end of the list.
     * @param i_resolution The new resolution
     */
    void push(const std::shared_ptr<Resolution>& i_resolution)
    {
        iv_list.push_back(i_resolution);
    }

    // Overloaded from parent.
    void resolve(ServiceData& io_sd) const override
    {
        for (const auto& e : iv_list)
        {
            e->resolve(io_sd);
        }
    }
};

} // namespace analyzer
