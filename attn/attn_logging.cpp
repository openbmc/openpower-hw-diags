#include <unistd.h>

#include <attn/attn_logging.hpp>
#include <attn/pel/pel_minimal.hpp>
#include <phosphor-logging/log.hpp>

namespace attn
{

/** @brief Journal entry of type INFO using phosphor logging */
template <>
void trace<INFO>(const char* i_message)
{
    phosphor::logging::log<phosphor::logging::level::INFO>(i_message);
}

/** @brief Tuple containing information about ffdc files */
using FFDCTuple =
    std::tuple<util::FFDCFormat, uint8_t, uint8_t, sdbusplus::message::unix_fd>;

/** @brief Gather messages from the journal */
std::vector<std::string> sdjGetMessages(const std::string& field,
                                        const std::string& fieldValue,
                                        unsigned int max);

/**
 * Create FFDCTuple objects corresponding to the specified FFDC files.
 *
 * The D-Bus method to create an error log requires a vector of tuples to
 * pass in the FFDC file information.
 *
 * @param   files - FFDC files
 * @return  vector of FFDCTuple objects
 */
std::vector<FFDCTuple>
    createFFDCTuples(const std::vector<util::FFDCFile>& files)
{
    std::vector<FFDCTuple> ffdcTuples{};
    for (const util::FFDCFile& file : files)
    {
        ffdcTuples.emplace_back(
            file.getFormat(), file.getSubType(), file.getVersion(),
            sdbusplus::message::unix_fd(file.getFileDescriptor()));
    }

    return ffdcTuples;
}

/**
 * @brief Create an FFDCFile object containing raw data
 *
 * Throws an exception if an error occurs.
 *
 * @param   i_buffer - raw data to add to ffdc faw data file
 * @param   i_size - size of the raw data
 * @return  FFDCFile object
 */
util::FFDCFile createFFDCRawFile(void* i_buffer, size_t i_size)
{
    util::FFDCFile file{util::FFDCFormat::Custom};

    // Write buffer to file and then reset file description file offset
    int fd = file.getFileDescriptor();
    write(fd, static_cast<char*>(i_buffer), i_size);
    lseek(fd, 0, SEEK_SET);

    return file;
}

/**
 * @brief Create an FFDCFile object containing the specified lines of text data
 *
 * Throws an exception if an error occurs.
 *
 * @param   lines - lines of text data to write to file
 * @return  FFDCFile object
 */
util::FFDCFile createFFDCTraceFile(const std::vector<std::string>& lines)
{
    // Create FFDC file of type Text
    util::FFDCFile file{util::FFDCFormat::Text};
    int fd = file.getFileDescriptor();

    // Write FFDC lines to file
    std::string buffer;
    for (const std::string& line : lines)
    {
        // Copy line to buffer.  Add newline if necessary.
        buffer = line;
        if (line.empty() || (line.back() != '\n'))
        {
            buffer += '\n';
        }

        // write buffer to file
        write(fd, buffer.c_str(), buffer.size());
    }

    // Seek to beginning of file so error logging system can read data
    lseek(fd, 0, SEEK_SET);

    return file;
}

/**
 * Create FDDC files from journal messages of relevant executables
 *
 * Parse the system journal looking for log entries created by the executables
 * of interest for logging. For each of these entries create a ffdc trace file
 * that will be used to create ffdc log entries. These files will be pushed
 * onto the stack of ffdc files.
 *
 * @param   i_files - vector of ffdc files that will become log entries
 */
void createFFDCTraceFiles(std::vector<util::FFDCFile>& i_files)
{
    // Executables of interest
    std::vector<std::string> executables{"openpower-hw-diags"};

    for (const std::string& executable : executables)
    {
        try
        {
            // get journal messages
            std::vector<std::string> messages =
                sdjGetMessages("SYSLOG_IDENTIFIER", executable, 30);

            // Create FFDC file containing the journal messages
            if (!messages.empty())
            {
                i_files.emplace_back(createFFDCTraceFile(messages));
            }
        }
        catch (const std::exception& e)
        {
            std::stringstream ss;
            ss << "createFFDCFiles: " << e.what();
            trace<level::INFO>(ss.str().c_str());
        }
    }
}

/**
 * Create FFDCFile objects containing debug data to store in the error log.
 *
 * If an error occurs, the error is written to the journal but an exception
 * is not thrown.
 *
 * @param   i_buffer - raw data (if creating raw dump ffdc entry in log)
 * @return  vector of FFDCFile objects
 */
std::vector<util::FFDCFile> createFFDCFiles(char* i_buffer = nullptr,
                                            size_t i_size  = 0)
{
    std::vector<util::FFDCFile> files{};

    // Create raw dump file
    if ((nullptr != i_buffer) && (0 != i_size))
    {
        files.emplace_back(createFFDCRawFile(i_buffer, i_size));
    }

    // Create trace dump file
    createFFDCTraceFiles(files);

    return files;
}

/**
 * Get file descriptor of exisitng PEL
 *
 * The backend logging code will search for a PEL having the provided PEL ID
 * and return a file descriptor to a file containing this PEL's  raw PEL data.
 *
 * @param  i_pelid - the PEL ID
 * @return file descriptor of file containing the raw PEL data
 */
int getPelFd(uint32_t i_pelId)
{
    // GetPEL returns file descriptor (int)
    int fd = -1;

    // Sdbus call specifics
    constexpr auto service   = "xyz.openbmc_project.Logging";
    constexpr auto path      = "/xyz/openbmc_project/logging";
    constexpr auto interface = "org.open_power.Logging.PEL";
    constexpr auto function  = "GetPEL";

    // Get the PEL file descriptor
    try
    {
        auto bus    = sdbusplus::bus::new_default_system();
        auto method = bus.new_method_call(service, path, interface, function);
        method.append(i_pelId);

        auto resp = bus.call(method);

        sdbusplus::message::unix_fd msgFd;
        resp.read(msgFd);
        fd = dup(msgFd); // -1 if not found
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        std::stringstream ss;
        ss << "getPelFd: " << e.what();
        trace<level::INFO>(ss.str().c_str());
    }

    // File descriptor or -1 if not found or call failed
    return fd;
}

/**
 * Create a PEL for the specified event type
 *
 * The additional data provided in the map will be placed in a user data
 * section of the PEL and may additionally contain key words to trigger
 * certain behaviors by the backend logging code. Each set of data described
 * in the vector of ffdc data will be placed in additional user data sections.
 *
 * @param  i_event - the event type
 * @param  i_additional - map of additional data
 * @param  9_ffdc - vector of ffdc data
 * @return The created PEL's platform log-id
 */
uint32_t createPel(std::string i_event,
                   std::map<std::string, std::string> i_additional,
                   std::vector<util::FFDCTuple> i_ffdc)
{
    // CreatePELWithFFDCFiles returns log-id and platform log-id
    std::tuple<uint32_t, uint32_t> pelResp = {0, 0};

    // Sdbus call specifics
    constexpr auto level     = "xyz.openbmc_project.Logging.Entry.Level.Error";
    constexpr auto service   = "xyz.openbmc_project.Logging";
    constexpr auto path      = "/xyz/openbmc_project/logging";
    constexpr auto interface = "org.open_power.Logging.PEL";
    constexpr auto function  = "CreatePELWithFFDCFiles";

    // Need to provide pid when using create or create-with-ffdc methods
    i_additional.emplace("_PID", std::to_string(getpid()));

    // Create the PEL
    try
    {
        auto bus    = sdbusplus::bus::new_default_system();
        auto method = bus.new_method_call(service, path, interface, function);
        method.append(i_event, level, i_additional, i_ffdc);

        auto resp = bus.call(method);

        resp.read(pelResp);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        std::stringstream ss;
        ss << "createPel: " << e.what();
        trace<level::INFO>(ss.str().c_str());
    }

    // pelResp<0> == log-id, pelResp<1> = platform log-id
    return std::get<1>(pelResp);
}

/*
 * Create a PEL from raw PEL data
 *
 * The backend logging code will create a PEL based on the specified PEL data.
 *
 * @param   i_buffer - buffer containing a raw PEL
 */
void createPelRaw(std::vector<uint8_t>& i_buffer)
{
    // Sdbus call specifics
    constexpr auto event     = "xyz.open_power.Attn.Error.Terminate";
    constexpr auto level     = "xyz.openbmc_project.Logging.Entry.Level.Error";
    constexpr auto service   = "xyz.openbmc_project.Logging";
    constexpr auto path      = "/xyz/openbmc_project/logging";
    constexpr auto interface = "xyz.openbmc_project.Logging.Create";
    constexpr auto function  = "Create";

    // Create FFDC file from buffer data
    util::FFDCFile pelFile{util::FFDCFormat::Text};
    auto fd = pelFile.getFileDescriptor();

    write(fd, i_buffer.data(), i_buffer.size());
    lseek(fd, 0, SEEK_SET);

    auto filePath = pelFile.getPath();

    // Additional data for log
    std::map<std::string, std::string> additional;
    additional.emplace("RAWPEL", filePath.string());
    additional.emplace("_PID", std::to_string(getpid()));

    // Create the PEL
    try
    {
        auto bus    = sdbusplus::bus::new_default_system();
        auto method = bus.new_method_call(service, path, interface, function);
        method.append(event, level, additional);
        bus.call_noreply(method);
    }
    catch (const sdbusplus::exception::SdBusError& e)
    {
        std::stringstream ss;
        ss << "createPelRaw: " << e.what();
        trace<level::INFO>(ss.str().c_str());
    }
}

/**
 * Create a PEL from an existing PEL
 *
 * Create a new PEL based on the specified raw PEL and submit the new PEL
 * to the backend logging code as a raw PEL. Note that  additional data map
 * here contains data to be committed to the PEL and it can also be used to
 * create the PEL as it contains needed information.
 *
 * @param   i_buffer - buffer containing a raw PEL
 * @param   i_additional - additional data to be added to the new PEL
 */
void createPelCustom(std::vector<uint8_t>& i_rawPel,
                     std::map<std::string, std::string> i_additional)
{
    // create PEL object from buffer
    auto tiPel = std::make_unique<pel::PelMinimal>(i_rawPel);

    // The additional data contains the TI info as well as the value for the
    // subystem that provided the TI info. Get the subystem from additional
    // data and then populate the prmary SRC and SRC words for the custom PEL
    // based on the sybsystem's TI info.
    uint8_t subsystem = std::stoi(i_additional["Subsystem"]);
    tiPel->setSubsystem(subsystem);

    if (static_cast<uint8_t>(pel::SubsystemID::hypervisor) == subsystem)
    {
        // populate hypervisor SRC words
        tiPel->setSrcWords(std::array<uint32_t, pel::numSrcWords>{
            (uint32_t)std::stoul(i_additional["0x10 SRC Word 12"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x14 SRC Word 13"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x18 SRC Word 14"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x1c SRC Word 15"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x20 SRC Word 16"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x24 SRC Word 17"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x28 SRC Word 18"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x2c SRC Word 19"], 0, 16)});

        // populate hypervisor primary SRC
        std::array<char, pel::asciiStringSize> srcChars{'0'};
        std::string srcString = i_additional["SrcAscii"];
        srcString.copy(srcChars.data(),
                       std::min(srcString.size(), pel::asciiStringSize), 0);
        tiPel->setAsciiString(srcChars);
    }
    else
    {
        // Populate hostboot SRC words - note HB word 0 from the shared info
        // data (additional data "0x10 HB Word") is reflected in the PEL as
        // "reason code" so we zero it here. Also note that the first word
        // in this group of words starts at word 0 and word 1 does not exits.
        tiPel->setSrcWords(std::array<uint32_t, pel::numSrcWords>{
            (uint32_t)0x00000000,
            (uint32_t)std::stoul(i_additional["0x14 HB Word 2"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x18 HB Word 3"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x1c HB Word 4"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x20 HB Word 5"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x24 HB Word 6"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x28 HB Word 7"], 0, 16),
            (uint32_t)std::stoul(i_additional["0x2c HB Word 8"], 0, 16)});

        // populate hostboot primary SRC
        std::array<char, pel::asciiStringSize> srcChars{'0'};
        std::string srcString = i_additional["0x30 error_data"];
        srcString.copy(srcChars.data(),
                       std::min(srcString.size(), pel::asciiStringSize), 0);
        tiPel->setAsciiString(srcChars);
    }

    // set severity, event type and action flags
    tiPel->setSeverity(static_cast<uint8_t>(pel::Severity::termination));
    tiPel->setType(static_cast<uint8_t>(pel::EventType::na));
    tiPel->setAction(static_cast<uint16_t>(pel::ActionFlags::service |
                                           pel::ActionFlags::report |
                                           pel::ActionFlags::call));

    // The raw PEL that we used as the basis for this custom PEL contains the
    // attention handler trace data and does not needed to be in this PEL so
    // we remove it here.
    tiPel->setSectionCount(tiPel->getSectionCount() - 1);

    // Update the raw PEL with the new custom PEL data
    tiPel->raw(i_rawPel);

    // create PEL from raw data
    createPelRaw(i_rawPel);
}

/**
 * Log an event handled by the attention handler
 *
 * Basic (non TI) events will generate a standard message-registry based PEL
 *
 * TI events will create two PEL's. One PEL will be informational and will
 * contain trace information relevent to attention handler. The second PEL
 * will be specific to the TI type (including the primary SRC) and will be
 * based off of the TI information provided to the attention handler through
 * shared TI info data area.
 *
 * @param  i_event - The event type
 * @param  i_additional - Additional PEL data
 * @param  i_ffdc - FFDC PEL data
 */
void event(EventType i_event, std::map<std::string, std::string>& i_additional,
           const std::vector<util::FFDCFile>& i_ffdc)
{
    bool eventValid = false; // assume no event created
    bool tiEvent    = false; // assume not a terminate event

    std::string eventName;

    switch (i_event)
    {
        case EventType::Checkstop:
            eventName  = "org.open_power.HwDiags.Error.Checkstop";
            eventValid = true;
            break;
        case EventType::Terminate:
            eventName  = "org.open_power.Attn.Error.Terminate";
            eventValid = true;
            tiEvent    = true;
            break;
        case EventType::Vital:
            eventName  = "org.open_power.Attn.Error.Vital";
            eventValid = true;
            break;
        case EventType::HwDiagsFail:
        case EventType::AttentionFail:
            eventName  = "org.open_power.Attn.Error.Fail";
            eventValid = true;
            break;
        default:
            eventValid = false;
            break;
    }

    if (true == eventValid)
    {
        // Create PEL with additional data and FFDC data. The newly created
        // PEL's platform log-id will be returned.
        auto pelId =
            createPel(eventName, i_additional, createFFDCTuples(i_ffdc));

        // If this is a TI event we will create an additional PEL that is
        // specific to the subsystem that generated the TI.
        if (true == tiEvent)
        {
            // get file descriptor and size of information PEL
            int pelFd = getPelFd(pelId);

            // if PEL found, read into buffer
            if (-1 != pelFd)
            {
                auto pelSize = lseek(pelFd, 0, SEEK_END);
                lseek(pelFd, 0, SEEK_SET);

                // read information PEL into buffer
                std::vector<uint8_t> buffer(pelSize);
                read(pelFd, buffer.data(), buffer.size());
                close(pelFd);

                // create PEL from buffer
                createPelCustom(buffer, i_additional);
            }
        }
    }
}

/**
 * Commit special attention TI event to log
 *
 * Create a event log with provided additional information and standard
 * FFDC data plus TI FFDC data
 *
 * @param i_additional - Additional log data
 * @param i_ti_InfoData - TI FFDC data
 */
void eventTerminate(std::map<std::string, std::string> i_additionalData,
                    char* i_tiInfoData)
{
    // Create log event with aodditional data and FFDC data
    event(EventType::Terminate, i_additionalData,
          createFFDCFiles(i_tiInfoData, 0x53));
}

/** @brief Commit SBE vital event to log */
void eventVital()
{
    // Additional data for log
    std::map<std::string, std::string> additionalData;

    // Create log event with additional data and FFDC data
    event(EventType::Vital, additionalData, createFFDCFiles(nullptr, 0));
}

/**
 * Commit attention handler failure event to log
 *
 * Create an event log containing the specified error code.
 *
 * @param i_error - Error code
 */
void eventAttentionFail(int i_error)
{
    // Additional data for log
    std::map<std::string, std::string> additionalData;
    additionalData["ERROR_CODE"] = std::to_string(i_error);

    // Create log event with additional data and FFDC data
    event(EventType::AttentionFail, additionalData,
          createFFDCFiles(nullptr, 0));
}

/**
 * Parse systemd journal message field
 *
 * Parse the journal looking for the specified field and return the journal
 * data for that field.
 *
 * @param  journal - The journal to parse
 * @param  field - Field containing the data to retrieve
 * @return Data for the speciefied field
 */
std::string sdjGetFieldValue(sd_journal* journal, const char* field)
{
    const char* data{nullptr};
    size_t length{0};

    // get field value
    if (0 == sd_journal_get_data(journal, field, (const void**)&data, &length))
    {
        size_t prefix{0};

        // The data returned  by sd_journal_get_data will be prefixed with the
        // field name and "="
        const void* eq = memchr(data, '=', length);
        if (nullptr != eq)
        {
            // get just data following the "="
            prefix = (const char*)eq - data + 1;
        }
        else
        {
            // all the data (should not happen)
            prefix = 0;
            std::string value{}; // empty string
        }

        return std::string{data + prefix, length - prefix};
    }
    else
    {
        return std::string{}; // empty string
    }
}

/**
 * Gather messages from the journal
 *
 * Fetch journal entry data for all entries with the specified field equal to
 * the specified value.
 *
 * @param   field - Field to search on
 * @param   fieldValue -  Value to search for
 * @param   max - Maximum number of messages fetch
 * @return  Vector of journal entry data
 */
std::vector<std::string> sdjGetMessages(const std::string& field,
                                        const std::string& fieldValue,
                                        unsigned int max)
{
    sd_journal* journal;
    std::vector<std::string> messages;

    if (0 == sd_journal_open(&journal, SD_JOURNAL_LOCAL_ONLY))
    {
        SD_JOURNAL_FOREACH_BACKWARDS(journal)
        {
            // Get input field
            std::string value = sdjGetFieldValue(journal, field.c_str());

            // Compare field value and read data
            if (value == fieldValue)
            {
                // Get SYSLOG_IDENTIFIER field (process that logged message)
                std::string syslog =
                    sdjGetFieldValue(journal, "SYSLOG_IDENTIFIER");

                // Get _PID field
                std::string pid = sdjGetFieldValue(journal, "_PID");

                // Get MESSAGE field
                std::string message = sdjGetFieldValue(journal, "MESSAGE");

                // Get timestamp
                uint64_t usec{0};
                if (0 == sd_journal_get_realtime_usec(journal, &usec))
                {

                    // Convert realtime microseconds to date format
                    char dateBuffer[80];
                    std::string date;
                    std::time_t timeInSecs = usec / 1000000;
                    strftime(dateBuffer, sizeof(dateBuffer), "%b %d %H:%M:%S",
                             std::localtime(&timeInSecs));
                    date = dateBuffer;

                    // Store value to messages
                    value = date + " " + syslog + "[" + pid + "]: " + message;
                    messages.insert(messages.begin(), value);
                }
            }

            // limit maximum number of messages
            if (messages.size() >= max)
            {
                break;
            }
        }

        sd_journal_close(journal); // close journal when done
    }

    return messages;
}

} // namespace attn
