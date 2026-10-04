#!/usr/bin/env python3
"""Compila la suite oficial riscv-tests (rv32ui, rv32um, rv32uc) y la corre en el ESP32-C3 de Logisim.

Cada prueba se carga en la ROM FLASH (logisim --tty tty --load) y se espera a que la UART diga
PASS o FAIL <n>. Uso: python correr_riscv_tests.py [--circ X.circ] [--solo add,sub] [-j 4]
"""
import os, sys, glob, struct, subprocess, argparse, time, threading, queue
from concurrent.futures import ThreadPoolExecutor

AQUI = os.path.dirname(os.path.abspath(__file__))
TESTS = os.path.join(AQUI, "riscv-tests")
ENV = os.path.join(AQUI, "env_c3")
LOGISIM = os.environ.get("LOGISIM", r"C:\Program Files\logisim-evolution\logisim-evolution.exe")
CC = glob.glob(os.path.expanduser(r"~\.espressif\tools\riscv32-esp-elf\*\riscv32-esp-elf\bin\riscv32-esp-elf-gcc.exe"))[-1]
OBJCOPY = CC.replace("gcc.exe", "objcopy.exe")

# Tres pruebas oficiales suponen cosas que el ESP32-C3 (real y modelo) no permite:
#   fence_i  salta a la SRAM por su vista de DATOS; en el C3 se ejecuta solo por la vista de
#            instrucciones (0x40380000)  -> se corre adaptados/fence_i_c3.S
#   rvc      escribe en datos guardados dentro del codigo, que aqui vive en la FLASH (solo lectura)
#            -> se corre adaptados/rvc_c3.S
#   ma_data  accesos desalineados: el C3 no los hace en hardware (da excepcion, como el chip real)
# control/ tiene una prueba que DEBE fallar: demuestra que el arnes detecta errores.
GRUPOS = {"rv32ui": ["ma_data", "fence_i"], "rv32um": [], "rv32uc": ["rvc"], "adaptados": [], "control": []}


def compilar(grupo, nombre, SALIDA):
    src = (os.path.join(ENV, grupo, nombre + ".S") if grupo in ("adaptados", "control")
           else os.path.join(TESTS, "isa", grupo, nombre + ".S"))
    elf = os.path.join(SALIDA, "%s-%s.elf" % (grupo, nombre))
    binf = elf[:-4] + ".bin"
    hexf = elf[:-4] + ".hex"
    march = "rv32imc_zicsr_zifencei" if grupo in ("rv32uc", "adaptados") else "rv32im_zicsr_zifencei"
    cmd = [CC, "-march=" + march, "-mabi=ilp32", "-nostdlib", "-nostartfiles", "-static",
           "-I" + ENV, "-I" + os.path.join(TESTS, "isa", "macros", "scalar"),
           "-T", os.path.join(ENV, "link.ld"), os.path.join(ENV, "entry.S"), src, "-o", elf]
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode:
        return None, r.stderr
    subprocess.run([OBJCOPY, "-O", "binary", elf, binf], check=True)
    data = open(binf, "rb").read()
    data += b"\0" * (-len(data) % 4)
    w = struct.unpack("<%dI" % (len(data) // 4), data)
    with open(hexf, "w") as f:
        f.write("v2.0 raw\n")
        for k in range(0, len(w), 8):
            f.write(" ".join("%x" % x for x in w[k:k + 8]) + "\n")
    return hexf, None


def correr(circ, hexf, limite):
    t0 = time.time()
    p = subprocess.Popen([LOGISIM, circ, "--tty", "tty", "--toplevel-circuit", "PlacaDevKit",
                          "--load", hexf, "FLASH"], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    q = queue.Queue()
    threading.Thread(target=lambda: [q.put(l) for l in iter(p.stdout.readline, b"")] or q.put(None),
                     daemon=True).start()
    texto, res = "", "TIMEOUT"
    while time.time() - t0 < limite:
        try:
            l = q.get(timeout=1)
        except queue.Empty:
            continue
        if l is None:
            break
        l = l.decode("latin-1")
        texto += l
        s = l.strip()
        if s == "PASS":
            res = "PASS"; break
        if s.startswith("F ") or s.startswith("T "):
            res = "FAIL " + s; break
    p.kill()
    return res, time.time() - t0, texto


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--circ", default=os.path.join(AQUI, "..", "ESP32C3_Logisim.circ"))
    ap.add_argument("--solo")
    ap.add_argument("-j", type=int, default=4)
    ap.add_argument("--limite", type=int, default=600)
    a = ap.parse_args()
    SALIDA = os.path.join(AQUI, "build_tests", os.path.splitext(os.path.basename(a.circ))[0])
    os.makedirs(SALIDA, exist_ok=True)
    trabajos = []
    for g, fuera in GRUPOS.items():
        carpeta = os.path.join(ENV, g) if g in ("adaptados", "control") else os.path.join(TESTS, "isa", g)
        for f in sorted(glob.glob(os.path.join(carpeta, "*.S"))):
            n = os.path.splitext(os.path.basename(f))[0]
            if n in fuera or (a.solo and n not in a.solo.split(",")):
                continue
            hexf, err = compilar(g, n, SALIDA)
            if not hexf:
                print("%-22s NO COMPILA\n%s" % (g + "-" + n, err)); continue
            trabajos.append((g + "-" + n, hexf))
    print("Circuito: %s\n%d pruebas\n" % (os.path.basename(a.circ), len(trabajos)), flush=True)
    resultados = {}

    def uno(t):
        res, seg, txt = correr(a.circ, t[1], a.limite)
        if t[0].startswith("control-"):
            # el control negativo tiene que fallar justo en el caso 3
            res = "OK (fallo esperado: %s)" % res if res.startswith("FAIL F 00000003") else "CONTROL ROTO: " + res
        resultados[t[0]] = (res, seg)
        print("%-22s %-28s %5.0f s" % (t[0], res, seg), flush=True)
        if res != "PASS":
            with open(os.path.join(SALIDA, t[0] + ".log"), "w") as f:
                f.write(txt)

    with ThreadPoolExecutor(a.j) as ex:
        list(ex.map(uno, trabajos))
    pruebas = {k: v for k, v in resultados.items() if not k.startswith("control-")}
    ok = sum(1 for r, _ in pruebas.values() if r == "PASS")
    print("\nRESULTADO: %d de %d pasan" % (ok, len(pruebas)))
    for k, (r, _) in resultados.items():
        if k.startswith("control-"):
            print("Control negativo: %s" % r)


if __name__ == "__main__":
    main()
