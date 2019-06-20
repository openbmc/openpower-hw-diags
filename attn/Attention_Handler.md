# Attention Handler (ATTN)

## High Level Design

The following events will be reported to the BMC via a single GPIO pin:
 * Checkstop attention
 * Host TI
 * PHYP TI
 * PHYP breakpoint
 * SBE vital attention

When the GPIO Monitor detects an event, it will tell ATTN to query each SBE for
the type of event that occurred.

First, ATTN must check if any SBEs are reporting vital attentions because they
are inaccessible when there is a vital attention. If there is a vital attention,
ATTN will **TBD** and cease any further queries on that chip.
**TBD:** How will the query be done?
**TBD:** If there is a vital attention on a chip, is there any point to looking
         at attentions on the other chips?

If there an no vital attentions, ATTN will issue a chip-op to each SBE (via
libpdbg API). The SBE will return data indicating the active event, if any, on
the chip. Also, the SBE will return additional data for debug if there was a TI.
**TBD:** What are the chip-op details?
**TBD:** What is the format of the data returned?
**TBD:** What does ATTN do with the extra TI data?

### Actions for Checkstop Attention

ATTN will simply tell the Analyzer to isolate the error and generate a log.

### Actions for Host and PHYP TI

ATTN will tell the Analyzer to isolate the error and generate a log.
**TBD:** Is there anything else to do?

### Actions for PHYP breakpoint

**TBD:** What should be done?

