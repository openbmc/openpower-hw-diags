# Introduction
This document describes the architecture and design for the Attention Handler
application. The Attention Handler provides support for platform error handling
and runtime diagnostics. The Attention Handler utilizes hardware status and
data, and interacts with other firmware components to meet the overall platform
error handling objectives.

The purpose of this document is to describe the architecture and design of the
Attention Handler component. And to do this in a way to meet the needs of the
Attention Handler developers as well as the developers of the associated
software and firmware components, namely:

* SBE
* CRONUS
* PHYP
* OPAL
* Hostboot
* HEI
* Hardware Diagnostics
* PDBG
* PHAL

# Design Goals
Continually evolve the Attention Handler firmware toward meeting its overall
objective as well as the objectives of the associated firmware components. Keep
all development efforts in alignment with the following practices:

* Minimize complexity.
* Maximize reusability.
* Minimize throw-away.

# High-Level Design
The attention handler will listen for a notification from the GPIO handler
indicating that a system error or breakpoint event has occurred. The attention
handler will then query the hardware to determine the specific event or events
that caused the notification and attempt to gracefully handle the following:

* Vital Attention
* TI Hostboot
* TI PHYP
* TI OPAL
* Checkstop
* PHYP BreakPoint
* PHYP CoreCode
* Unknown Special Attention

The first step the attention handler takes is to read from the CFAM registers to
determine if there are any Special Attention or Vital Attention events active.
In the case of any active Vital Attention the associated SBE will be noted as
"unusable" and will be ommited from any further event analyses.

If it is determined that any Special Attention is active, the attention handler
will query each "usable" SBE as to whether it has an active event that needs to
be be handled. This done by issuing a SBE ChipOp for special attentions.

Error events reported by the SBE will be further analysed to determine root
cause and the error event will be recorded in an error event log. In the case of
TI errors, additional relevent data may also be recorded in the event log.

If an SBE reports a breakpoint event the attention handler will notify the
breakpoint handler and provide it with the associated processor, core and
thread. This communication is via D-BUS.

If the special attention type was not a known error type or breakpoint the
attention handler will note the event as "unknown" and may log the event.

After the attention handler has addressed all pending events it will return to
waiting for another attention event. If the attention handler has processed a TI
event the system may ultimately reboot.

# Key Components
The Attention Handler is responsible for handling error and breakpoint events by
interacting with hardware, software and firmware components. Each of the key
hardware/software/firmware components that will play a role in the Attention
Handler design is discussed here.

## CFAM - Common FRU Access Module
The Attention Handler will query the CFAM to determine whether there are any
active attentions.

## SBE - Self Boot Engine
The Attention Handler will send commands to and receive data from the SBE to
help determine the course of action needed to successfully handle the event.

## CRONUS - A hardware breakpoint handler
When the Attention Handler is notified of a breakpoint event it will notify
CRONUS with the processor, core and thread that triggered the breakpoint.

## Hardware Diagnostics - Error diagnostics and isolation application
WHen the Attention Handler is notified of an error event it will notify Hardware
Diagnostics with error information received from the SBE.

## PHYP - A hypervisor
(tbd)

## OPAL - A firmware abstraction layer
(tbd)

## PHAL - A hardware abstraction layer
(tbd)

## Hostboot - A bootloader
(tbd)

## HEI - Hardware Error Isolator
(tbd)

## PDBG - A debug interface
(tbd)
