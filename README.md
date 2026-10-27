# terminal-controlled-stm

A small command-line shell for the STM32F446 (Nucleo-F446RE), written bare metal with direct register access and no HAL. Connect over the ST-LINK virtual COM port, type commands, and the board responds.

## Commands

| Command | Description |
| --- | --- |
| `help [command]` | List all commands, or show details for one |
| `led <on\|off\|toggle\|blink [ms]>` | Control the green user LED (LD2) |
| `uptime` | Show milliseconds since reset |
| `echo [text...]` | Print the text back |

`led blink` toggles the LED every `ms` milliseconds (default 1000). `led on`, `led off` and `led toggle` stop blinking.

The shell supports backspace and accepts `\r`, `\n` or `\r\n` line endings.

## How it works

- **UART:** USART2 on PA2/PA3 at 115200 8N1. Both directions are interrupt-driven through ring buffers: the RX interrupt fills the receive buffer, and the main loop pops bytes, builds a line, and dispatches it to a command handler. Output is queued in a transmit buffer that the TXE interrupt drains.
- **Clock:** runs on the 16 MHz internal oscillator; SysTick provides a 1 ms tick.
- **LED blink:** non-blocking; the main loop checks the tick count each pass and toggles the LED when the interval has elapsed, so the shell stays responsive while blinking.

## Building

Requires `arm-none-eabi-gcc`, CMake and Ninja.

```sh
cmake --preset Debug
cmake --build --preset Debug
```

The firmware is written to `build/Debug/terminal-controlled-stm.elf`. Flash it with STM32CubeProgrammer or your preferred tool.

## Connecting

Open the board's serial port at 115200 baud, for example:

```sh
picocom -b 115200 /dev/ttyACM0
```

## Tests

The ring buffer has host-side unit tests in `tests/`:

```sh
cd tests
cmake -B out && cmake --build out && ctest --test-dir out
```
