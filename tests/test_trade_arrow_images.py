"""Exercise the production trade button setup without starting the game.

Only external FRM loading and window registration are replaced. Registered
pixels are checked after setup returns, including all six button states.

Requires Clang with ASan/UBSan and LLVM coverage tools. Set CXX, LLVM_COV,
and LLVM_PROFDATA to executable paths, or provide clang++, llvm-cov, and
llvm-profdata on PATH. Windows additionally needs an initialized C++ SDK
compiler environment and the Clang runtime directory on PATH.
"""
import json
import os
import pathlib
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


def block_at(source, start):
    opening = source.index("{", start)
    depth = 1
    end = opening + 1
    while depth:
        depth += (source[end] == "{") - (source[end] == "}")
        end += 1
    return source[opening + 1:end - 1]


class TradeArrowImagesTest(unittest.TestCase):
    def test_registered_states_preserve_rows_for_localized_and_standard_frms(self):
        source = (ROOT / "src/inventory.cc").read_text(encoding="latin-1")
        up = source.index("// Large arrow up (normal).")
        down = source.index("// Large dialog down button (normal)")
        up = block_at(source, source.rfind("if (inventoryWindowType", 0, up))
        down = block_at(source, source.rfind("if (inventoryWindowType", 0, down))
        down = down[:down.index("// Invisible button representing left character.")] + "}"
        support = ""
        if "static unsigned char gTradeArrowButtonImages" in source:
            start = source.index("static unsigned char gTradeArrowButtonImages")
            support += source[start:source.index(";", start) + 1]
        if "static void inventoryPrepareTradeArrowImages(" in source:
            start = source.index("static void inventoryPrepareTradeArrowImages(")
            opening = source.index("{", start)
            support += source[start:opening] + "{" + block_at(source, start) + "}"
        harness = r'''
#include <array>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <algorithm>
struct FrmImage {
    int width=22, height=23;
    bool locked=true;
    std::vector<unsigned char> data;
    void lock(int) {}
    bool isLocked() const { return locked; }
    unsigned char* getData() { return data.data(); }
    int getWidth() const { return width; }
    int getHeight() const { return height; }
};
FrmImage _inventoryFrmImages[16];
struct Button { int x,y,w,h,key; unsigned char *up,*down,*disabled; };
int attempts=0, failButton=-1, callbacks=0, disables=0;
std::vector<Button> buttons;
int gInventoryWindow=1;
int gTradeLeftUpButton, gTradeRightUpButton, gTradeLeftDownButton, gTradeRightDownButton;
enum { OBJ_TYPE_INTERFACE, KEY_ARROW_UP, KEY_CTRL_ARROW_UP, KEY_ARROW_DOWN,
KEY_CTRL_ARROW_DOWN };
int buildFid(int, int fid, int, int, int) { return fid; }
void _gsound_red_butt_press(int) {}
void _gsound_red_butt_release(int) {}
void buttonSetCallbacks(int, void(*)(int), void(*)(int)) { callbacks++; }
void buttonDisable(int) { disables++; }
int buttonCreate(int, int x,int y,int w,int h,int,int,int key,int,
    unsigned char* up, unsigned char* down, unsigned char*,int) {
    if (attempts++ == failButton) return -1;
    buttons.push_back({x,y,w,h,key,up,down,nullptr}); return (int)buttons.size()-1;
}
void _win_register_button_disable(int i, unsigned char* up, unsigned char* down, unsigned char* hover) {
    if(up!=down || up!=hover) std::abort();
    buttons[i].disabled=up;
}
__SUPPORT__
void setup() { int fid,btn; __UP__ __DOWN__ }
int main() {
    constexpr int indices[6]={2,3,14,5,6,15};
    constexpr int identityX[22]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21};
    constexpr int downX[22]={0,1,2,3,4,5,6,7,8,9,10,12,13,14,15,16,17,18,19,20,21,22};
    constexpr int identityY[23]={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22};
    constexpr int downY[23]={0,1,2,3,4,5,6,7,8,9,10,11,13,14,15,16,17,18,19,20,21,22,23};
    // Every fixture has positive dimensions and width*height valid bytes,
    // matching the precondition of successfully loaded interface FRMs.
    const int sizes[4][6][2]={
        {{22,23},{22,23},{22,23},{22,23},{22,23},{22,23}},
        {{23,24},{23,24},{22,23},{24,25},{24,25},{22,23}},
        {{23,24},{24,25},{23,24},{24,25},{23,24},{24,25}},
        {{1,1},{22,23},{1,1},{1,1},{24,25},{1,1}},
    };
    const int keys[4]={KEY_ARROW_UP,KEY_CTRL_ARROW_UP,KEY_ARROW_DOWN,KEY_CTRL_ARROW_DOWN};
    for (int fixture=0;fixture<4;fixture++) for(int opening=0;opening<2;opening++) {
        buttons.clear(); attempts=callbacks=disables=0;
        for (int state=0;state<6;++state) {
            auto& image=_inventoryFrmImages[indices[state]];
            image.width=sizes[fixture][state][0];
            image.height=sizes[fixture][state][1];
            image.data.resize(image.width*image.height);
            for (int y=0;y<image.height;++y) for(int x=0;x<image.width;++x)
                image.data[y*image.width+x]=(opening*83+state*29+y*7+x)%256;
        }
        setup();
        if(buttons.size()!=4 || callbacks!=4 || disables!=4) return 1;
        if(gTradeLeftUpButton!=0 || gTradeRightUpButton!=1
            ||gTradeLeftDownButton!=2 || gTradeRightDownButton!=3) return 4;
        for (int side=0;side<4;++side) {
            auto& b=buttons[side];
            if(b.x!=(side%2?342:111) || b.y!=(side<2?57:82)
                ||b.w!=22 || b.h!=23 || b.key!=keys[side]) return 2;
            const unsigned char* states[3]={b.up,b.down,b.disabled};
            for(int state=0;state<3;++state) for(int y=0;y<23;++y) for(int x=0;x<22;++x) {
                int slot=(side<2?0:3)+state;
                // Explicit source-coordinate oracles for 1/22/23/24 widths
                // and 1/23/24/25 heights, independent of production arithmetic.
                int width=sizes[fixture][slot][0],height=sizes[fixture][slot][1];
                int sx=width==1?0:width==24?downX[x]:identityX[x];
                int sy=height==1?0:height==25?downY[y]:identityY[y];
                int expected=(opening*83+slot*29+sy*7+sx)%256;
                int actual=states[state][y*22+x];
                if(actual!=expected) {
                    std::fprintf(stderr,"fixture=%d opening=%d side=%d state=%d pixel=(%d,%d): expected %d got %d\n",
                        fixture,opening,side,state,x,y,expected,actual); return 3;
                }
            }
        }
        // The window manager retains its pointers after setup returns.
        // Change FRM storage and verify retained button bytes remain owned.
        unsigned char first=buttons[0].up[0];
        _inventoryFrmImages[2].data[0]^=255;
        if(fixture!=0 && buttons[0].up[0]!=first) return 5;
    }
    // Cover every short-circuit lock outcome and every registration failure.
    for(int index:indices) {
        buttons.clear(); attempts=callbacks=disables=0;
        _inventoryFrmImages[index].locked=false;
        setup();
        if(buttons.size()!=2 || callbacks!=2 || disables!=2) return 6;
        _inventoryFrmImages[index].locked=true;
    }
    for(failButton=0;failButton<4;failButton++) {
        buttons.clear(); attempts=callbacks=disables=0;
        setup();
        if(buttons.size()!=3 || callbacks!=3 || disables!=3) return 7;
    }
    std::puts("PASS: all four trade buttons, six states, standard 22x23 and Russian 23x24/24x25 FRMs");
}
'''.replace("__SUPPORT__", support).replace("__UP__", up).replace("__DOWN__", down)
        with tempfile.TemporaryDirectory(prefix="trade-arrows-") as directory:
            path = pathlib.Path(directory)
            source_file = path / "test.cc"
            executable = path / "test.exe"
            source_file.write_text(harness, encoding="utf-8")
            compiler = os.environ.get("CXX", "clang++")
            command = [compiler, "-std=c++17", "-g", "-fsanitize=address,undefined",
                       "-fno-omit-frame-pointer", "-fno-sanitize-recover=all",
                       "-fprofile-instr-generate", "-fcoverage-mapping",
                       str(source_file), "-o", str(executable)]
            compiled = subprocess.run(command, capture_output=True, text=True)
            self.assertEqual(compiled.returncode, 0, compiled.stdout + compiled.stderr)
            environment = dict(os.environ, LLVM_PROFILE_FILE=str(path / "test.profraw"))
            result = subprocess.run([str(executable)], env=environment, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            print(result.stdout.strip())
            subprocess.run([os.environ.get("LLVM_PROFDATA", "llvm-profdata"),
                            "merge", "-sparse", str(path / "test.profraw"), "-o", str(path / "test.profdata")],
                           check=True, capture_output=True)
            coverage = json.loads(subprocess.check_output([
                os.environ.get("LLVM_COV", "llvm-cov"), "export", str(executable),
                "-instr-profile=" + str(path / "test.profdata")]))
            lcov = subprocess.check_output([
                os.environ.get("LLVM_COV", "llvm-cov"), "export", "-format=lcov", str(executable),
                "-instr-profile=" + str(path / "test.profdata")], text=True)
            line_counts = dict(tuple(map(int, row[3:].split(",")[:2]))
                               for row in lcov.splitlines() if row.startswith("DA:"))
        functions = [fn for fn in coverage["data"][0]["functions"]
                     if "inventoryPrepareTradeArrowImages" in fn["name"] or "setup" in fn["name"]]
        self.assertTrue(functions)
        for function in functions:
            regions = [r for r in function["regions"] if r[7] == 0]
            branches = function["branches"]
            start = min(r[0] for r in regions)
            end = max(r[2] for r in regions)
            lines = [count for line, count in line_counts.items() if start <= line <= end]
            self.assertTrue(lines)
            self.assertTrue(all(count > 0 for count in lines), function)
            self.assertTrue(regions)
            self.assertTrue(branches)
            self.assertTrue(all(r[4] > 0 for r in regions), function)
            self.assertTrue(all(b[4] > 0 and b[5] > 0 for b in branches), function)
            print("COVERAGE %s: lines %d/%d, regions %d/%d, branches %d/%d" % (
                function["name"], len(lines), len(lines), len(regions), len(regions),
                len(branches)*2, len(branches)*2))


if __name__ == "__main__":
    unittest.main()
