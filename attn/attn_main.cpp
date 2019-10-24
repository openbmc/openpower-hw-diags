#include <attn_handler.hpp>

/**
 * @brief Attention handler application main()
 *
 * This is the main interface to the Attention handler application.
 */
int main()
{
    int rc = 0;

    rc = attnHandler(); // handle active attentions

    return rc;
}
