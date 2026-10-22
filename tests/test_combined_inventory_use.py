"""Regression for the production inventory USE dispatch; no game assets required.

Run with Python and Clang/LLVM on PATH, in a compiler developer environment on
Windows. CXX, LLVM_PROFDATA and LLVM_COV may name individual tool executables.
"""
import json
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

HARNESS = r'''
#include <cstdio>
#include <initializer_list>
#include <cstdlib>
#define CHECK(x) do { if (!(x)) { std::fprintf(stderr,"failed: %s at %d\n",#x,__LINE__); std::abort(); } } while (0)
enum { GAME_MOUSE_ACTION_MENU_ITEM_USE, GAME_MOUSE_ACTION_MENU_ITEM_LOOK,
ITEM_TYPE_CONTAINER, ITEM_TYPE_DRUG, ITEM_TYPE_WEAPON, ITEM_TYPE_MISC,
INVENTORY_WINDOW_TYPE_NORMAL, INVENTORY_WINDOW_TYPE_USE_ITEM_ON,
INVENTORY_WINDOW_TYPE_LOOT, INVENTORY_WINDOW_TYPE_TRADE, INVENTORY_WINDOW_CURSOR_ARROW };
enum UseItemResultCode { USE_ITEM_RESULT_REMOVE, USE_ITEM_RESULT_KEEP };
struct Object { int value=42,tile=1,elevation=0; };
Object dude, companion;
Object* gDude=&dude;
Object* _inven_dude=&companion;
Object* _stack[1]={&companion};
Object* _target_pud=nullptr;
Object* _ptable=nullptr;
Object* _btable=nullptr;
int _stack_offset[1]={0},_target_stack_offset[1]={0};
int _curr_stack=0,_target_curr_stack=0,_barter_back_win=0;
bool gUseCombinedInventory=true,acceptDrug=true,canUse=true,removeMisc=true;
int quantity=1,rebuilds=0,hoverReads=0,redraws=0,connections=0,uses=0,useOns=0,containers=0;
Object *liveItem=nullptr,*equipped=nullptr,*cached=nullptr,*lastFocus=nullptr;
void itemRemove(Object*,Object* item,int count) {
    CHECK(item==liveItem && count==1);
    if(quantity>1) { liveItem=new Object(*item); --quantity; }
    else { liveItem=nullptr; quantity=0; }
}
void itemAdd(Object*,Object* item,int count) {
    CHECK(count==1);
    if(liveItem) { delete liveItem; }
    liveItem=item; ++quantity;
}
bool drugItemTakeDrug(Object*,Object*) { return acceptDrug; }
void _obj_connect(Object*,int,int,void*) { ++connections; }
void objectDestroy(Object* item) { delete item; }
void interfaceRenderHitPoints(bool) {}
bool _obj_action_can_use(Object*) { return canUse; }
UseItemResultCode objectUseItemInternal(Object*,Object*) {
    ++uses; return removeMisc?USE_ITEM_RESULT_REMOVE:USE_ITEM_RESULT_KEEP;
}
UseItemResultCode objectUseItemOnInternal(Object*,Object*,Object*) {
    ++useOns; return removeMisc?USE_ITEM_RESULT_REMOVE:USE_ITEM_RESULT_KEEP;
}
void _container_enter(int,int) { ++containers; }
void inventoryBuildCombinedList(Object* focus) {
    ++rebuilds; lastFocus=focus; cached=liveItem;
}
void inventorySetCursor(int) {
    // Match the immediate hover lookup before redraw, reading the cached item.
    CHECK(redraws==0);
    Object* hovered=gUseCombinedInventory?cached:liveItem;
    if(hovered) { volatile int value=hovered->value; CHECK(value==42); }
    CHECK(!gUseCombinedInventory || cached==liveItem);
    ++hoverReads;
}
void inventoryRenderSummary() {}
void _display_target_inventory(int,int,Object*,int) {}
void _display_inventory(int,int,int) { CHECK(hoverReads==1); ++redraws; }
void inventoryWindowRenderInnerInventories(int,Object*,Object*,int) {}
void _adjust_fid() {}
void dispatch(int itemType,int inventoryWindowType,Object* item,Object** itemSlot) {
    Object* owner=&companion;
    int keyCode=0,actionMenuItem=GAME_MOUSE_ACTION_MENU_ITEM_USE;
    switch(actionMenuItem) {
__USE__
    }
__TAIL__
}
int main() {
    const int windows[]={INVENTORY_WINDOW_TYPE_NORMAL,INVENTORY_WINDOW_TYPE_USE_ITEM_ON,
                         INVENTORY_WINDOW_TYPE_LOOT,INVENTORY_WINDOW_TYPE_TRADE};
    int scenarios=0;
    for(int window:windows) for(bool combined:{true,false}) for(int count:{1,3})
    for(int kind:{ITEM_TYPE_DRUG,ITEM_TYPE_MISC,ITEM_TYPE_WEAPON,ITEM_TYPE_CONTAINER})
    for(bool consumed:{true,false}) for(bool direct:{false,true}) for(bool slot:{false,true}) {
        liveItem=new Object; cached=liveItem; equipped=slot?liveItem:nullptr;
        // Equipped objects are not in the combined list or the owner's inventory.
        if(slot) { liveItem=nullptr; cached=nullptr; }
        Object* selected=slot?equipped:liveItem;
        quantity=slot?0:count;
        gUseCombinedInventory=combined; acceptDrug=consumed; removeMisc=consumed; canUse=direct;
        rebuilds=hoverReads=redraws=connections=uses=useOns=containers=0; lastFocus=nullptr;
        dispatch(kind,window,selected,slot?&equipped:nullptr);
        CHECK(hoverReads==1 && redraws==1);
        CHECK(rebuilds==(combined?1:0));
        if(combined) CHECK(lastFocus==((window==INVENTORY_WINDOW_TYPE_NORMAL ||
                                      window==INVENTORY_WINDOW_TYPE_USE_ITEM_ON)?_inven_dude:gDude));
        bool destroys=consumed && kind!=ITEM_TYPE_CONTAINER;
        CHECK(connections==(destroys?1:0));
        if(slot) CHECK((equipped==nullptr)==destroys);
        else CHECK(quantity==count-(destroys?1:0));
        if(kind==ITEM_TYPE_MISC || kind==ITEM_TYPE_WEAPON) {
            CHECK(uses==(direct?1:0)); CHECK(useOns==(direct?0:1));
        }
        CHECK(containers==(kind==ITEM_TYPE_CONTAINER?1:0));
        delete liveItem; delete equipped; liveItem=equipped=cached=nullptr;
        ++scenarios;
    }
    std::printf("PASS: %d scenarios, hover before redraw, all window/focus modes\n",scenarios);
}
'''


