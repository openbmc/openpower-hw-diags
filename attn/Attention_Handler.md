# Hardware error attention handling design for POWER Systems

Attentions are host error and debug breakpoint conditions that can be handled
by software running on the BMC. The host alerts the BMC of these conditions
via the attention GPIO pin. When the attention GPIO pin becomes active the
BMC gpio handler daemon will launch the Attention Handler application (Attn).
Attn then queries the hardware to determine the highest priority attention
that needs to be handled and services it. Attn may handle multiple active
attentions if possible.

The following is a breakdown of the attention conditions that may be present
and the actions Attn will take to service them. These are ordered by priority.

1. SBE Attention (vital). This attention indicates that the SBE
is in a state that is considered non-functional. Attn will ... TBD

2. System Checkstop (checkstop). This attention indicates an error condition
in which the host cannot continue operating properly. Attn will launch the
Hardware Diagnostics application (Hwdiags) and may provide Hwdiags with
information needed to help it analyze the checkstop.

3. Special Attention (special). This attention indicates that one of the
following conditions has occurred.

    a. Cronus Breakpoint (breakpoint). Attn will clear the associated
    attention status bits and notify Cronus with the processor, core and
    thread number associated with the event.

    b. PHYP Terminate Immediate (PHYP TI). Attn will ... TBD

    c. OPAL Terminate Immediate (OPAL TI). Attn will ... TBD

    d. Core Code to SP (PHYP CoreCode). Attn will ... TBD

    e. Other (instruction stop, core recovery handshake). Attention will log
    this event and clear the associated attention status bits.

4. Recoverable Error. This attention indicates that a host error has occurred
and the host is able to continue operating. Attn will log this condition and
clear the associated attention status bits.

After handling the above attentions the application will exit via the return
instruction. Attn does not take any command line parameters and returns a
non-zero error code in the case of any error conditions encountered while
handling the active attention(s).

This application is built using the standard Meson/Ninja build setup and will
dynamically links against the POWER Debug (pdbg) library (version #TBD).

This application can also be statically linked against pdbg (see meson.build).
