# Hardware error attention handling design for POWER Systems

Several hardware attentions are OR'd to a single GPIO pin. The GPIO monitor
will call this application if the pin becomes active. This application will
take the steps below to handle System Checkstop (checkstop), Breakpoint (BP),
Terminate Immediately (TI), SBE Attention (vital), CoreCodeToSp (corecode).

* Checkstop: Notify hardware diagnostics application (hw-diags).

* BP: Clear attention status and notify breakpoing handler (cronus).

* TI (hostboot, phyp, opal): Write associated memory dump to a file.

* Vital: Clear attention status and ... TBD

* Corecode: Clear attention status.


