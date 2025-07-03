
#if !defined(_2PACMPEG_DOT_H)

#define _2PACMPEG_VERSION_MAJOR (2)
#define _2PACMPEG_VERSION_MINOR (6)
#define _2PACMPEG_VERSION_PATCH (3)

#define PMEMORY_AMT MEGABYTES(7)

#define PMEM_STDOUTBUFFERSIZE MEGABYTES((u64)5)
#define PMEM_STDOUTLINEBUFFERSIZE KILOBYTES((u64)500)
#define PMEM_CONFIGBUFFERSIZE KILOBYTES((u64)50)
#define PMEM_TEMPBUFFERSIZE KILOBYTES((u64)50)
#define PMEM_COMMANDBUFFERSIZE KILOBYTES((u64)10)
#define PMEM_USRCOMMANDBUFFERSIZE KILOBYTES((u64)8)
#define PMEM_INPUTPATHBUFFERSIZE KILOBYTES((u64)2)
#define PMEM_WCHAR_INPUTBUFSIZE KILOBYTES((u64)2)
#define PMEM_OUTPUTPATHBUFFERSIZE KILOBYTES((u64)2) 
#define PMEM_DIAGNOSTICBUFFERSIZE 512

#define PMEM_WORKINGDIRSIZE KILOBYTES((u64)2)
#define PMEM_CONFIGPATHSIZE KILOBYTES((u64)2)
#define PMEM_FFMPEGPATHSIZE KILOBYTES((u64)2)

#define SMALL_TEXTBUF_SIZE 32
#define AUDIO_TRACK_COUNT_BUFSIZE 16
#define BITRATE_RESULT_BUFSIZE 128

#define MAX_PRESETS 1000
#define PRESETNAME_PITCH 64

#define STDOUT_BUFFER_RESET_THRESHOLD (PMEM_STDOUTBUFFERSIZE - KILOBYTES(5))

#define TOKEN_OUTPUTDIR     (char)0xAA
#define TOKEN_PRESETNAME    (char)0xAC
#define TOKEN_PRESETCMD     (char)0xAF

#if _2PACMPEG_LINUX
    #define MAX_FRAMETIME_MICROSECONDS ((useconds_t)16667)
    #define DEFAULT_FONT_SIZE (16.0f)
#elif _2PACMPEG_WIN32
    #define MAX_FRAMETIME_MILLISECONDS ((DWORD)16)
    #define DEFAULT_FONT_SIZE (13.0f)
#endif

struct program_memory;
struct runtime_vars;
struct text_buffer_group;
struct preset_table;
struct platform_thread_info;
struct cmd_options;

struct program_memory 
{
    void *memory;
    void *write_ptr;
    u64 capacity;
};

struct runtime_vars 
{
    int win_width;
    int win_height;
    GLFWwindow *win_ptr;
    bool32 ffmpeg_is_running;
    ImFont *default_font;
    text_buffer_group *tbuf_group_ptr;
    preset_table *p_table_ptr;
    cmd_options *cmd_opts_ptr;
    platform_thread_info *thread_info_ptr;
};

enum last_diagnostic_type 
{
    undefined = 0,
    error, 
    info
};

enum program_enum 
{
    program_enum_ffmpeg = 0,
    program_enum_ffprobe,
    program_enum_ffplay,
    program_enum_other
};

struct text_buffer_group 
{
    s8 *command_buffer;
    s8 *input_path_buffer;
    s8 *output_path_buffer;
    s8 *default_path_buffer;
    s8 *temp_buffer;
    s8 *user_cmd_buffer;
    s8 *stdout_line_buffer;
    s8 *stdout_buffer;
    s8 *config_buffer;

    wchar_t *wchar_input_buffer;

    s8 *working_directory;
    s8 *config_path;

    s8 *diagnostic_buffer;
    s8 *ffprobe_buffer;
};

struct preset_table
{
    int entry_amount;
    int capacity;
    s8 **command_table; //array of pointers that all point to places in text_buffer_group::config_buffer
    s8 *name_array;
};

struct cmd_options 
{
    bool8 use_bmp_font;
    bool8 output_quiet;
    float font_size;
};

//(forward declarations)

struct platform_thread_info;

INTERNAL char *get_version_string(char *ptr2buf);
INTERNAL void show_version(void);
INTERNAL void show_help(void);
INTERNAL void show_license(void);
INTERNAL bool8 process_options(cmd_options *cmd_opts, int arg_count, char **args);
INTERNAL void handle_gui_options(cmd_options *cmd_opts, runtime_vars *rt_vars);
INTERNAL void get_window_title(char *title);
inline void *heapbuf_alloc_region(program_memory *pool, u64 region_size);
INTERNAL void imgui_font_load_glyphs(char *font2load, float font_size, runtime_vars *rt_vars);
INTERNAL void glfw_drop_callback(GLFWwindow *win_ptr, int path_count, char **path_list);
INTERNAL text_buffer_group *get_text_buffer_group_ptr(text_buffer_group *in_tbuf_group);
INTERNAL last_diagnostic_type log_diagnostic(s8 *message, last_diagnostic_type type, text_buffer_group *tbuf_group);
INTERNAL void show_diagnostic(text_buffer_group *tbuf_group);
INTERNAL void load_startup_files(text_buffer_group *tbuf_group, preset_table *p_table);
inline void adjust_pointer_table(preset_table *p_table, text_buffer_group *tbuf_group, int rm_index, int subtract_from_ceil);
INTERNAL void save_default_output_path(text_buffer_group *tbuf_group, preset_table *p_table);
INTERNAL bool32 serialize_preset(s8 *preset_name, s8 *preset_command, text_buffer_group *tbuf_group);
inline void insert_preset_name(preset_table *p_table, s8 *preset_name, int preset_name_length, int insert_index);
inline int command_length(s8 *command_begin);
INTERNAL void remove_preset(preset_table *p_table, text_buffer_group *tbuf_group, int rm_index);
inline bool32 check_duplicate_presetname(preset_table *p_table, s8 *p_name);
inline void strip_end_filename(s8 *file_path);
INTERNAL void wait_ffprobe_result(text_buffer_group *tbuf_group, runtime_vars *rt_vars, platform_thread_info *thread_info);
INTERNAL void argument_options_calculate_bitrate(text_buffer_group *tbuf_group, runtime_vars *rt_vars, platform_thread_info *thread_info, char *target_filesize_buffer, char *bitrate_buf);
INTERNAL void argument_options_count_audio_tracks(text_buffer_group *tbuf_group, runtime_vars *rt_vars, platform_thread_info *thread_info, char *target_filesize_buffer, char *bitrate_buf);
INTERNAL void argument_options(text_buffer_group *tbuf_group, runtime_vars *rt_vars, platform_thread_info *thread_info, char *target_filesize_buffer, char *bitrate_buf);
INTERNAL void add_args_to_presets(text_buffer_group *tbuf_group, preset_table *p_table, char *preset_name_buffer);
INTERNAL void basic_controls_update(text_buffer_group *tbuf_group, preset_table *p_table, runtime_vars *rt_vars, platform_thread_info *thread_info);
INTERNAL void preset_list_update(text_buffer_group *tbuf_group, preset_table *p_table, runtime_vars *rt_vars);
INTERNAL void update_window(text_buffer_group *tbuf_group, preset_table *p_table, runtime_vars *rt_vars, platform_thread_info *thread_info);

#define _2PACMPEG_DOT_H
#endif
