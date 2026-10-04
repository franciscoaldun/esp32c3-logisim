# Verification with riscv-tests · Verificación con riscv-tests

**English** · [Español](#español)

This folder runs the official [`riscv-tests`](https://github.com/riscv-software-src/riscv-tests) suite on the Logisim model of the ESP32-C3. Every test is compiled with Espressif's GCC, loaded into the model's FLASH ROM and executed by the complete circuit (board → chip → bus → CPU) in Logisim Evolution's headless mode. The test reports its verdict through UART0 and the harness reads it from the board's terminal.

| File | What it is |
|---|---|
| `correr_riscv_tests.py` | The harness: compiles every test, runs it in Logisim (`--tty tty --load <hex> FLASH`) and reads `PASS` / `FAIL`. Runs several Logisim instances in parallel. |
| `env_c3/riscv_test.h` | Test environment for this chip: boots through the model's ROM ("direct boot" header, entry at `0x42000008`), copies `.data` to SRAM, installs a trap vector and prints the verdict on UART0. |
| `env_c3/link.ld`, `entry.S` | Memory layout identical to the real ESP32-C3: code in FLASH (`0x42000000`), data in SRAM (`0x3FC80000`). |
| `env_c3/adaptados/` | The two tests that assume things the ESP32-C3 doesn't allow, adapted with the minimum change (see the header of each file). |
| `env_c3/control/` | Negative control: a test that **must** fail (1 + 1 = 3). |
| `resultado_compuertas.txt` | Results of the gate-level model (`ESP32C3_Logisim.circ`). |
| `resultado_transistores.txt` | Results of the transistor-ALU model (`ESP32C3_Logisim_ALU_transistores.circ`). |
| `riscv-tests/` | The official suite (git submodule, BSD license). |

### Run it

```
git clone --recursive https://github.com/franciscoaldun/esp32c3-logisim
cd esp32c3-logisim/verificacion
python correr_riscv_tests.py -j 4                                          # gate-level model
python correr_riscv_tests.py -j 4 --circ ../ESP32C3_Logisim_ALU_transistores.circ
python correr_riscv_tests.py --solo add,mul,rvc_c3                         # a few tests
```

Needs ESP-IDF (for `riscv32-esp-elf-gcc`) and Logisim Evolution 5. If Logisim isn't in `C:\Program Files\logisim-evolution`, set the `LOGISIM` environment variable to its executable. Each test takes 20–60 s of simulation.

### Which tests and why

- **Run as-is:** all of `rv32ui` except `fence_i` and `ma_data` (40 tests) and all of `rv32um` (8 tests).
- **Adapted:**
  - `rvc` → `adaptados/rvc_c3.S`. The original stores writable data inside the code section. On the ESP32-C3 the code runs from read-only FLASH, so the write raises a store access fault (`mcause` 7), as on the real chip. The writable data moves to `.data`; the instructions tested are the same.
  - `fence_i` → `adaptados/fence_i_c3.S`. The original jumps to self-modified code through SRAM's data address. The ESP32-C3 executes SRAM only through its instruction address (`0x40380000`); the data address gives an instruction access fault (`mcause` 1), as on the real chip. The jump adds the offset between the two views; the stores, the `fence.i` and the checks are the original ones.
- **Not run:**
  - `ma_data` (misaligned loads and stores): the ESP32-C3 doesn't support them in hardware.
  - `rv32mi` / `rv32si`: they need a privileged test environment that this harness doesn't provide yet.

---

## Español

Esta carpeta corre la suite oficial [`riscv-tests`](https://github.com/riscv-software-src/riscv-tests) sobre el modelo del ESP32-C3 en Logisim. Cada prueba se compila con el GCC de Espressif, se carga en la ROM FLASH del modelo y la ejecuta el circuito completo (placa → chip → bus → CPU) en el modo sin ventana de Logisim Evolution. La prueba informa su resultado por la UART0 y el arnés lo lee de la terminal de la placa.

| Archivo | Qué es |
|---|---|
| `correr_riscv_tests.py` | El arnés: compila cada prueba, la corre en Logisim (`--tty tty --load <hex> FLASH`) y lee `PASS` / `FAIL`. Corre varias instancias de Logisim en paralelo. |
| `env_c3/riscv_test.h` | El entorno de pruebas para este chip: arranca por la ROM del modelo (cabecera «direct boot», entrada en `0x42000008`), copia `.data` a la SRAM, instala un vector de excepciones e imprime el resultado por la UART0. |
| `env_c3/link.ld`, `entry.S` | La memoria igual que en el ESP32-C3 real: código en la FLASH (`0x42000000`), datos en la SRAM (`0x3FC80000`). |
| `env_c3/adaptados/` | Las dos pruebas que suponen cosas que el ESP32-C3 no permite, adaptadas con el cambio mínimo (ver la cabecera de cada archivo). |
| `env_c3/control/` | Control negativo: una prueba que **tiene** que fallar (1 + 1 = 3). |
| `resultado_compuertas.txt` | Resultados del modelo con compuertas (`ESP32C3_Logisim.circ`). |
| `resultado_transistores.txt` | Resultados del modelo con la ALU de transistores (`ESP32C3_Logisim_ALU_transistores.circ`). |
| `riscv-tests/` | La suite oficial (submódulo de git, licencia BSD). |

### Correrla

```
git clone --recursive https://github.com/franciscoaldun/esp32c3-logisim
cd esp32c3-logisim/verificacion
python correr_riscv_tests.py -j 4                                          # modelo con compuertas
python correr_riscv_tests.py -j 4 --circ ../ESP32C3_Logisim_ALU_transistores.circ
python correr_riscv_tests.py --solo add,mul,rvc_c3                         # algunas pruebas
```

Necesitas ESP-IDF (por `riscv32-esp-elf-gcc`) y Logisim Evolution 5. Si Logisim no está en `C:\Program Files\logisim-evolution`, pon la ruta de su ejecutable en la variable de entorno `LOGISIM`. Cada prueba tarda entre 20 y 60 s de simulación.

### Qué pruebas y por qué

- **Se corren tal cual:** todo `rv32ui` menos `fence_i` y `ma_data` (40 pruebas) y todo `rv32um` (8 pruebas).
- **Adaptadas:**
  - `rvc` → `adaptados/rvc_c3.S`. El original guarda datos escribibles dentro del código. En el ESP32-C3 el código corre desde la FLASH, que es de solo lectura, así que esa escritura da «store access fault» (`mcause` 7), igual que en el chip real. Los datos escribibles pasan a `.data`; las instrucciones probadas son las mismas.
  - `fence_i` → `adaptados/fence_i_c3.S`. El original salta al código que se modificó a sí mismo por la dirección de datos de la SRAM. El ESP32-C3 solo ejecuta la SRAM por su dirección de instrucciones (`0x40380000`); por la de datos da «instruction access fault» (`mcause` 1), igual que el chip real. El salto suma el desplazamiento entre las dos vistas; las escrituras, el `fence.i` y las verificaciones son las originales.
- **No se corren:**
  - `ma_data` (lecturas y escrituras desalineadas): el ESP32-C3 no las hace por hardware.
  - `rv32mi` / `rv32si`: necesitan un entorno de pruebas privilegiado que este arnés todavía no tiene.
