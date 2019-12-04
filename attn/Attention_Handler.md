# Hardware error attention handling design for POWER Systems


Attentions are host error and debug breakpoint conditions that can be handled by software running on the BMC. The host alerts the BMC of these conditions via the Attention GPIO pin. The Attention Handler application (Attn) monitors this gpio for activity and services active attentions. When activity is detected on the Attention GPIO, Attn will query the hardware to determine active attention that needs to be serviced.

***
**The following logic is implemented in Attn to allow for controlling the flow of attention handling once an attention event has been detected**:

- Attn can adjust the handling priority of active attention events.

- Attn can choose to defer or ignore handling of attention events.

- Attn can handle any number of active attentions per GPIO event.

- Attn can continue monitoring or stop monitoring the attention GPIO.

***
**List of attention conditions and actions (by default priority)**

1. SBE Attention (vital): This attention indicates that the SBE is in a state that is considered non-functional. Attn will log this event and *... TBD*

2. System Checkstop (checkstop): This attention indicates an error condition in which the host cannot continue operating properly. Attn will launch the Hardware  Diagnostics application (Hwdiags) and may provide Hwdiags with information need ed to help it analyze the checkstop *... TBD*

3. Special Attention (special): This attention indicates that one of the following conditions has occurred.

    - Cronus Breakpoint (breakpoint): Attn will notify Cronus with the processor, core and thread number associated with the event.

    - PHYP Terminate Immediate (PHYP TI): Attn will log this event.

    - OPAL Terminate Immediate (OPAL TI): Attn will log this event.

    - Core Code to SP (PHYP CoreCode): Attn will log this event.

    - Other (instruction stop, core recovery handshake): Attention will log these events.

4. Recoverable Error (recoverable): This attention indicates that a host error has occurred and the host is able to continue operating. Attn will log this condition.

***
**Command Line Interface**

Attn supports the following command line interface *... TBD*

***
**Building**

This application is built using the standard Meson/Ninja build setup and will dynamically links against the POWER Debug (pdbg) library *(version # TBD)*. This application can also be statically linked against pdbg *(see meson.build)*.

	meson build && cd build && ninja
