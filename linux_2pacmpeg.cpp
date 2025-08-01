
#define _2PACMPEG_LINUX 1
#undef _2PACMPEG_WIN32

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <fontconfig/fontconfig.h>
#include <stdlib.h>

#if _2PACMPEG_DEBUG
    #include <errno.h>
#endif

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h" //only for calling SetShortcutRouting() to unbind ctrl+tab

#define GLFW_EXPOSE_NATIVE_X11
#include "GLFW/glfw3.h"
#include "GLFW/glfw3native.h"

#define THANGZ_POSIXAPI_HELPERS 1
#include "thangz.h"

#include "2pacmpeg.h"
#include "linux_2pacmpeg.h" 

#include "2pacmpeg.cpp"
    
INTERNAL void *platform_make_heap_buffer(program_memory *target, u64 pool_size) 
{
    target->memory = mmap(0, pool_size, PROT_READ|PROT_WRITE, 
                        MAP_PRIVATE|MAP_ANONYMOUS, 0, 0);
    target->write_ptr = target->memory;
    target->capacity = pool_size;
    return target->memory;
}

INTERNAL void platform_init_threading(platform_thread_info *thread_info) 
{
    //NOTE: no need to implement this on linux
    return;
}

extern void *platform_thread_read_proc_stdout(void *args_voidptr) 
{
    linux_thread_args *thread_args = (linux_thread_args *)args_voidptr;
    if (!posixapi_get_stdout(thread_args->_tbuf_group->command_buffer,
            &thread_args->_thread_info->file_descriptor,
            &thread_args->_thread_info->proc_id, true)) {
        log_diagnostic("[fatal error]: process failed to start.",
                        last_diagnostic_type::error,
                        thread_args->_tbuf_group);
        pthread_exit(0);
    }

    thread_args->_rt_vars->ffmpeg_is_running = true;
    char *line_buffer = thread_args->_tbuf_group->stdout_line_buffer;
    char *full_buffer = thread_args->_tbuf_group->stdout_buffer;
    char *ffprobe_buffer = thread_args->_tbuf_group->ffprobe_buffer;

    switch (*thread_args->_prog_enum) {
    case program_enum_ffmpeg: {
        log_diagnostic("[info]: FFmpeg started...",
                        last_diagnostic_type::info,
                        thread_args->_tbuf_group);
        ssize_t bytes_read = 0;
        u64 stdout_buffer_bytes = 0;
        while (1) {
            bytes_read = read(thread_args->_thread_info->file_descriptor,
                            line_buffer,
                            PMEM_STDOUTLINEBUFFERSIZE - 1);
            if(! bytes_read) { break; }
            stdout_buffer_bytes += bytes_read;
            if (stdout_buffer_bytes >= STDOUT_BUFFER_RESET_THRESHOLD) {
                full_buffer[0] = 0x0;
                stdout_buffer_bytes = 0;
            }
            strncat(full_buffer, line_buffer,
                    PMEM_STDOUTBUFFERSIZE - stdout_buffer_bytes - 1);
        }

        log_diagnostic("[info]: FFmpeg exited.", last_diagnostic_type::info, thread_args->_tbuf_group);
    } break;

    case program_enum_ffprobe: {
        char temp_buffer[PMEM_DIAGNOSTICBUFFERSIZE];
        ssize_t bytes_read = 0;
        while (1) {
            bytes_read = read(thread_args->_thread_info->file_descriptor,
                            line_buffer,
                            PMEM_STDOUTLINEBUFFERSIZE);
            if (!bytes_read) { break; }
            line_buffer[bytes_read] = 0x0;
            strncat(ffprobe_buffer, line_buffer, PMEM_STDOUTBUFFERSIZE);
        }
    } break;

    case program_enum_ffplay: {
    } break;

    case program_enum_other: {
    } break;

    default: break;
    }

    waitpid(thread_args->_thread_info->proc_id, 0, 0);
    if (fcntl(thread_args->_thread_info->file_descriptor, F_GETFD) != -1) { 
        close(thread_args->_thread_info->file_descriptor); 
    }
    thread_args->_rt_vars->ffmpeg_is_running = false;
    pthread_exit(EXIT_SUCCESS);
}

