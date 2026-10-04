#!/usr/bin/env python3
"""
compilar.py - compila tus programas para el ESP32-C3 del modelo Logisim (Windows, Mac o Linux).

Uso:
    python compilar.py arduino/blink                 sketch de Arduino (carpeta con .ino, o el .ino)
    python compilar.py idf/blink                     programa estilo ESP-IDF (con app_main)
    python compilar.py examples/hola_mundo           programa en C con main() usando sdk.h

Opciones:
    --circ RUTA          circuito base (por defecto busca ESP32C3_Logisim.circ al lado de esta carpeta)
    --sin-circ           no generar el circuito .circ con el programa ya cargado
    --cc RUTA            ruta del compilador (riscv32-esp-elf-gcc o similar)
    --ciclos-por-ms N    camara lenta: 1 ms del programa = N ciclos del modelo (por defecto 1)
    -v                   muestra los comandos

Resultado (en la carpeta del programa, subcarpeta 'logisim'):
    NOMBRE.hex   imagen para la ROM FLASH (clic derecho en FLASH > Load Image...)
    NOMBRE.circ  copia del circuito con el programa ya cargado: abrela y presiona Ctrl+K

El compilador: sirve el que instala ESP-IDF (riscv32-esp-elf-gcc, en ~/.espressif o C:\\Espressif),
el del core de Arduino para ESP32, o cualquier riscv*-elf-gcc.
"""
import os, sys, re, glob, shutil, struct, subprocess, tempfile, argparse, html

AQUI = os.path.dirname(os.path.abspath(__file__))
EXE = ".exe" if os.name == "nt" else ""

# ------------------------------------------------------------------ busqueda del compilador
def candidatos_cc():
    nombres = ["riscv32-esp-elf-gcc", "riscv64-unknown-elf-gcc", "riscv32-unknown-elf-gcc", "riscv-none-elf-gcc",
               "riscv64-elf-gcc", "riscv32-elf-gcc"]
    for n in nombres:
        p = shutil.which(n)
        if p:
            yield p
    casa = os.path.expanduser("~")
    bases = [os.environ.get("IDF_TOOLS_PATH", ""), os.path.join(casa, ".espressif"), "C:\\Espressif", "C:\\Espressif\\tools",
             os.path.join(casa, ".espressif", "tools")]
    pats = []
    for b in bases:
        if b:
            pats += [os.path.join(b, "tools", "riscv32-esp-elf", "*", "riscv32-esp-elf", "bin", "riscv32-esp-elf-gcc" + EXE),
                     os.path.join(b, "riscv32-esp-elf", "*", "riscv32-esp-elf", "bin", "riscv32-esp-elf-gcc" + EXE)]
    # core de Arduino para ESP32 (Arduino IDE 2)
    ard = [os.path.join(os.environ.get("LOCALAPPDATA", ""), "Arduino15"), os.path.join(casa, "Library", "Arduino15"),
           os.path.join(casa, ".arduino15"), os.path.join(casa, "AppData", "Local", "Arduino15")]
    for a in ard:
        pats += [os.path.join(a, "packages", "esp32", "tools", "esp-rv32", "*", "bin", "riscv32-esp-elf-gcc" + EXE),
                 os.path.join(a, "packages", "esp32", "tools", "riscv32-esp-elf-gcc", "*", "bin", "riscv32-esp-elf-gcc" + EXE),
                 os.path.join(a, "packages", "esp32", "tools", "riscv32-esp-elf-gcc", "*", "riscv32-esp-elf", "bin",
                              "riscv32-esp-elf-gcc" + EXE)]
    for p in pats:
        for f in sorted(glob.glob(p), reverse=True):      # la version mas nueva primero
            yield f


def herramienta(cc, nombre):
    """riscv32-esp-elf-gcc -> riscv32-esp-elf-<nombre>"""
    d, b = os.path.split(cc)
    b2 = re.sub(r"gcc(\.exe)?$", nombre + r"\1", b)
    return os.path.join(d, b2)


def funciona(cmd):
    try:
        r = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace")
        return r.returncode == 0, r.stdout + r.stderr
    except OSError as e:
        return False, str(e)


