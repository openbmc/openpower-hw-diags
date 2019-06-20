# Hardware error attention handling design for POWER Systems

Several hardware attentions are OR'd to a single GPIO pin. The GPIO monitor
will call this application if the pin becomes active. This application will
take the steps below to handle the following attentions:

    * SBE Vital Attention
    * Checkstop
    * Recoverable
    * Special Attention
    * Hostboot TI
    * PHYP TI
    * OPAL TI
    * PHYP Breakpoint
    * PHYP Corecode

1. Query interrupt status registers to determine active attentions.

2. If there is an active SBE Vital Attentions ... TBD

3. If the attention is Checkstop or TI, look for recoverable errors and ... TBD

4. If the attention is a Special Attention ... TBD

4. If the attention is a PHYP Breakpoint, notify the breakpoint handler ... TBD

5. If the attention is PHYP Corecode ... TBD

6. If the attention was a TI ... TBD

7. If the attention was not a TI, exit.
