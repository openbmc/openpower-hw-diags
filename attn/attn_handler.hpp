#pragma once

static const int EVT_NA         = 0;
static const int EVT_CHECKSTOP  = 1;
static const int EVT_TI_HOST    = 2;
static const int EVT_TI_PHYP    = 3;
static const int EVT_BP_PHYP    = 4;
static const int EVT_VITAL      = 5;

// todo
int getEvent();

// todo
void isolateEvent( const int i_event );

//todo
void logEvent( const int i_event );

// todo
void handleCheckstop();

// todo
void handleTi( const int i_event );

// todo
void handleBreakpoint();

// todo
void handleVital();
