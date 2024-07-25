
#pragma once

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#define GL_SILENCE_DEPRECATION
#if defined(_2PACMPEG_WIN32)
    #define GLFW_EXPOSE_NATIVE_WIN32
#endif

#include "GLFW\glfw3.h"
#include "GLFW\glfw3native.h"

#include "thangz.h"

#define PMEMORY_AMT MEGABYTES(2)

// lol
#define PMEM_STDOUTBUFFERSIZE MEGABYTES((u64)1)
#define PMEM_STDOUTLINEBUFFERSIZE KILOBYTES((u64)500)
#define PMEM_CONFIGBUFFERSIZE KILOBYTES((u64)50)
#define PMEM_TEMPBUFFERSIZE KILOBYTES((u64)50)
#define PMEM_COMMANDBUFFERSIZE KILOBYTES((u64)10)
#define PMEM_USRCOMMANDBUFFERSIZE KILOBYTES((u64)8)
#define PMEM_INPUTPATHBUFFERSIZE KILOBYTES((u64)2)
#define PMEM_WCHAR_INPUTBUFSIZE KILOBYTES((u64)2) // actually 4k lol
#define PMEM_OUTPUTPATHBUFFERSIZE KILOBYTES((u64)2) 
#define PMEM_DIAGNOSTICBUFFERSIZE 512

#define PMEM_WORKINGDIRSIZE KILOBYTES((u64)2)
#define PMEM_CONFIGPATHSIZE KILOBYTES((u64)2)
#define PMEM_FFMPEGPATHSIZE KILOBYTES((u64)2)

// // NOTE: max amount of insertions is 1000
// #define PMEM_PRESETTABLESIZE (sizeof(s8 *) * 200)

#define MAX_PRESETS 1000
#define PRESETNAME_PITCH 64

#define TOKEN_OUTPUTDIR (char)0xAA
#define TOKEN_PRESETNAME (char)0xAC // ¬
#define TOKEN_PRESETCMD (char)0xAF // ¯

struct program_memory {
    void *memory;
    void *write_ptr;
    u64 capacity;
};

struct runtime_vars {
    int win_width;
    int win_height;
    GLFWwindow *win_ptr;
    bool32 ffmpeg_is_running;
};

enum last_diagnostic_type {
    undefined = 0,
    error, 
    info
};

struct text_buffer_group {
    s8 *command_buffer;
    s8 *input_path_buffer;
    s8 *output_path_buffer;
    s8 *default_path_buffer;
    s8 *stdout_buffer;
    s8 *temp_buffer;
    s8 *user_cmd_buffer;
    s8 *stdout_line_buffer;
    s8 *config_buffer;

    wchar_t *wchar_input_buffer;

    s8 *working_directory;
    s8 *config_path;
    s8 *ffmpeg_path;

    s8 *diagnostic_buffer; // NOTE: stack allocated
};

struct preset_table {
    int entry_amount;
    int capacity;
    s8 **command_table;
    s8 *name_array;
};

struct platform_thread_info;

 //////////////////////

// updated 18.7.24@11am (not in the order in which they are defined)

INTERNAL last_diagnostic_type diagnostic_callback(s8 *message, last_diagnostic_type type, text_buffer_group *tbuf_group);
inline void *heapbuf_alloc_region(program_memory *pool, u64 region_size);
INTERNAL bool32 serialize_preset(s8 *preset_name, s8 *preset_command, text_buffer_group *tbuf_group);
inline void insert_preset_name(preset_table *p_table, s8 *preset_name, int preset_name_length, int insert_index);
inline int command_length(s8 *command_begin);
INTERNAL void remove_preset(preset_table *p_table, text_buffer_group *tbuf_group, int rm_index);
inline bool32 check_duplicate_presetname(preset_table *p_table, s8 *p_name);
INTERNAL void basic_controls_update(text_buffer_group *tbuf_group, preset_table *p_table, runtime_vars *rt_vars, platform_thread_info *thread_info);
INTERNAL void preset_list_update(text_buffer_group *tbuf_group, preset_table *p_table, runtime_vars *rt_vars);
INTERNAL void update_window(text_buffer_group *tbuf_group, preset_table *p_table, runtime_vars *rt_vars, platform_thread_info *thread_info);