def source_section(source, start, end):
    """Preserve production source locations in sanitizer diagnostics."""
    begin = source.index(start)
    finish = source.index(end, begin)
    line = source.count('\n', 0, begin) + 1
    path = (ROOT / 'src/inventory.cc').as_posix()
    return f'#line {line} "{path}"\n' + source[begin:finish] + '\n#line 1 "harness.cc"\n'


class CombinedInventoryUseTest(unittest.TestCase):
    def test_use_refreshes_cache_before_hover_and_redraw(self):
        source = (ROOT / 'src/inventory.cc').read_text(encoding='latin-1')
        use = source_section(source, '    case GAME_MOUSE_ACTION_MENU_ITEM_USE:\n',
                             '    case GAME_MOUSE_ACTION_MENU_ITEM_UNLOAD:')
        tail = source_section(source,
                              '    inventorySetCursor(INVENTORY_WINDOW_CURSOR_ARROW);\n\n    if (inventoryWindowType',
                              '\n}\n\n// 0x473904')
        harness = HARNESS.replace('__USE__', use).replace('__TAIL__', tail)
        with tempfile.TemporaryDirectory(prefix='combined-inventory-test-') as tmp:
            folder = Path(tmp)
            cpp = folder / 'test.cc'
            executable = folder / 'test.exe'
            cpp.write_text(harness, encoding='utf-8')
            compiler = os.environ.get('CXX', 'clang++')
            result = subprocess.run([compiler, '-std=c++17', '-g', '-fsanitize=address,undefined',
                                     '-fno-omit-frame-pointer', '-fno-sanitize-recover=all', '-fprofile-instr-generate',
                                     '-fcoverage-mapping', str(cpp), '-o', str(executable)],
                                    capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            environment = dict(os.environ, LLVM_PROFILE_FILE=str(folder / 'test.profraw'))
            result = subprocess.run([str(executable)], env=environment, capture_output=True, text=True)
            self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
            print(result.stdout.strip())
            profile = folder / 'test.profdata'
            subprocess.run([os.environ.get('LLVM_PROFDATA', 'llvm-profdata'), 'merge', '-sparse',
                            str(folder / 'test.profraw'), '-o', str(profile)], check=True, capture_output=True)
            cov = os.environ.get('LLVM_COV', 'llvm-cov')
            args = [cov, 'export', str(executable), '-instr-profile=' + str(profile)]
            coverage = json.loads(subprocess.check_output(args))
            begin = harness.index('        // Using an item can destroy it,')
            end = harness.index('        break;', begin)
            first, last = harness.count('\n', 0, begin) + 1, harness.count('\n', 0, end) + 1
            function = next(f for f in coverage['data'][0]['functions'] if 'dispatch' in f['name'])
            regions = [r for r in function['regions'] if first <= r[0] < last and r[7] == 0]
            branches = [b for b in function['branches'] if first <= b[0] < last]
            self.assertTrue(regions)
            self.assertTrue(branches)
            self.assertTrue(all(r[4] > 0 for r in regions), regions)
            self.assertTrue(all(b[4] > 0 and b[5] > 0 for b in branches), branches)
            lcov = subprocess.check_output(args + ['-format=lcov'], text=True)
            lines = [int(row.split(',')[1]) for row in lcov.splitlines() if row.startswith('DA:')
                     and first <= int(row[3:].split(',')[0]) < last]
            self.assertTrue(lines)
            self.assertTrue(all(count > 0 for count in lines), lines)
            print(f'PATCH COVERAGE: lines {len(lines)}/{len(lines)}, regions {len(regions)}/{len(regions)}, '
                  f'branch outcomes {len(branches)*2}/{len(branches)*2}')


if __name__ == '__main__':
    unittest.main()
