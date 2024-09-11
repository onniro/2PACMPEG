
#if !defined(LINUX_2PACMPEG_DOT_H)

struct platform_thread_info { 
    //TODO
    int file_descriptor;
    FILE *read_pipe;
    pthread_t read_thread_handle;
    program_enum prog_enum;
};

struct linux_thread_args {
    text_buffer_group *_tbuf_group;
    platform_thread_info *_thread_info;
    runtime_vars *_rt_vars;
    program_enum *_prog_enum;
};

////////////////////

INTERNAL void * platform_make_heap_buffer(program_memory *target, u64 pool_size);
INTERNAL void platform_init_threading(platform_thread_info *thread_info);
INTERNAL bool32 platform_kill_process(platform_thread_info *thread_info);
INTERNAL void platform_ffmpeg_execute_command(text_buffer_group *tbuf_group, platform_thread_info *thread_info, runtime_vars *rt_vars);
INTERNAL wchar_t * platform_file_input_dialog(wchar_t *output_buffer);
INTERNAL char * platform_get_working_directory(char *destination, uint32_t buffer_size);
inline bool32 platform_file_exists(char *file_path);
inline bool32 platform_directory_exists(char *directory_name);
INTERNAL bool32 platform_read_file(char *file_path, char *destination, u64 *dest_size);
INTERNAL bool32 platform_write_file(char *file_path, void *in_buffer, u64 buffer_size);

#define LINUX_2PACMPEG_DOT_H
#endif