def elegir_march(cc, tmp):
    prueba = os.path.join(tmp, "prueba.c")
    with open(prueba, "w") as f:
        f.write("int f(int a, int b) { return a * b; }\n")
    for m in ["rv32imc_zicsr_zifencei", "rv32imc_zicsr", "rv32imc"]:
        ok, _ = funciona([cc, "-march=" + m, "-mabi=ilp32", "-c", prueba, "-o", os.path.join(tmp, "prueba.o")])
        if ok:
            return m
    return None

# ------------------------------------------------------------------ prototipos para sketches .ino
PALABRAS = {"if", "while", "for", "switch", "return", "else", "do", "sizeof", "catch"}

def limpiar(src):
    """quita comentarios y cadenas (conserva saltos de linea) para analizar la estructura"""
    out, i, n = [], 0, len(src)
    while i < n:
        c = src[i]
        if src.startswith("//", i):
            j = src.find("\n", i)
            j = n if j < 0 else j
            out.append(" " * (j - i)); i = j
        elif src.startswith("/*", i):
            j = src.find("*/", i + 2)
            j = n if j < 0 else j + 2
            out.append(re.sub(r"[^\n]", " ", src[i:j])); i = j
        elif c in "\"'":
            j = i + 1
            while j < n and src[j] != c:
                j += 2 if src[j] == "\\" else 1
            j = min(j + 1, n)
            out.append(c + " " * (j - i - 2) + c if j - i >= 2 else " " * (j - i)); i = j
        else:
            out.append(c); i += 1
    return "".join(out)


def prototipos(src):
    s = limpiar(src)
    # quita directivas del preprocesador
    s = re.sub(r"^[ \t]*#.*$", lambda m: " " * len(m.group(0)), s, flags=re.M)
    protos, inicio, prof, ultimo = [], None, 0, 0
    for i, c in enumerate(s):
        if c == "{":
            if prof == 0:
                cab = s[ultimo:i].strip()
                m = re.match(r"^(.*?)\b([A-Za-z_]\w*)\s*\((.*)\)\s*(const)?$", cab, re.S)
                if m and m.group(1).strip() and m.group(2) not in PALABRAS and "=" not in m.group(1) \
                        and not re.search(r"\b(struct|class|enum|union|namespace)\b", m.group(1)):
                    protos.append(re.sub(r"\s+", " ", cab) + ";")
                    if inicio is None:
                        inicio = len(re.findall(r"\n", src[:ultimo]))
            prof += 1
        elif c == "}":
            prof -= 1
            if prof == 0:
                ultimo = i + 1
        elif c == ";" and prof == 0:
            ultimo = i + 1
    return protos, inicio


def sketch_a_cpp(inos, destino):
    """une los .ino (como el IDE de Arduino), agrega #include <Arduino.h> y los prototipos"""
    partes = []
    for f in inos:
        with open(f, encoding="utf-8", errors="replace") as h:
            partes.append((f, h.read()))
    todo = "\n".join(t for _, t in partes)
    protos, linea = prototipos(todo)
    out = ["#include <Arduino.h>\n"]
    puestos = False
    for f, texto in partes:
        nombre = f.replace("\\", "/")
        lineas = texto.split("\n")
        if not puestos and linea is not None and linea < len(lineas):
            out.append('#line 1 "%s"\n' % nombre)
            out.append("\n".join(lineas[:linea]) + "\n")
            out.append("\n".join(protos) + "\n")
            out.append('#line %d "%s"\n' % (linea + 1, nombre))
            out.append("\n".join(lineas[linea:]) + "\n")
            puestos = True
        else:
            out.append('#line 1 "%s"\n' % nombre)
            out.append(texto + "\n")
    with open(destino, "w", encoding="utf-8") as h:
        h.write("".join(out))

