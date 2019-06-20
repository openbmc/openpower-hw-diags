#include "attn_handler.hpp"

int main()
{
    int event = getEvent();

    switch( event )
    {
        case EVT_CHECKSTOP:
            handleCheckstop();
            break;

        case EVT_TI_HOST:
        case EVT_TI_PHYP:
            handleTi( event );
            break;

        case EVT_BP_PHYP:
            handleBreakpoint();
            break;

        case EVT_VITAL:
            handleVital();
            break;

        default:
            return 1; // should not get here
    }

    return 0;
}


// todo
int getEvent()
{
    return EVT_NA;
}


// todo
void isolateEvent( const int i_event )
{
    if ( EVT_CHECKSTOP == i_event || \
         EVT_TI_HOST == i_event || \
         EVT_TI_PHYP == i_event )
    {
        // do something
    }

    return;
}


// todo
void logEvent( const int i_event )
{
    if ( EVT_CHECKSTOP == i_event || \
         EVT_TI_HOST == i_event || \
         EVT_TI_PHYP == i_event )
    {
        // do something
    }

    return;
}


// todo
void handleCheckstop()
{
    isolateEvent( EVT_CHECKSTOP );
    logEvent( EVT_CHECKSTOP );
    return;
}


// todo
void handleTi( const int i_event )
{
    isolateEvent( i_event );
    logEvent( i_event );
    return;
}


// todo
void handleBreakpoint()
{
    return;
}


// todo
void handleVital()
{
    return;
}
