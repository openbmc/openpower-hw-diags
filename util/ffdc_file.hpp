#pragma once

#include "util/file_descriptor.hpp"
#include "util/temporary_file.hpp"
#include "xyz/openbmc_project/Logging/Create/server.hpp"

#include <cstdint>
#include <filesystem>

namespace util
{

namespace fs = std::filesystem;
using FFDCFormat =
    sdbusplus::xyz::openbmc_project::Logging::server::Create::FFDCFormat;

/**
 * @class FFDCFile
 *
 * File that contains FFDC (first failure data capture) data.
 *
 * This class is used to store FFDC data in an error log.  The FFDC data is
 * passed to the error logging system using a file descriptor.
 *
 * The constructor creates the file and opens it for both reading and writing.
 *
 * Use getFileDescriptor() to obtain the file descriptor needed to read or write
 * data to the file.
 *
 * Use remove() to delete the file.  Otherwise the file will be deleted by the
 * destructor.
 *
 * FFDCFile objects cannot be copied, but they can be moved.  This enables them
 * to be stored in containers like std::vector.
 */
class FFDCFile
{
  public:
    // Specify which compiler-generated methods we want
    FFDCFile() = delete;
    FFDCFile(const FFDCFile&) = delete;
    FFDCFile(FFDCFile&&) = default;
    FFDCFile& operator=(const FFDCFile&) = delete;
    FFDCFile& operator=(FFDCFile&&) = default;
    ~FFDCFile() = default;

    /**
     * Constructor.
     *
     * Creates the file and opens it for both reading and writing.
     *
     * Throws an exception if an error occurs.
     *
     * @param format format type of the contained data
     * @param subType format subtype; used for the 'Custom' type
     * @param version version of the data format; used for the 'Custom' type
     */
    explicit FFDCFile(FFDCFormat format, uint8_t subType = 0,
                      uint8_t version = 0);

    /**
     * Returns the file descriptor for the file.
     *
     * The file is open for both reading and writing.
     *
     * @return file descriptor
     */
    int getFileDescriptor() const
    {
        // Return the integer file descriptor within the FileDescriptor object
        return descriptor();
    }

    /**
     * Returns the format type of the contained data.
     *
     * @return format type
     */
    FFDCFormat getFormat() const
    {
        return format;
    }

    /**
     * Returns the absolute path to the file.
     *
     * @return absolute path
     */
    const fs::path& getPath() const
    {
        return tempFile.getPath();
    }

    /**
     * Returns the format subtype.
     *
     * @return subtype
     */
    uint8_t getSubType() const
    {
        return subType;
    }

    /**
     * Returns the version of the data format.
     *
     * @return version
     */
    uint8_t getVersion() const
    {
        return version;
    }

    /**
     * Closes and deletes the file.
     *
     * Does nothing if the file has already been removed.
     *
     * Throws an exception if an error occurs.
     */
    void remove();

  private:
    /**
     * Format type of the contained data.
     */
    FFDCFormat format{FFDCFormat::Text};

    /**
     * Format subtype; used for the 'Custom' type.
     */
    uint8_t subType{0};

    /**
     * Version of the data format; used for the 'Custom' type.
     */
    uint8_t version{0};

    /**
     * Temporary file where FFDC data is stored.
     *
     * The TemporaryFile destructor will automatically delete the file if it was
     * not explicitly deleted using remove().
     */
    TemporaryFile tempFile{};

    /**
     * File descriptor for reading from/writing to the file.
     *
     * The FileDescriptor destructor will automatically close the file if it was
     * not explicitly closed using remove().
     */
    FileDescriptor descriptor{};
};

using FFDCTuple =
    std::tuple<FFDCFormat, uint8_t, uint8_t, sdbusplus::message::unix_fd>;

/** Transforms a list of FFDC files to a list of FFDC tuples. */
inline void transformFFDC(const std::vector<FFDCFile>& i_files,
                          std::vector<FFDCTuple>& o_tuples)
{
    o_tuples.clear();

    std::transform(i_files.begin(), i_files.end(), std::back_inserter(o_tuples),
                   [](const auto& e) {
                       return FFDCTuple(
                           e.getFormat(), e.getSubType(), e.getVersion(),
                           sdbusplus::message::unix_fd(e.getFileDescriptor()));
                   });
}

} // namespace util