INTERNAL bool32 platform_kill_process(platform_thread_info *thread_info) 
{
    bool32 result = false;
    //pid + 1 works lmao?
    if (-1 != kill(thread_info->proc_id + 1, SIGKILL)) 
    { result = true; } 
#if _2PACMPEG_DEBUG
    else 
    { printf("errno=%i:%s\n", errno, strerror(errno)); }
#endif
    return result;
}

INTERNAL void platform_ffmpeg_execute_command(text_buffer_group *tbuf_group,
                                            platform_thread_info *thread_info,
                                            runtime_vars *rt_vars,
                                            bool8 detach) 
{
#if _2PACMPEG_DEBUG
    printf("[debug]: attempting to execute:\n%s\n", tbuf_group->command_buffer);
#endif
    LOCAL_STATIC linux_thread_args thread_args;
    thread_args._tbuf_group =   tbuf_group;
    thread_args._thread_info =  thread_info;
    thread_args._rt_vars =      rt_vars;
    thread_args._prog_enum =    &thread_info->prog_enum;
    if (pthread_create(&thread_info->read_thread_handle, 0, 
                        platform_thread_read_proc_stdout,
                        (void *)&thread_args)) {
        log_diagnostic("[error]: spawning thread failed.",
                        last_diagnostic_type::error, tbuf_group);
#if _2PACMPEG_DEBUG
        fprintf(stderr, "[error]: spawning thread failed.");
#endif
        return;
    }

    if (detach) {
        pthread_detach(thread_info->read_thread_handle);
    }
}

INTERNAL wchar_t *platform_file_input_dialog(wchar_t *output_buffer) 
{
    //NOTE: X11 doesn't have a standard way to do this unlike Windows
    //so this won't be implemented for a while
    return output_buffer;
}

INTERNAL char *platform_get_working_directory(char *destination, uint32_t buffer_size) 
{
    size_t bytes_read = readlink("/proc/self/exe", destination, buffer_size);
    destination[bytes_read] = 0x0;
    for (int char_index = (int)bytes_read; char_index >= 0; --char_index) {
        if (destination[char_index] != '/') {
            destination[char_index] = 0x0;
        } else {
            break;
        }
    }
    return destination;
}

inline bool32 platform_file_exists(char *file_path) 
{
    bool32 result = false;
    struct stat stat_struct;
    if (!stat(file_path, &stat_struct)) 
    { result = true; }
    return result;
}

inline bool32 platform_directory_exists(char *directory_name) 
{
    bool32 result = false;
    struct stat stat_struct;
    if (!stat(directory_name, &stat_struct) && 
        S_ISDIR(stat_struct.st_mode)) 
    { result = true; }
    return result;
}

INTERNAL bool32 platform_read_file(char *file_path, char *destination, u64 *dest_size) 
{
    bool32 result = false;
    int file_descriptor = open(file_path, O_RDONLY);
    if (file_descriptor != -1) {
        struct stat stat_buf;
        fstat(file_descriptor, &stat_buf);
        *dest_size = read(file_descriptor, 
                        destination, 
                        stat_buf.st_size);
        result = (*dest_size == stat_buf.st_size);
        close(file_descriptor);
    }
    return result;
}

