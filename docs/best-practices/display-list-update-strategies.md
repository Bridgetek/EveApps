# Display-list update strategies

The EVE graphics engine scans the active display list for every LCD frame.
The host decides when to construct and swap a new list. A stable list can keep
driving the display while the host polls input or changes data in RAM_G that
the list references. This choice applies across EveApps projects; select it
from the behavior and constraints of the new application.

## Choose the host update rule

| Rule | Use when | Host responsibility |
| --- | --- | --- |
| Rebuild continuously | Display-list commands change frequently, such as moving geometry, text, touch tags, or animation state. | Set a suitable update interval and budget the full list and swap on each iteration. The achieved rate includes drawing and command-processing time. |
| Rebuild on change | The display-list commands are usually stable. | Mark the list dirty for every change requiring new commands, including page, text, layout, tag, and touch-feedback changes. Include periodic triggers for time-dependent content. |
| Keep a list while changing referenced data | Dynamic bitmap or other RAM_G content can change without changing the commands that draw it. | Own and update the live buffers safely while EVE scans them. Combine this technique with either rebuild rule above. |

These are host scheduling choices, not LCD refresh modes. In every case EVE
continues scanning the active list. A missed dirty trigger leaves stale list
content on screen, while a RAM_G update can become visible without a swap.

Fewer rebuilds can reduce host work and command-FIFO traffic. They do not
necessarily reduce external-flash reads during scanout: a retained list may
still reference flash assets on every LCD frame. Measure underrun behavior on
the intended EVE IC and physical display rather than inferring it from the
host redraw rate or emulator.

For live RAM_G data, account for writes concurrent with scanout. Keep writes
bounded, consider double buffering when an intermediate image is unacceptable,
and inspect the target display for tearing or partially updated graphics.
Check RAM_G allocation and any buffering scheme against the exact device.

For a substantial HMI project, record the selected rule, its redraw interval
or dirty triggers, ownership of live buffers, and hardware observations. A
small instructional sample may state the rule in a brief code comment.

## Examples in this repository

- [EV_HMI's main loop](../../DemoApps/AI-generated/EV_HMI/Src/EVCharger.c#L799)
  calls `draw()` every iteration and sleeps 33 ms. Its
  [draw function](../../DemoApps/AI-generated/EV_HMI/Src/EVCharger.c#L622)
  constructs and swaps a full list. It illustrates continuous rebuilding;
  the actual rate depends on drawing and command-processing time.
- [PatientMonitor's main loop](../../DemoApps/AI-generated/PatientMonitor/Src/PatientMonitor.c#L239)
  sets `dirty` for touch-state changes and a one-second numeric update, then
  calls `frame()` only when dirty. Its
  [waveform updater](../../DemoApps/AI-generated/PatientMonitor/Src/PatientMonitorWave.c#L77)
  writes bitmap data in RAM_G independently. It illustrates a change-triggered
  list with live data; it does not establish tear-free behavior on other
  hardware.

See Bridgetek's [FT81x Programmer Guide](https://brtchip.com/wp-content/uploads/Support/Documentation/Programming_Guides/ICs/EVE/FT81X_Series_Programmer_Guide.pdf)
for the general display-list and swap model. Verify commands, registers,
memory limits, and diagnostics for the project's selected EVE IC.
