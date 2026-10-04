# ESP32-C3 in Logisim

**A complete ESP32-C3 microcontroller built as a digital logic circuit in Logisim Evolution. It runs code written Arduino-style and ESP-IDF-style, and passes the official RISC-V test suite.**

[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.23135545.svg)](https://doi.org/10.5281/zenodo.23135545) ![riscv-tests 50/50](https://img.shields.io/badge/riscv--tests-50%2F50-brightgreen) ![Logisim Evolution 5](https://img.shields.io/badge/Logisim%20Evolution-5-blue) ![License MIT](https://img.shields.io/badge/license-MIT-lightgrey)

**English** · [Español](README.es.md)

By [Francisco Aldunate](https://franciscoaldunate.cl) · Talca, Chile · version 0.2 · October 2026

<p align="center"><img src="docs/placa.png" alt="The ESP32-C3 board running in Logisim Evolution: the chip, an LED on every GPIO, the terminal and the debug panel" width="900"></p>

---

## What it is

It's Espressif's ESP32-C3, the Wi-Fi RISC-V microcontroller found in millions of devices, rebuilt **gate by gate** in the Logisim Evolution simulator:

- **Multicycle RISC-V RV32IMC CPU**: compressed instructions (C), multiply and divide (M), control and status registers (CSR), exceptions and interrupts.
- **System bus, boot ROM, FLASH and SRAM** at the same addresses as the real chip.
- **Digital peripherals** with the registers from Espressif's technical reference manual: GPIO with IO MUX, UART ×2, SYSTIMER, two timer groups with watchdogs, RTC_CNTL, PWM (LEDC), SPI, I2C, interrupt matrix, SYSTEM and eFuse.
- **A development board**: RESET and BOOT buttons, an LED on every GPIO, a terminal with keyboard, a 24C02 I2C memory and a 74HC595 shift register driving an LED bar.
- **An SDK** that compiles **Arduino-style** programs (`setup`/`loop`, `Serial`, `Wire`, `SPI`…), **ESP-IDF-style** programs (`app_main`, FreeRTOS, `driver/gpio`, `driver/ledc`…) or plain C, using Espressif's official compiler.
- **A transistor-level ALU version**: the 32-bit adder built from 896 CMOS transistors, plus a lab with gates, flip-flops and a counter made of transistors.

You can go from the program down to the transistor by double-clicking: `PlacaDevKit › U1 › CPU › U_ALU › SUMADOR › FA0`.

## First of its kind

As far as we could find (search done in October 2026), this is **the first ESP32 recreated in Logisim**. There are RISC-V CPUs built in Logisim, and teaching microcontrollers invented for the simulator. We found no Logisim model of **a real commercial microcontroller**, with its memory map and peripherals, that runs **code written for Arduino and ESP-IDF**. We also found no Logisim model **validated against the official `riscv-tests` suite**.

## Verified with the official RISC-V test suite

The CPU passes the official [`riscv-tests`](https://github.com/riscv-software-src/riscv-tests) for the extensions the ESP32-C3 implements. Each test is compiled, loaded into FLASH and run on the full circuit (board, chip and bus) with no shortcuts. The verdict comes out through the UART.

| Extension | Tests | Gate-level version | Transistor-ALU version |
|---|---|---|---|
| RV32I (rv32ui) | 40 | ✅ 40 / 40 | ✅ 40 / 40 |
| RV32M (rv32um) | 8 | ✅ 8 / 8 | ✅ 8 / 8 |
| RV32C (rvc, adapted) | 1 | ✅ 1 / 1 | ✅ 1 / 1 |
| fence.i (adapted) | 1 | ✅ 1 / 1 | ✅ 1 / 1 |
| **Total** | **50** | **✅ 50 / 50** | **✅ 50 / 50** |

- **Two tests were adapted, and in both the model behaves exactly like the real chip.**
  - `rvc` writes to data stored inside the code. On the ESP32-C3 that code lives in read-only FLASH.
  - `fence_i` jumps to SRAM through its data address. The ESP32-C3 only executes SRAM through its instruction address (`0x40380000`).
  - Run unmodified, the model raises exactly the exceptions the chip would (`mcause` 7 and 1).
  - The adapted versions only change where the data lives or which address is used for the jump, and test exactly the same instructions. They are in [`verificacion/env_c3/adaptados`](verificacion/env_c3/adaptados), explained line by line.
- **`ma_data` is not run.** It tests misaligned accesses, which the ESP32-C3 doesn't do in hardware (it raises an exception, like the real chip).
- **Negative control.** The harness includes a test built to fail (1 + 1 = 3). It fails exactly at the expected case, so a PASS means something.

To reproduce:
1. `git clone --recursive` this repository.
2. Install ESP-IDF and Logisim Evolution 5.
3. Run `python verificacion/correr_riscv_tests.py`.

The full results are in [`verificacion/`](verificacion).

## Gallery

**What the board's terminal prints when it boots** (real output, captured from the simulation):

```
ESP-ROM:esp32c3-logisim (modelo educativo)
rst:0x1 (POWERON),boot:0x8 (SPI_FAST_FLASH_BOOT)

Hola desde el ESP32-C3 en Logisim!
Motivo del reset: POWERON
MAC: 02:4c:47:53:43:33
misa=0x40001104  mvendorid=0x612
123456 * 789 = 97406784 ; 123456 / 789 = 156
Escribe algo en el teclado:
```

And the official ESP-IDF FreeRTOS example (`idf/tareas_y_colas`), with the real `ESP_LOGI` format:

```
I (5602) tareas: creando tareas
I (9690) tareas: 3 tareas corriendo; presiona BOOT
```

### Inside the chip

| | |
|---|---|
| <img src="docs/chip_por_dentro.png" alt="The ESP32C3 circuit: CPU, system bus, memories, peripherals, timers and interrupt system as blocks"> | **The chip, one level down.** The `ESP32C3` circuit, laid out like a floorplan, left to right: CPU, system bus, memories (boot ROM, FLASH, SRAM, RTC), peripherals, timers and reset, and the interrupt system. Double-click any block to go inside. |
| <img src="docs/alu_32_sumadores_cmos.png" alt="32 full adders in a row forming the ALU's ripple-carry adder"> | **The ALU adder of the transistor version.** 32 full adders in a row (ripple carry); the carry travels from bit 0 on the right to bit 31 on the left. Each box is the 28-transistor adder below: 896 transistors take part in every addition the CPU makes. |

### Down to the transistor

<p align="center"><img src="docs/sumador_cmos_28_transistores.png" alt="CMOS mirror full adder made of 28 P and N transistors" width="860"></p>

**A 28-transistor CMOS full adder (the "mirror adder").** A 10-transistor carry network, a 14-transistor sum network that reuses the inverted carry, and two inverters. Green wires are at 1, dark green at 0.

| | |
|---|---|
| <img src="docs/nand_cmos.png" alt="CMOS NAND gate with two P transistors in parallel and two N in series"> | **CMOS NAND, 4 transistors.** Two P in parallel pull the output to 1, two N in series pull it to 0, and they never conduct at the same time. |
| <img src="docs/contador_flipflops.png" alt="4-bit counter made of four D flip-flops and the CMOS adder"> | **Memory: a 4-bit counter.** Four D flip-flops built from transistor NANDs, plus the CMOS adder computing Q + 1 between clock edges. |

<p align="center"><img src="docs/laboratorio_transistores.png" alt="The transistor lab: NOT, NAND and NOR gates, full adder, 4-bit adder with displays and a counter" width="760"></p>

**The transistor lab** (`LaboratorioTransistores`): gates, the full adder, a 4-bit adder with hex displays and the counter with its own clock, all interactive.

### The interactive guide

`GUIDE.html` (English) and `GUIA.html` (Spanish) explain the architecture with widgets that work offline.

| | |
|---|---|
| <img src="docs/guia_paso_a_paso.png" alt="Step-by-step widget: six instructions blinking GPIO8, debug panel and timing diagram"> | **Step by step.** Six instructions that blink GPIO8, run half a clock cycle at a time, with the same debug panel the board has and a timing diagram. Here the `sw` to `GPIO_OUT_W1TS` has just turned the LED on. |
| <img src="docs/guia_camino_de_datos.png" alt="CPU datapath diagram highlighting the blocks used by a store instruction"> | **The datapath.** Pick an instruction and see which blocks and wires it uses. Here `sw a1, 8(a0)`: the address adder, the load/store unit and the bus. |
| <img src="docs/guia_sumador.png" alt="8-bit adder widget with clickable bits and the carry rippling"> | **An 8-bit adder** you can click, with the carry rippling through each full adder, signed and unsigned results and overflow. |
| <img src="docs/guia_decodificador.png" alt="Instruction decoder widget splitting a 32-bit instruction into its fields"> | **Instruction decoder.** Paste the value from the board's IR and it shows the assembly, what it does and every field of the instruction. |

---

## Quick start

1. Install [Logisim Evolution 5](https://github.com/logisim-evolution/logisim-evolution/releases).
2. Open `ESP32C3_Logisim.circ`.
3. Press **Ctrl+K** (Simulate › Auto-Tick Enabled).
4. The boot ROM writes to the terminal, then the program in FLASH starts (`hola_mundo`).
5. Click the board's keyboard and type: the program answers.

If it's slow, raise **Simulate › Auto-Tick Frequency** to the maximum. The real speed limit is your computer.

**[GUIDE.html](GUIDE.html)** explains in full how it works inside. Open it in a browser; it works offline. It has interactive widgets: the CPU step by step, the datapath, an adder and an instruction decoder. Circuit and signal names inside the model are in Spanish; the guide explains what each one means.

## What's in the folder

| | |
|---|---|
| `ESP32C3_Logisim.circ` | The board with the chip. Start here. |
| `ESP32C3_Logisim_ALU_transistores.circ` | Same, but the ALU adder is built from 896 transistors (32 adders of 28). A bit slower. |
| `GUIDE.html` · `GUIA.html` | The architecture guide in English and Spanish. |
| `programas/` | The 18 examples, precompiled (`.hex`), in `c/`, `arduino/` and `idf/`. |
| `sdk/` | To compile your own programs: `compilar.py`, the Arduino and ESP-IDF layers and the source of every example. |
| `verificacion/` | The official `riscv-tests` suite adapted to the model, the harness that runs it, and the results. |

## Looking inside

- **Debug panel** (on the board, right of the chip): PC, CPU phase (BUSCANDO = fetch / EJECUTANDO = execute), instruction, the last ALU operation (A, operation, B, result), the register written, bus activity, and cycle and instruction counters.
- **State tab** (bottom left, next to Properties): registers `x01_ra` … `x31_t6`, the IR, the phase and the CSRs (`mepc`, `mcause`, `mtvec`…), live.
- **Simulate tab** (top left): double-click `PlacaDevKit > U1 > CPU > …` to enter any block **without stopping** the simulation.
- **Step by step**: Ctrl+K stops the clock, **Ctrl+T** advances half a cycle, **Ctrl+F9** a full cycle. **Ctrl+E** disables automatic propagation and each **Ctrl+I** moves the signals one gate further.
- **Transistor lab**: Design tab, double-click `LaboratorioTransistores`.

## Compiling your own programs

You need [ESP-IDF](https://docs.espressif.com/projects/esp-idf/) installed: it provides Espressif's RISC-V compiler and Python. Works on Windows, macOS and Linux.

```
cd sdk
python compilar.py arduino/blink
```

Open `arduino/blink/logisim/blink.circ`: it's the circuit with your program already loaded. Press Ctrl+K.

`compilar.py` detects the style by itself:

| Style | Needs | Example |
|---|---|---|
| Arduino | `setup()` and `loop()` in an `.ino` | `python compilar.py arduino/my_sketch` |
| ESP-IDF | `app_main()` in a `.c` | `python compilar.py idf/my_app` |
| C | `main()` using `sdk.h` | `python compilar.py examples/my_test` |

All 18 examples compile with zero warnings on Espressif's GCC 15.2. They cover:
- blink;
- serial echo;
- PWM breathing;
- the three timers and the watchdog;
- interrupts;
- SPI and I2C;
- FreeRTOS tasks and queues fed from an ISR;
- the official ESP-IDF `hello_world` and `blink`.

## Differences from the real chip

- **Time runs in slow motion**: 1 ms of the program = 1 model cycle.
- **FreeRTOS is cooperative**: tasks switch on `vTaskDelay`, queue/semaphore waits or `taskYIELD()`.
- **No Wi-Fi, Bluetooth or ADC** (radio and analog circuits aren't digital logic).
- **Fixed-speed UART** (~40 cycles per character) and **8-bit hardware PWM**.
- **Programs are built from source** with `compilar.py`. Binaries from `idf.py build` or the Arduino IDE don't run on the model.
- **The CPU is multicycle** (~2.2 cycles per instruction), not a 160 MHz pipeline.
- **Some parts use Logisim's built-in components** (registers, memories, multiplier, shifter) so simulation doesn't take forever. The ALU adder is hand-built from gates, and from transistors in the other version.

## Author

**Francisco Aldunate Rodríguez** · Talca, Chile
ESP32 firmware (P4, S3, C3) and web developer.
Portfolio: **[franciscoaldunate.cl](https://franciscoaldunate.cl)** · GitHub: [@franciscoaldun](https://github.com/franciscoaldun)

Related projects:
- [4-bit adder built only from transistors](https://github.com/franciscoaldun/sumador-4-bits-transistores), the predecessor of this one;
- [ESP32-C3 atomic clock](https://github.com/franciscoaldun/reloj-atomico-esp32c3);
- [ESP32-C3 fractal oven](https://github.com/franciscoaldun/horno-fractal-esp32c3).

## How to cite

If you use it in teaching, research or a publication, please cite it. GitHub also offers this under "Cite this repository", from [`CITATION.cff`](CITATION.cff):

> Aldunate Rodríguez, F. (2026). *ESP32-C3 in Logisim: a gate-level ESP32-C3 microcontroller for Logisim Evolution* (version 0.2). https://doi.org/10.5281/zenodo.23135545

## License

- **This project:** [MIT](LICENSE).
- **`riscv-tests`** (in `verificacion/riscv-tests`, as a submodule): keeps its own BSD license.
- **ESP32-C3** is a product of Espressif Systems. This is an independent educational model, not affiliated with Espressif.
