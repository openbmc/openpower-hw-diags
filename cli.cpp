#include <algorithm>
#include <string>

/** @brief Search the command line arguments for an option */
bool getCliOption(char** i_begin, char** i_end, const std::string& i_option)
{
    return (i_end != std::find(i_begin, i_end, i_option));
}

/** @brief Search the command line arguments for a setting value */
char* getCliSetting(char** i_begin, char** i_end, const std::string& i_setting)
{
    char** value = std::find(i_begin, i_end, i_setting);
    return (value != i_end && ++value != i_end) ? *value : 0;
}

/** @brief Parse command line for configuration flags */
void parseConfig(char** i_begin, char** i_end, bool& i_vital, bool& i_checkstop,
                 bool& i_terminate, bool& i_breakpoints)
{
    char* setting;

    setting = getCliSetting(i_begin, i_end, "--vital");
    if (nullptr != setting)
    {
        i_vital = std::string("off") == setting ? false : i_vital;
    }

    setting = getCliSetting(i_begin, i_end, "--checkstop");
    if (nullptr != setting)
    {
        i_checkstop = std::string("off") == setting ? false : i_checkstop;
    }

    setting = getCliSetting(i_begin, i_end, "--terminate");
    if (nullptr != setting)
    {
        i_terminate = std::string("off") == setting ? false : i_terminate;
    }

    setting = getCliSetting(i_begin, i_end, "--breakpoints");
    if (nullptr != setting)
    {
        i_breakpoints = std::string("off") == setting ? false : i_breakpoints;
    }
}
