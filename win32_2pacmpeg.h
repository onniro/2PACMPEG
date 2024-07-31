
#if !defined(WIN32_2PACMPEG_DOT_H)

#if _2PACMPEG_RELEASE
    #define __ICON_ID 101
#endif

struct platform_thread_info {
    SECURITY_ATTRIBUTES cmd_stream_attribs;
    STARTUPINFO cmd_stream_startupinfo;
    PROCESS_INFORMATION cmd_stream_processinfo;

    HANDLE stdio_thread_handle;
    HANDLE wait_thread_handle;
    HANDLE read_handle;
    HANDLE write_handle;
};

struct win32_thread_args {
    text_buffer_group *_tbuf_group;
    platform_thread_info *_thread_info;
    runtime_vars *_rt_vars;
};

 //////////////////////

INTERNAL void * platform_make_heap_buffer(program_memory *target, u64 pool_size);
INTERNAL void platform_init_threading(platform_thread_info *thread_info);
DWORD __stdcall platform_thread_read_stdout(void *thread_args_voidptr);
DWORD __stdcall platform_thread_wait_for_exit(void *thread_args_voidptr);
INTERNAL bool32 platform_kill_process(platform_thread_info *thread_info);
INTERNAL void platform_ffmpeg_execute_command(text_buffer_group *tbuf_group, platform_thread_info *thread_info, runtime_vars *rt_vars);
INTERNAL wchar_t * platform_file_input_dialog(wchar_t *output_buffer);
INTERNAL s8 * platform_get_working_directory(s8 *destination, DWORD buffer_size);
inline bool32 platform_file_exists(s8 *file_path);
inline bool32 platform_directory_exists(s8 *directory_name);
INTERNAL bool32 platform_read_file(s8 *file_path, s8 *destination, u64 *dest_size);
INTERNAL bool32 platform_write_file(s8 *file_path, void *in_buffer, u32 buffer_size);

#define WIN32_2PACMPEG_DOT_H
#endif