INTERNAL bool32 platform_write_file(char *file_path, void *in_buffer, u64 buffer_size) 
{
    bool32 result = false;
    int file_descriptor = open(file_path, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
    if (file_descriptor != -1) {
        s64 write_status = write(file_descriptor, in_buffer, buffer_size);  
        result = (write_status == buffer_size);
        close(file_descriptor);
    }
    return result;
}

//FIXME: very hood method of searching for fonts
INTERNAL void platform_load_font(runtime_vars *rt_vars, float font_size) 
{
    char font2load[1024];
#if 0
    font2load[0] = 0;
    if (platform_file_exists("/usr/share/fonts/dejavu/DejaVuSansMono.ttf")) { 
        strncpy(font2load, "/usr/share/fonts/dejavu/DejaVuSansMono.ttf", 1024); 
    } else if (platform_file_exists("/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf")) { 
        strncpy(font2load, "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf", 1024); 
    }
#else
    FcInit();
    FcConfig *fc_cfg = FcInitLoadConfigAndFonts();
    FcPattern *pattern = FcNameParse((const FcChar8 *)"Liberation Mono:Regular");
    FcObjectSet* obj_set = FcObjectSetBuild(FC_FILE, (void *)0);
    FcFontSet *font_set = FcFontList(fc_cfg, pattern, obj_set);
    FcChar8 *file;
    FcPattern *font;
    FcResult res;

    for (int i = 0; i < font_set->nfont; ++i) {
        font = font_set->fonts[i];
        FcPatternGetString(font, FC_FILE, 0, &file);
        if (!strcasestr((char *)file, "italic")) {
            snprintf(font2load, sizeof(font2load), "%s", (char *)file);
#if _2PACMPEG_DEBUG
            printf("loading font %s\n", font2load);
#endif
            break;
        }
    }

    FcFontSetDestroy(font_set);
    FcObjectSetDestroy(obj_set);
    FcPatternDestroy(pattern);
    FcConfigDestroy(fc_cfg);
    FcFini();

    //exit(1);

#endif

    if (*font2load) { 
        imgui_font_load_glyphs(font2load, font_size, rt_vars); 
    }
}

int main(int arg_count, char **args) 
{
    if (process_options_simple(arg_count, args)) 
    { return 0; }
    program_memory p_memory = {0};
    platform_make_heap_buffer(&p_memory, PMEMORY_AMT);

    if (!p_memory.memory) { return -1; }
    cmd_options cmd_opts = {0};
    text_buffer_group tbuf_group = {0};
    tbuf_group.input_path_buffer =      (s8 *)heapbuf_alloc_region(&p_memory, PMEM_INPUTPATHBUFFERSIZE);
    tbuf_group.output_path_buffer =     (s8 *)heapbuf_alloc_region(&p_memory, PMEM_OUTPUTPATHBUFFERSIZE);
    tbuf_group.default_path_buffer =    (s8 *)heapbuf_alloc_region(&p_memory, PMEM_OUTPUTPATHBUFFERSIZE); //no this is not an accident (but it is retarded)
    tbuf_group.wchar_input_buffer =     (wchar_t *)heapbuf_alloc_region(&p_memory, PMEM_WCHAR_INPUTBUFSIZE);
    tbuf_group.command_buffer =         (s8 *)heapbuf_alloc_region(&p_memory, PMEM_COMMANDBUFFERSIZE);
    tbuf_group.user_cmd_buffer =         (s8 *)heapbuf_alloc_region(&p_memory, PMEM_USRCOMMANDBUFFERSIZE);
    tbuf_group.temp_buffer =            (s8 *)heapbuf_alloc_region(&p_memory, PMEM_TEMPBUFFERSIZE);
    tbuf_group.config_buffer =          (s8 *)heapbuf_alloc_region(&p_memory, PMEM_CONFIGBUFFERSIZE);
    tbuf_group.stdout_line_buffer =     (s8 *)heapbuf_alloc_region(&p_memory, PMEM_STDOUTLINEBUFFERSIZE);
    tbuf_group.stdout_buffer =          (s8 *)heapbuf_alloc_region(&p_memory, PMEM_STDOUTBUFFERSIZE);

    s8 _diagnostic_buffer[PMEM_DIAGNOSTICBUFFERSIZE] = {0};
    s8 _ffprobe_buffer[PMEM_DIAGNOSTICBUFFERSIZE] = {0}; 
    tbuf_group.diagnostic_buffer = _diagnostic_buffer;
    tbuf_group.ffprobe_buffer = _ffprobe_buffer;

    preset_table p_table = {0};
    p_table.capacity = MAX_PRESETS;
    p_table.name_array = (s8 *)heapbuf_alloc_region(&p_memory, PRESETNAME_PITCH*MAX_PRESETS);
    memset(p_table.name_array, 0, PRESETNAME_PITCH*MAX_PRESETS);
    p_table.command_table = (s8 **)heapbuf_alloc_region(&p_memory, MAX_PRESETS);

    platform_thread_info thread_info = {0};
    runtime_vars rt_vars = {0};
    rt_vars.win_width = 960;
    rt_vars.win_height = 540;
    rt_vars.ffmpeg_is_running = false;
    rt_vars.tbuf_group_ptr = &tbuf_group;
    rt_vars.p_table_ptr = &p_table;
    rt_vars.thread_info_ptr = &thread_info;
    rt_vars.cmd_opts_ptr = &cmd_opts;

    tbuf_group.working_directory =  (s8 *)heapbuf_alloc_region(&p_memory, PMEM_WORKINGDIRSIZE);
    platform_get_working_directory(tbuf_group.working_directory, 1024);
    if (tbuf_group.working_directory) {
        tbuf_group.config_path = (s8 *)heapbuf_alloc_region(&p_memory, PMEM_CONFIGPATHSIZE);
        sprintf(tbuf_group.config_path, 
                "%sPRESETFILE", tbuf_group.working_directory);
    }
    load_startup_files(&tbuf_group, &p_table);

    if (process_options_complex(arg_count, args, &cmd_opts, &rt_vars)) 
    { return 0; }

    if (!glfwInit()) { 
        fprintf(stderr, "glfwInit failed.\n"); 
        return -1; 
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    char win_title[64];
    get_window_title(win_title);
    rt_vars.win_ptr = glfwCreateWindow(rt_vars.win_width, rt_vars.win_height, win_title, 0, 0);

    if (!rt_vars.win_ptr) {
        fprintf(stderr, "couldn't allocate GLFW window\n");
        return -1;
    }

    glfwMakeContextCurrent(rt_vars.win_ptr);
    glfwSwapInterval(0);
    glfwSetDropCallback(rt_vars.win_ptr, (GLFWdropfun)glfw_drop_callback);

    IMGUI_CHECKVERSION();
    ImGuiContext *imgui_context = ImGui::CreateContext();

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::GetIO().ConfigFlags |= ImGuiWindowFlags_NoSavedSettings;
    ImGui::GetIO().IniFilename = 0;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(rt_vars.win_ptr, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    if (tbuf_group.default_path_buffer) 
    { tbuf_group.default_path_buffer[0] = 0x0; }

    set_text_buffer_group_ptr(&tbuf_group);

    handle_gui_options(&cmd_opts, &rt_vars);

#if _2PACMPEG_DEBUG
    printf("memory used:%.2f/%.2f MiB\nworking_directory:%s\nconfig_path:%s\n", 
            ((f32)(((u64)p_memory.write_ptr - (u64)p_memory.memory )) / 1024.0f / 1024.0f), 
            ((f32)p_memory.capacity) / 1024.0f / 1024.0f,
            tbuf_group.working_directory,
            tbuf_group.config_path);
#endif

    uint64_t start_timestamp, end_timestamp, deltatime;
    useconds_t us2sleep;
    while (!glfwWindowShouldClose(rt_vars.win_ptr)) {
        start_timestamp = posixapi_get_timestamp();
        update_window(&tbuf_group, &p_table, &rt_vars, &thread_info);
        end_timestamp = posixapi_get_timestamp();
        deltatime = end_timestamp - start_timestamp;
        
        if (deltatime < MAX_FRAMETIME_MICROSECONDS) {
            us2sleep = MAX_FRAMETIME_MICROSECONDS - (useconds_t)deltatime;
            usleep(us2sleep); 
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(rt_vars.win_ptr);
    glfwTerminate();

    if (rt_vars.ffmpeg_is_running) 
    { platform_kill_process(&thread_info); }

    return EXIT_SUCCESS;
}
