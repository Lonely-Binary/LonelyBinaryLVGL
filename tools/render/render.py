#!/usr/bin/env python3
"""
Render LVGL examples on the host, to PNG.

    tools/render/render.py                          every example, default panels
    tools/render/render.py 20_Button                one example
    tools/render/render.py 20_Button -p tft_24      one example, one panel

Output: tools/render/shots/<Example>/<panel>_<shot>.png

LVGL is built once into tools/render/build/. The sketch is compiled against a
stand-in LB_Display (shim/) that keeps the real panel table, so every panel
renders at its true size and rotation. What this checks: layout, text fitting,
colours, what a tap or a drag does. What it cannot: speed, and anything the
real bus or touch controller does - that stays a bench job.
"""
import argparse, os, pathlib, subprocess, sys, concurrent.futures as cf

HERE = pathlib.Path(__file__).resolve().parent
LIB = HERE.parent.parent                      # LonelyBinaryLVGL
FAMILY = LIB.parent
DISPLAY = FAMILY / "LonelyBinaryDisplay" / "src"
GFX = FAMILY / "LonelyBinaryGFX" / "src"
BUILD = HERE / "build"
SHOTS = HERE / "shots"

# A spread of shapes: the touch square, a common portrait, the big one, a
# narrow landscape strip and the smallest.
DEFAULT_PANELS = ["square_392_ctp", "tft_24", "tft_35", "narrow_19", "tft_096"]

CFLAGS = ["-O1", "-w", "-DLV_CONF_INCLUDE_SIMPLE", f"-I{LIB/'src'}"]


def sh(cmd):
    r = subprocess.run(cmd, capture_output=True, text=True)
    if r.returncode:
        sys.stderr.write(" ".join(map(str, cmd))[:300] + "\n" + r.stdout + r.stderr)
        raise SystemExit(1)
    return r


def build_lvgl():
    lib = BUILD / "liblvgl.a"
    srcs = sorted((LIB / "src" / "lvgl").rglob("*.c"))
    conf = LIB / "src" / "lv_conf.h"
    if lib.exists() and lib.stat().st_mtime > conf.stat().st_mtime:
        return lib
    objdir = BUILD / "lvgl"
    objdir.mkdir(parents=True, exist_ok=True)

    def one(src):
        obj = objdir / (str(src.relative_to(LIB / "src")).replace("/", "_") + ".o")
        sh(["clang", "-c", *CFLAGS, str(src), "-o", str(obj)])
        return obj

    print(f"building LVGL ({len(srcs)} files)...", file=sys.stderr)
    with cf.ThreadPoolExecutor(os.cpu_count()) as ex:
        objs = list(ex.map(one, srcs))
    if lib.exists():
        lib.unlink()
    sh(["ar", "rcs", str(lib), *map(str, objs)])
    return lib


def build_example(ex: pathlib.Path, lvgl):
    ino = ex / f"{ex.name}.ino"
    exe = BUILD / "bin" / ex.name
    exe.parent.mkdir(parents=True, exist_ok=True)
    # A sketch's own .c files (images, fonts) are C, as Arduino builds them:
    # C designated initialisers do not compile as C++.
    extra = []
    for src in ex.iterdir():
        if src.suffix == ".c":
            obj = BUILD / "obj" / f"{ex.name}_{src.stem}.o"
            obj.parent.mkdir(parents=True, exist_ok=True)
            sh(["clang", "-c", *CFLAGS, str(src), "-o", str(obj)])
            extra.append(str(obj))
        elif src.suffix == ".cpp":
            extra.append(str(src))
    sh(["clang++", "-std=c++17", *CFLAGS, f"-I{HERE/'shim'}", f"-I{DISPLAY}", f"-I{GFX}", f"-I{ex}",
        f'-DSKETCH="{ino}"', str(HERE / "render.cpp"),
        str(LIB / "src" / "LonelyBinaryLVGL.cpp"), str(LIB / "src" / "LB_Styles.cpp"),
        str(DISPLAY / "LB_Touch.cpp"), *extra, str(lvgl), "-lz", "-o", str(exe)])
    return exe


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("examples", nargs="*")
    ap.add_argument("-p", "--panel", action="append")
    a = ap.parse_args()
    lvgl = build_lvgl()
    exs = sorted(p for p in (LIB / "examples").iterdir() if p.is_dir())
    if a.examples:
        exs = [e for e in exs if any(e.name.startswith(n) for n in a.examples)]
    panels = a.panel or DEFAULT_PANELS

    def job(ex):
        exe = build_example(ex, lvgl)
        out = SHOTS / ex.name
        out.mkdir(parents=True, exist_ok=True)
        for old in out.glob("*.png"):
            if old.name.split("_")[0] in {p.split("_")[0] for p in panels}:
                old.unlink()
        scen = HERE / "scenarios" / f"{ex.name}.txt"
        for p in panels:
            r = subprocess.run([str(exe), p, str(scen) if scen.exists() else "-", str(out / p)],
                               capture_output=True, text=True, timeout=120)
            if r.returncode:
                return f"{ex.name} {p}: FAILED rc={r.returncode}\n{r.stderr[-800:]}"
        return f"{ex.name}: ok"

    with cf.ThreadPoolExecutor(min(8, os.cpu_count())) as pool:
        for msg in pool.map(job, exs):
            print(msg)


if __name__ == "__main__":
    main()
