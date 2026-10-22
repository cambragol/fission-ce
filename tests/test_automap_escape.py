"""Regression tests against the real automap key handler (no game or data files).

Requires clang++, llvm-profdata and llvm-cov on PATH and a configured native C++
toolchain environment. Uses ASan/UBSan and reports coverage of the key dispatch.
Run: python -m unittest discover -s tests -p test_automap_escape.py -v
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


class AutomapEscapeTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory(prefix="automap-escape-")
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
        modal_start = source.index("            switch (keyCode)", source.index("        while (!done)"))
        modal_end = source.index("            case KEY_UPPERCASE_H:", modal_start)
        modal = source[modal_start:modal_end]
        modal_line = source.count("\n", 0, modal_start) + 1
        cls.modal_lines = (modal_line, modal_line + modal.count("\n"))
        modal = ('\n#line 1 "modal_driver"\nbool modalCloseKey(int keyCode) { bool done = false;\n'
                 + f'#line {modal_line} "{SOURCE.as_posix()}"\n' + modal
                 + '\n#line 1 "modal_driver"\n default: break; } return done; }\n')
        harness = cls.directory / "harness.cc"
        harness.write_text(STUBS + keys + f'\n#line {cls.first_line} "{SOURCE.as_posix()}"\n' + handler + modal
                          + '\n#line 1 "test_driver"\n' + r"""
int main(int argc, char** argv) {
    if (argc != 4) return 2;
    gAutomapWindowOpen = std::atoi(argv[2]);
    wide = std::atoi(argv[3]);
    bool handled = std::atoi(argv[2]) == 2 ? modalCloseKey(std::atoi(argv[1]))
                                        : automapHandleKey(std::atoi(argv[1]));
    std::printf("%d %d %d\n", handled, closes, gAutomapWindowOpen);
}
""", encoding="utf-8")
        compiled = harness.read_text(encoding="utf-8")
        cls.first_line = compiled.count("\n", 0, compiled.index("    if (keyCode == KEY_ESCAPE)")) + 1
        cls.last_line = compiled.count("\n", 0, compiled.index("    bool handled = true;")) + 1
        modal_start = compiled.index("            switch (keyCode)")
        modal_end = compiled.index('\n#line 1 "modal_driver"\n default:')
        cls.modal_lines = (compiled.count("\n", 0, modal_start) + 1,
                           compiled.count("\n", 0, modal_end) + 1)
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
            modal_lines = []
            for line in annotated.splitlines():
                match = re.match(r"\s*(\d+)\|\s*([0-9]+)\|", line)
                if match and cls.first_line <= int(match[1]) < cls.last_line:
                    executable_lines.append(int(match[2]))
                if match and cls.modal_lines[0] <= int(match[1]) < cls.modal_lines[1]:
                    modal_lines.append(int(match[2]))
            if not executable_lines or not all(executable_lines):
                raise AssertionError("Key dispatch must have 100% line coverage")
            print(f"automap key dispatch lines: {len(executable_lines)}/{len(executable_lines)}",
                  flush=True)
            if not modal_lines or not all(modal_lines):
                raise AssertionError("Modal close dispatch must have 100% line coverage")
            print(f"modal close dispatch lines: {len(modal_lines)}/{len(modal_lines)}",
                  flush=True)
            file = next(item for item in data["files"]
                        if Path(item["filename"]).name == "harness.cc")
            modal_segments = [segment for segment in file["segments"]
                              if cls.modal_lines[0] <= segment[0] < cls.modal_lines[1]
                              and segment[3] and segment[4] and not segment[5]]
            if not modal_segments or any(not segment[2] for segment in modal_segments):
                raise AssertionError("Modal close dispatch must have 100% region coverage")
            print("modal close dispatch executable regions: 100%", flush=True)
            modal_branches = [branch for branch in file["branches"]
                              if cls.modal_lines[0] <= branch[0] < cls.modal_lines[1]]
            modal_covered = sum((branch[4] > 0) + (branch[5] > 0)
                                for branch in modal_branches)
            modal_total = 2 * len(modal_branches)
            if not modal_branches or modal_covered != modal_total:
                raise AssertionError("Modal close dispatch must have 100% branch coverage")
            print(f"modal close dispatch branches: {modal_covered}/{modal_total}", flush=True)
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

    def test_escape_closes_and_consumes_open_map(self):
        for wide in (False, True):
            with self.subTest(wide=wide):
                self.assertEqual(self.run_key(27, True, wide), (1, 1, 0))

    def test_escape_passes_when_map_closed(self):
        for wide in (False, True):
            with self.subTest(wide=wide):
                self.assertEqual(self.run_key(27, False, wide), (0, 0, 0))

    def test_tab_still_closes_the_map(self):
        for opened in (False, True):
            for wide in (False, True):
                with self.subTest(opened=opened, wide=wide):
                    handled, closes, remains_open = self.run_key(9, opened, wide)
                    self.assertEqual((closes, remains_open), (int(opened), 0))
                    if not opened:
                        self.assertFalse(handled)

    def test_unrelated_key_passes_without_closing_map(self):
        for opened in (False, True):
            for wide in (False, True):
                with self.subTest(opened=opened, wide=wide):
                    self.assertEqual(self.run_key(32, opened, wide), (0, 0, int(opened)))

    def test_modal_escape_closes(self):
        self.assertEqual(self.run_modal(27), 1)

    def test_modal_tab_still_closes(self):
        self.assertEqual(self.run_modal(9), 1)

    def test_modal_unrelated_key_does_not_close(self):
        self.assertEqual(self.run_modal(32), 0)

    def run_modal(self, key):
        result = subprocess.check_output([str(self.binary), str(key), "2", "0"],
                                         env=self.environment, text=True)
        return int(result.split()[0])


if __name__ == "__main__":
    unittest.main()
