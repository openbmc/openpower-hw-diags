#include <attn/attn_logging.hpp>

#include <iostream>

namespace attn
{

/** @brief Log message of type INFO using stdout */
template <>
void trace<INFO>(const char* i_message)
{
    std::cout << "trace: " << i_message << std::endl;
}

/** @brief Log message of type ERROR using stdout */
template <>
void trace<ERROR>(const char* i_message)
{
    std::cout << "error: " << i_message << std::endl;
}

void eventAttentionFail(int i_error)
{
    std::cout << "event: attention fail" << i_error << std::endl;
}

void eventTerminate(std::map<std::string, std::string> i_additionalData,
                    char* i_tiInfoData)
{
    std::cout << "event: terminate" << std::endl;

    std::map<std::string, std::string>::iterator itr;
    for (itr = i_additionalData.begin(); itr != i_additionalData.end(); ++itr)
    {
        std::cout << '\t' << itr->first << '\t' << itr->second << '\n';
    }
    std::cout << std::endl;

    if (nullptr != i_tiInfoData)
    {
        std::cout << "TI data present" << std::endl;
    }
}

uint32_t eventVital()
{
    std::cout << "event: vital" << std::endl;
    return 0;
}

void eventPhalSbeChipop(uint32_t proc)
{
    std::cout << "event: sbe timeout proc " << proc << std::endl;
}

} // namespace attn
