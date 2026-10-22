"""Regression tests against the real automap key handler (no game or data files).

Requires clang++, llvm-profdata and llvm-cov on PATH and a configured native C++
toolchain environment. Uses ASan/UBSan and reports coverage of the key dispatch.
Run: python -m unittest discover -s tests -p test_automap_tab.py -v
"""
import json
import os
from pathlib import Path
import re
import shutil
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "src" / "automap.cc"


def function(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 1
    end = brace + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[start:end], source.count("\n", 0, start) + 1


STUBS = r"""
#include <cstdio>
#include <cstdlib>
struct Object {
    int pid = 0, tile = 0, elevation = 0;
    struct { struct { struct { int ap = 0; } combat; } critter; } data;
};
struct Rect { int left, top, right, bottom; };
struct MessageListItem {};
struct {
    struct { bool strict_vanilla = false, minimap = true; } enhancements;
    struct { bool running = false; } preferences;
} settings;
bool gAutomapWindowOpen, wide, gAutomapNeedsRefresh, gUseNewAutomapProjection;
bool gPressedPhysicalKeys[2];
int closes, zoom, gAutomapFlags, gAutomapElevation, gElevation, gAutomapWindow;
int gAutomapMapLeft, gAutomapMapRight, gAutomapMapTop, gAutomapMapBottom;
int _combat_free_move, gMiscMessageList, _colorTable[1];
Object dude;
Object* gDude = &dude;
enum { AUTOMAP_WTH_HIGH_DETAILS = 1, AUTOMAP_WITH_SCANNER = 2,
    PROTO_ID_MOTION_SENSOR = 42, MOUSE_EVENT_LEFT_BUTTON_UP = 1,
    SDL_SCANCODE_LSHIFT = 0, SDL_SCANCODE_RSHIFT = 1,
    ANIMATION_REQUEST_RESERVED = 1, COL_ORANGE = 0 };
bool interfaceIsSuperWide() { return wide; }
void automapClose() { ++closes; gAutomapWindowOpen = false; }
Object* critterGetItem1(Object*) { return nullptr; }
Object* critterGetItem2(Object*) { return nullptr; }
const char* getmsg(int*, MessageListItem*, int) { return ""; }
int windowGetRect(int, Rect*) { return -1; }
void mouseGetPosition(int* x, int* y) { *x = 0; *y = 0; }
template<class... T> int automapUpdateButtonStates(T...) { return 0; }
template<class... T> int multidexRefreshMap(T...) { return 0; }
template<class... T> int miscItemGetCharges(T...) { return 0; }
template<class... T> int miscItemConsumeCharge(T...) { return 0; }
template<class... T> int soundPlayFile(T...) { return 0; }
template<class... T> int showDialogBox(T...) { return 0; }
template<class... T> int mouseGetEvent(T...) { return 0; }
template<class... T> int automapScreenToTile(T...) { return -1; }
template<class... T> int reg_anim_clear(T...) { return 0; }
template<class... T> int isInCombat(T...) { return 0; }
template<class... T> int reg_anim_begin(T...) { return 0; }
template<class... T> int animationRegisterRunToTile(T...) { return 0; }
template<class... T> int animationRegisterMoveToTile(T...) { return 0; }
template<class... T> int reg_anim_end(T...) { return 0; }
"""


class AutomapTabTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="automap-tab-")
        cls.directory = Path(cls.temp.name)
        cls.environment = os.environ.copy()
        cls.environment["LLVM_PROFILE_FILE"] = str(cls.directory / "%p.profraw")
        cls.compiler = os.environ.get("CXX") or shutil.which("clang++")
        cls.profdata = os.environ.get("LLVM_PROFDATA") or shutil.which("llvm-profdata")
        cls.cov = os.environ.get("LLVM_COV") or shutil.which("llvm-cov")
        if not all((cls.compiler, cls.profdata, cls.cov)):
            raise RuntimeError("clang++, llvm-profdata and llvm-cov are required")
        source = SOURCE.read_text(encoding="utf-8")
        handler, cls.first_line = function(source, "bool automapHandleKey(int keyCode)")
        cls.last_line = cls.first_line + handler[:handler.index("    switch (keyCode)")].count("\n")
        keys = (ROOT / "src" / "kb.h").read_text(encoding="utf-8")
        keys = re.search(r"typedef enum Key \{.*?\} Key;", keys, re.S).group()
        harness = cls.directory / "harness.cc"
        harness.write_text(STUBS + keys + f'\n#line {cls.first_line} "{SOURCE.as_posix()}"\n' + handler
                          + '\n#line 1 "test_driver"\n' + r"""
int main(int argc, char** argv) {
    if (argc != 4) return 2;
    gAutomapWindowOpen = std::atoi(argv[2]);
    wide = std::atoi(argv[3]);
    bool handled = automapHandleKey(std::atoi(argv[1]));
    std::printf("%d %d %d\n", handled, closes, gAutomapWindowOpen);
}
""", encoding="utf-8")
        compiled = harness.read_text(encoding="utf-8")
        cls.first_line = compiled.count("\n", 0, compiled.index("    if (keyCode == KEY_TAB)")) + 1
        cls.last_line = compiled.count("\n", 0, compiled.index("    switch (keyCode)")) + 1
        cls.binary = cls.directory / ("harness.exe" if os.name == "nt" else "harness")
        subprocess.run([cls.compiler, "-std=c++17", "-O0", "-g",
                        "-fsanitize=address,undefined", "-fno-sanitize-recover=all",
                        "-fprofile-instr-generate", "-fcoverage-mapping",
                        str(harness), "-o", str(cls.binary)], check=True, env=cls.environment)

    @classmethod
    def tearDownClass(cls):
        try:
            profiles = list(cls.directory.glob("*.profraw"))
            merged = cls.directory / "coverage.profdata"
            subprocess.run([cls.profdata, "merge", "-sparse", *map(str, profiles),
                            "-o", str(merged)], check=True)
            report = subprocess.check_output([cls.cov, "export", str(cls.binary),
                                              "-instr-profile=" + str(merged)], text=True)
            data = json.loads(report)["data"][0]
            annotated = subprocess.check_output(
                [cls.cov, "show", str(cls.binary), "-instr-profile=" + str(merged),
                 "-show-line-counts"], text=True)
            executable_lines = []
            for line in annotated.splitlines():
                match = re.match(r"\s*(\d+)\|\s*([0-9]+)\|", line)
                if match and cls.first_line <= int(match[1]) < cls.last_line:
                    executable_lines.append(int(match[2]))
            if not executable_lines or not all(executable_lines):
                raise AssertionError("Key dispatch must have 100% line coverage")
            print(f"automap key dispatch lines: {len(executable_lines)}/{len(executable_lines)}",
                  flush=True)
            file = next(item for item in data["files"]
                        if Path(item["filename"]).name == "harness.cc")
            branches = [branch for branch in file["branches"]
                        if cls.first_line <= branch[0] < cls.last_line]
            covered = sum((branch[4] > 0) + (branch[5] > 0) for branch in branches)
            total = 2 * len(branches)
            print(f"automap key dispatch branches: {covered}/{total}", flush=True)
            if covered != total:
                raise AssertionError("Key dispatch must have 100% branch coverage")
            uncovered = [segment for segment in file["segments"]
                         if cls.first_line <= segment[0] < cls.last_line
                         and segment[3] and segment[4] and not segment[2]
                         and not segment[5]]
            if uncovered:
                raise AssertionError(f"Uncovered dispatch regions: {uncovered}")
            print("automap key dispatch executable regions: 100%", flush=True)
        finally:
            cls.temp.cleanup()

    def run_key(self, key, opened, wide):
        result = subprocess.check_output([str(self.binary), str(key), str(int(opened)),
                                          str(int(wide))], env=self.environment, text=True)
        return tuple(map(int, result.split()))

    def test_tab_closes_and_consumes_open_map(self):
        for wide in (False, True):
            with self.subTest(wide=wide):
                self.assertEqual(self.run_key(9, True, wide), (1, 1, 0))

    def test_tab_passes_when_map_closed(self):
        for wide in (False, True):
            with self.subTest(wide=wide):
                self.assertEqual(self.run_key(9, False, wide), (0, 0, 0))

    def test_unrelated_key_passes_without_closing_map(self):
        for opened in (False, True):
            for wide in (False, True):
                with self.subTest(opened=opened, wide=wide):
                    self.assertEqual(self.run_key(32, opened, wide), (0, 0, int(opened)))


if __name__ == "__main__":
    unittest.main()
