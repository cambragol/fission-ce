#include "sfall_global_scripts.h"

#include <string.h>
#include <stdlib.h>

#include "db.h"
#include "input.h"
#include "memory.h"
#include "platform_compat.h"
#include "scripts.h"
#include "settings.h"
#include "mod_config.h"

namespace fallout {

#define DIR_SEPARATOR '/'

typedef struct GlobalScript {
    Program* program;
    int procs[SCRIPT_PROC_COUNT];
    int repeat;
    int count;
    int mode;
    bool once;
} GlobalScript;

typedef struct GlobalScriptsState {
    char** paths;
    int pathsLength;
    GlobalScript* scripts;
    int scriptsLength;
    int scriptsCapacity;
} GlobalScriptsState;

static GlobalScriptsState* g_state = NULL;

// Helper functions
static int compare_strings(const void* a, const void* b)
{
    const char* sa = *(const char**)a;
    const char* sb = *(const char**)b;
    return strcmp(sa, sb);
}

static void global_scripts_free_scripts(void)
{
    if (g_state == NULL) return;

    for (int i = 0; i < g_state->scriptsLength; i++) {
        programFree(g_state->scripts[i].program);
    }

    if (g_state->scripts != NULL) {
        internal_free(g_state->scripts);
        g_state->scripts = NULL;
    }
    g_state->scriptsLength = 0;
    g_state->scriptsCapacity = 0;
}

static void global_scripts_free_paths(void)
{
    if (g_state == NULL) return;

    for (int i = 0; i < g_state->pathsLength; i++) {
        if (g_state->paths[i] != NULL) {
            internal_free(g_state->paths[i]);
        }
    }

    if (g_state->paths != NULL) {
        internal_free(g_state->paths);
        g_state->paths = NULL;
    }
    g_state->pathsLength = 0;
}

static GlobalScript* global_scripts_find_by_program(Program* program)
{
    if (g_state == NULL) return NULL;

    for (int i = 0; i < g_state->scriptsLength; i++) {
        if (g_state->scripts[i].program == program) {
            return &g_state->scripts[i];
        }
    }
    return NULL;
}

static void global_scripts_process_simple(int mode1, int mode2)
{
    if (g_state == NULL) return;

    for (int i = 0; i < g_state->scriptsLength; i++) {
        GlobalScript* scr = &g_state->scripts[i];
        if (scr->repeat != 0 && (scr->mode == mode1 || scr->mode == mode2)) {
            scr->count++;
            if (scr->count >= scr->repeat) {
                programExecuteProcedure(scr->program, scr->procs[SCRIPT_PROC_START]);
                scr->count = 0;
            }
        }
    }
}

// Public API
bool sfall_gl_scr_init()
{
    if (g_state != NULL) {
        return true; // already initialized
    }

    g_state = (GlobalScriptsState*)internal_malloc(sizeof(GlobalScriptsState));
    if (g_state == NULL) {
        return false;
    }

    memset(g_state, 0, sizeof(GlobalScriptsState));

    char pattern[COMPAT_MAX_PATH];
    snprintf(pattern, sizeof(pattern), "scripts%cgl*.int", DIR_SEPARATOR);

    char dirPrefix[COMPAT_MAX_PATH] = { 0 };
    const char* lastSep = strrchr(pattern, DIR_SEPARATOR);
    if (lastSep) {
        size_t len = lastSep - pattern + 1;
        strncpy(dirPrefix, pattern, len);
        dirPrefix[len] = '\0';
    }

    char** files = NULL;
    int filesLength = fileNameListInit(pattern, &files);
    if (filesLength > 0) {
        g_state->paths = (char**)internal_malloc(sizeof(char*) * filesLength);
        if (g_state->paths == NULL) {
            internal_free(g_state);
            g_state = NULL;
            return false;
        }

        for (int i = 0; i < filesLength; i++) {
            char fullPath[COMPAT_MAX_PATH];
            snprintf(fullPath, sizeof(fullPath), "%s%s", dirPrefix, files[i]);
            g_state->paths[i] = internal_strdup(fullPath);
            if (g_state->paths[i] == NULL) {
                // cleanup partially allocated paths
                for (int j = 0; j < i; j++) {
                    internal_free(g_state->paths[j]);
                }
                internal_free(g_state->paths);
                internal_free(g_state);
                g_state = NULL;
                fileNameListFree(&files, 0);
                return false;
            }
            g_state->pathsLength++;
        }

        fileNameListFree(&files, 0);
    } else {
        fileNameListFree(&files, 0);
    }

    // Sort paths alphabetically (like original std::sort)
    if (g_state->pathsLength > 1) {
        qsort(g_state->paths, g_state->pathsLength, sizeof(char*), compare_strings);
    }

    return true;
}

void sfall_gl_scr_reset()
{
    if (g_state != NULL) {
        sfall_gl_scr_remove_all();
    }
}

void sfall_gl_scr_exit()
{
    if (g_state != NULL) {
        sfall_gl_scr_remove_all();

        global_scripts_free_paths();

        internal_free(g_state);
        g_state = NULL;
    }
}

void sfall_gl_scr_exec_start_proc()
{
    if (g_state == NULL) return;

    int capacity = g_state->pathsLength;

    if (capacity > 0) {
        g_state->scripts = (GlobalScript*)internal_malloc(sizeof(GlobalScript) * capacity);
        if (g_state->scripts == NULL) {
            tickersAdd(sfall_gl_scr_process_input);
            return;
        }
        g_state->scriptsCapacity = capacity;
        g_state->scriptsLength = 0;

        for (int i = 0; i < g_state->pathsLength; i++) {
            Program* program = programCreateByPath(g_state->paths[i]);
            if (program == NULL) {
                continue;
            }

            GlobalScript* scr = &g_state->scripts[g_state->scriptsLength];
            scr->program = program;
            for (int action = 0; action < SCRIPT_PROC_COUNT; action++) {
                scr->procs[action] = programFindProcedure(program, gScriptProcNames[action]);
            }
            scr->repeat = 0;
            scr->count = 0;
            scr->mode = 0;
            scr->once = true;

            g_state->scriptsLength++;

            programInterpret(program, -1);
        }
    }

    // ALWAYS register the ticker, even if no scripts were found.
    tickersAdd(sfall_gl_scr_process_input);
}

void sfall_gl_scr_remove_all()
{
    if (g_state == NULL) return;

    tickersRemove(sfall_gl_scr_process_input);

    global_scripts_free_scripts();
}

void sfall_gl_scr_exec_map_update_scripts(int action)
{
    if (g_state == NULL) return;

    for (int i = 0; i < g_state->scriptsLength; i++) {
        GlobalScript* scr = &g_state->scripts[i];
        if (scr->mode == 0 || scr->mode == 3) {
            if (scr->procs[action] != -1) {
                programExecuteProcedure(scr->program, scr->procs[action]);
            }
        }
    }
}

void sfall_gl_scr_process_main()
{
    global_scripts_process_simple(0, 3);
}

void sfall_gl_scr_process_input()
{
    global_scripts_process_simple(1, 1);
}

void sfall_gl_scr_process_worldmap()
{
    global_scripts_process_simple(2, 3);
}

void sfall_gl_scr_set_repeat(Program* program, int frames)
{
    GlobalScript* scr = global_scripts_find_by_program(program);
    if (scr != NULL) {
        scr->repeat = frames;
    }
}

void sfall_gl_scr_set_type(Program* program, int type)
{
    if (type < 0 || type > 3) {
        return;
    }

    GlobalScript* scr = global_scripts_find_by_program(program);
    if (scr != NULL) {
        scr->mode = type;
    }
}

bool sfall_gl_scr_is_loaded(Program* program)
{
    GlobalScript* scr = global_scripts_find_by_program(program);
    if (scr != NULL) {
        if (scr->once) {
            scr->once = false;
            return true;
        }
        return false;
    }

    // Not a global script.
    return true;
}

void sfall_gl_scr_update(int burstSize)
{
    if (g_state == NULL) return;

    for (int i = 0; i < g_state->scriptsLength; i++) {
        programInterpret(g_state->scripts[i].program, burstSize);
    }
}

} // namespace fallout