# ------------------------------------------------------------------ formato de Logisim
def palabras(binario):
    with open(binario, "rb") as f:
        data = f.read()
    data += b"\0" * (-len(data) % 4)
    return list(struct.unpack("<%dI" % (len(data) // 4), data))


def hex_logisim(words, destino):
    lineas = ["v2.0 raw"]
    for k in range(0, len(words), 8):
        lineas.append(" ".join("%x" % w for w in words[k:k + 8]))
    with open(destino, "w") as f:
        f.write("\n".join(lineas) + "\n")


def contenido_rom(words, bits):
    toks, i, n = [], 0, len(words)
    while n and words[n - 1] == 0:
        n -= 1
    while i < n:
        j = i
        while j < n and words[j] == words[i]:
            j += 1
        toks += ["%d*%x" % (j - i, words[i])] if j - i >= 4 else ["%x" % words[i]] * (j - i)
        i = j
    out = ["addr/data: %d 32" % bits] + [" ".join(toks[k:k + 8]) for k in range(0, len(toks), 8)]
    return "\n".join(out) + "\n"


def circuito_con_programa(base, words, destino):
    with open(base, encoding="utf-8") as f:
        xml = f.read()
    hecho = [False]

    def reemplazo(m):
        comp = m.group(0)
        if 'val="FLASH"' not in comp:
            return comp
        bits = int(re.search(r'name="addrWidth" val="(\d+)"', comp).group(1))
        if len(words) > (1 << bits):
            raise SystemExit("el programa (%d bytes) no cabe en la FLASH del modelo" % (4 * len(words)))
        nuevo = html.escape(contenido_rom(words, bits), quote=True).replace("\n", "&#10;")
        hecho[0] = True
        return re.sub(r'<a name="contents">.*?</a>', lambda _: '<a name="contents">%s</a>' % nuevo, comp, flags=re.S)
    xml = re.sub(r'<comp [^>]*name="ROM">.*?</comp>', reemplazo, xml, flags=re.S)
    if not hecho[0]:
        raise SystemExit("no encontre la ROM 'FLASH' en %s" % base)
    with open(destino, "w", encoding="utf-8") as f:
        f.write(xml)

# ------------------------------------------------------------------ compilacion
def main():
    ap = argparse.ArgumentParser(description="Compila un programa para el ESP32-C3 del modelo Logisim")
    ap.add_argument("programa")
    ap.add_argument("--circ")
    ap.add_argument("--sin-circ", action="store_true")
    ap.add_argument("--cc")
    ap.add_argument("--ciclos-por-ms", type=int, default=1)
    ap.add_argument("-v", action="store_true")
    a = ap.parse_args()

    prog = os.path.abspath(a.programa)
    if os.path.isdir(prog):
        carpeta = prog
        fuentes = sorted(glob.glob(os.path.join(prog, "*.ino")) + glob.glob(os.path.join(prog, "*.c")) +
                         glob.glob(os.path.join(prog, "*.cpp")) + glob.glob(os.path.join(prog, "*.S")))
        nombre = os.path.basename(prog.rstrip("/\\"))
    elif os.path.isfile(prog):
        carpeta = os.path.dirname(prog)
        fuentes = [prog]
        nombre = os.path.splitext(os.path.basename(prog))[0]
    else:
        sys.exit("no existe: " + a.programa)
    if not fuentes:
        sys.exit("no hay archivos .ino, .c o .cpp en " + prog)
    inos = [f for f in fuentes if f.endswith(".ino")]
    # el .ino que se llama como la carpeta va primero (como en el IDE de Arduino)
    inos.sort(key=lambda f: (os.path.splitext(os.path.basename(f))[0] != nombre, f))
    texto = ""
    for f in fuentes:
        with open(f, encoding="utf-8", errors="replace") as h:
            texto += h.read()
    if inos or re.search(r"\bvoid\s+setup\s*\(", texto) and re.search(r"\bvoid\s+loop\s*\(", texto):
        modo = "arduino"
    elif re.search(r"\bapp_main\s*\(", texto):
        modo = "idf"
    else:
        modo = "c"

    # compilador
    ccs = [a.cc] if a.cc else list(candidatos_cc())
    cc = None
    for c in ccs:
        if c and funciona([c, "--version"])[0]:
            cc = c
            break
    if not cc:
        sys.exit("No encontre un compilador RISC-V.\n"
                 "Instala ESP-IDF (trae riscv32-esp-elf-gcc) o indica la ruta con --cc,\n"
                 "por ejemplo: --cc C:\\Users\\TU_USUARIO\\.espressif\\tools\\riscv32-esp-elf\\esp-13.2.0_20240530"
                 "\\riscv32-esp-elf\\bin\\riscv32-esp-elf-gcc.exe")
    cxx = herramienta(cc, "g++")
    objcopy = herramienta(cc, "objcopy")
    size = herramienta(cc, "size")
    tmp = tempfile.mkdtemp(prefix="c3_")
    march = elegir_march(cc, tmp)
    if not march:
        sys.exit("el compilador %s no acepta -march=rv32imc" % cc)

    salida = os.path.join(carpeta, "logisim")
    obj = os.path.join(salida, "obj")
    os.makedirs(obj, exist_ok=True)
    inc = ["-I" + os.path.join(AQUI, "include", "libc"), "-I" + os.path.join(AQUI, "include")]
    if modo == "idf":
        inc.append("-I" + os.path.join(AQUI, "include", "idf"))
    comunes = ["-march=" + march, "-mabi=ilp32", "-Os", "-g", "-ffreestanding", "-fno-builtin", "-nostdlib",
               "-ffunction-sections", "-fdata-sections", "-Wall", "-Wno-unused-function",
               "-DSIM_CICLOS_POR_MS=%d" % a.ciclos_por_ms, "-DESP_PLATFORM"] + inc
    if modo == "arduino":
        comunes += ["-DARDUINO=10819", "-DARDUINO_ARCH_ESP32", "-DESP32"]
    cflags = comunes + ["-std=gnu11"]
    cxxflags = comunes + ["-std=gnu++17", "-fno-exceptions", "-fno-rtti", "-fno-threadsafe-statics",
                          "-fno-use-cxa-atexit"]
    sdk = [os.path.join(AQUI, "src", f) for f in ("start.S", "sdk.c", "libc_min.c")]
    if modo == "arduino":
        sdk.append(os.path.join(AQUI, "src", "arduino.cpp"))
    elif modo == "idf":
        sdk += [os.path.join(AQUI, "src", "idf.c"), os.path.join(AQUI, "src", "idf_switch.S")]
    propias = [f for f in fuentes if not f.endswith(".ino")]
    if inos:
        sk = os.path.join(obj, nombre + "_sketch.cpp")
        sketch_a_cpp(inos, sk)
        propias.insert(0, sk)

    print("Programa: %s   (modo %s)" % (nombre, {"arduino": "Arduino", "idf": "ESP-IDF", "c": "C + sdk.h"}[modo]))
    print("Compilador: %s  (-march=%s)" % (cc, march))
    objetos = []
    for f in propias + sdk:
        o = os.path.join(obj, os.path.basename(f) + ".o")
        es_cpp = f.endswith(".cpp")
        cmd = [cxx if es_cpp else cc] + (cxxflags if es_cpp else cflags) + ["-c", f, "-o", o]
        if a.v:
            print(" ".join(cmd))
        r = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace")
        if r.stdout or r.stderr:
            print((r.stdout + r.stderr).rstrip())
        if r.returncode != 0:
            sys.exit("\nError compilando %s" % os.path.basename(f))
        objetos.append(o)
    elf = os.path.join(salida, nombre + ".elf")
    enlazador = cxx if modo == "arduino" else cc
    cmd = [enlazador, "-march=" + march, "-mabi=ilp32", "-nostdlib", "-nostartfiles", "-Wl,--gc-sections",
           "-T", os.path.join(AQUI, "ld", "app.ld")] + objetos + ["-lgcc", "-o", elf]
    if a.v:
        print(" ".join(cmd))
    r = subprocess.run(cmd, capture_output=True, text=True, encoding="utf-8", errors="replace")
    if r.stdout or r.stderr:
        print((r.stdout + r.stderr).rstrip())
    if r.returncode != 0:
        sys.exit("\nError al enlazar")
    binario = os.path.join(salida, nombre + ".bin")
    subprocess.run([objcopy, "-O", "binary", elf, binario], check=True)
    w = palabras(binario)
    hexf = os.path.join(salida, nombre + ".hex")
    hex_logisim(w, hexf)
    print("\nListo: %s  (%d bytes de flash)" % (hexf, 4 * len(w)))

    if not a.sin_circ:
        base = a.circ or next((p for p in [os.path.join(AQUI, "..", "ESP32C3_Logisim.circ"),
                                           os.path.join(AQUI, "ESP32C3_Logisim.circ")] if os.path.exists(p)), None)
        if base:
            circ = os.path.join(salida, nombre + ".circ")
            circuito_con_programa(base, w, circ)
            print("Circuito con el programa ya cargado: %s" % circ)
            print("  -> abrelo en Logisim y presiona Ctrl+K")
        else:
            print("(no encontre ESP32C3_Logisim.circ: carga el .hex en la ROM FLASH con clic derecho > Load Image)")
    shutil.rmtree(tmp, ignore_errors=True)


if __name__ == "__main__":
    main()
