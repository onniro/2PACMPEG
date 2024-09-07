
#define _2PACMPEG_LINUX 1
#undef _2PACMPEG_WIN32

#include "stdio.h"
#include "string.h"
#include "sys/mman.h"
#include "sys/stat.h"
#include "stdint.h"
#include "unistd.h"
#include "fcntl.h"
#include "stdlib.h"

#include "thangz.h"
#include "2pacmpeg.h"
#include "linux_2pacmpeg.h" 

#include "2pacmpeg.cpp"

INTERNAL void *
platform_make_heap_buffer(program_memory *target, u64 pool_size) 
{
    //TODO: forreal fix this shit lmao
#define _2PACMPEG_USE_MALLOC 1
#if !_2PACMPEG_USE_MALLOC
    char path_buffer[2048];
    size_t bytes_read = readlink("/proc/self/exe", path_buffer, 2048);
    path_buffer[bytes_read] = 0x0;

    int file_descriptor = open(path_buffer, O_RDONLY);
    if(file_descriptor != -1) {
        target->memory = mmap(0, pool_size, 
                            PROT_READ|PROT_WRITE, MAP_PRIVATE,
                            file_descriptor, 0);
        target->write_ptr = target->memory;
        target->capacity = pool_size;
    }
#else 
    target->memory = malloc(pool_size);
    target->write_ptr = target->memory;
    target->capacity = pool_size;
#endif

    return target->memory;
}

INTERNAL void
platform_init_threading(platform_thread_info *thread_info)
{
    return;
}

INTERNAL bool32
platform_kill_process(platform_thread_info *thread_info)
{
    return true;
}

INTERNAL void //TODO
platform_ffmpeg_execute_command(text_buffer_group *tbuf_group,
                                platform_thread_info *thread_info,
                                runtime_vars *rt_vars) 
{
    return;
}

INTERNAL wchar_t *
platform_file_input_dialog(wchar_t *output_buffer)
{
    //NOTE: no standard way to do this on X11, so wont be implemented for a while
    return output_buffer;
}

INTERNAL char *
platform_get_working_directory(char *destination, uint32_t buffer_size)
{
    //NOTE: untested 
    size_t bytes_read = readlink("/proc/self/exe", destination, buffer_size);
    destination[bytes_read] = 0x0;

    for(int char_index = (int)bytes_read;
            char_index >= 0;
            --char_index) {
        if(destination[char_index] != '/') {
            destination[char_index] = 0x0;
        }
        else {
            break;
        }

    }

    return destination;
}

inline bool32
platform_file_exists(char *file_path)
{
    bool32 result = false;
    struct stat stat_struct;

    if(stat(file_path, &stat_struct)) {
        result = true;
    }

    return result;
}

inline bool32 
platform_directory_exists(char *directory_name) 
{
    bool32 result = false;
    struct stat stat_struct;

    if(stat(directory_name, &stat_struct) == 0 && 
            S_ISDIR(stat_struct.st_mode)) {
        result = true;
    }

    return result;
}

INTERNAL bool32
platform_read_file(char *file_path, 
                char *destination, 
                u64 *dest_size)
{
    return false;
}

INTERNAL bool32
platform_write_file(char *file_path,
                    void *in_buffer,
                    u64 buffer_size)
{
    return false;
}

int
main(int arg_count, char **args)
{
    if(!glfwInit()) {
        fprintf(stderr, "glfwInit() failed.\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    runtime_vars rt_vars = {0};
    rt_vars.win_width = 960;
    rt_vars.win_height = 540;
    rt_vars.ffmpeg_is_running = false;
    rt_vars.win_ptr = glfwCreateWindow(rt_vars.win_width, rt_vars.win_height, 
            "2PACMPEG v2.3 - 2PAC 4 LYFE (Definitive Edition)", 
            0, 0);
    if(!rt_vars.win_ptr) {
        fprintf(stderr, "null pointer to GLFW window\n");
        return -1;
    }
    platform_thread_info thread_info = {0};

    glfwMakeContextCurrent(rt_vars.win_ptr);
    glfwSwapInterval(0); // NOTE: it seems like this call was being ignored before
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::GetIO().ConfigFlags |= ImGuiWindowFlags_NoSavedSettings;
    ImGui::GetIO().IniFilename = 0;

    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(rt_vars.win_ptr, true);
    ImGui_ImplOpenGL3_Init("#version 130");


    //TODO: do something about thsi
#if 0
    if(!strstr(cmd_args, "--use-bitmap-font")) {
        rt_vars.default_font = ImGui::GetIO().Fonts->AddFontFromFileTTF("/usr/share/fonts/truetype/noto/NotoMono-Regular.ttf",
                                    13.0f, 0, ImGui::GetIO().Fonts->GetGlyphRangesDefault());
    }
#endif

    program_memory p_memory = {0};
    platform_make_heap_buffer(&p_memory, PMEMORY_AMT);

    if(!p_memory.memory) {
        return -1;
    }

    text_buffer_group tbuf_group = {0};
    tbuf_group.input_path_buffer =      (s8 *)heapbuf_alloc_region(&p_memory, PMEM_INPUTPATHBUFFERSIZE);
    tbuf_group.command_buffer =         (s8 *)heapbuf_alloc_region(&p_memory, PMEM_COMMANDBUFFERSIZE);
    tbuf_group.temp_buffer =            (s8 *)heapbuf_alloc_region(&p_memory, PMEM_TEMPBUFFERSIZE);
    tbuf_group.user_cmd_buffer =        (s8 *)heapbuf_alloc_region(&p_memory, PMEM_USRCOMMANDBUFFERSIZE);
    tbuf_group.output_path_buffer =     (s8 *)heapbuf_alloc_region(&p_memory, PMEM_OUTPUTPATHBUFFERSIZE);
    tbuf_group.default_path_buffer =    (s8 *)heapbuf_alloc_region(&p_memory, PMEM_OUTPUTPATHBUFFERSIZE); // no this is not an accident (but it is retarded)
    tbuf_group.stdout_buffer =          (s8 *)heapbuf_alloc_region(&p_memory, PMEM_STDOUTBUFFERSIZE);
    tbuf_group.stdout_line_buffer =     (s8 *)heapbuf_alloc_region(&p_memory, PMEM_STDOUTLINEBUFFERSIZE);
    tbuf_group.config_buffer =          (s8 *)heapbuf_alloc_region(&p_memory, PMEM_CONFIGBUFFERSIZE);
    tbuf_group.wchar_input_buffer =     (wchar_t *)heapbuf_alloc_region(&p_memory, PMEM_WCHAR_INPUTBUFSIZE);

    // ?? ok
    if(tbuf_group.default_path_buffer) {
        tbuf_group.default_path_buffer[0] = 0x0;
    }

    s8 __diagnostic_buffer[512];
    tbuf_group.diagnostic_buffer = __diagnostic_buffer;
    if(tbuf_group.diagnostic_buffer) {
        tbuf_group.diagnostic_buffer[0] = 0x0;
    }

    tbuf_group.working_directory =  (s8 *)heapbuf_alloc_region(&p_memory, PMEM_WORKINGDIRSIZE);
    platform_get_working_directory(tbuf_group.working_directory, 1024);

    if(tbuf_group.working_directory) {
        tbuf_group.config_path = (s8 *)heapbuf_alloc_region(&p_memory, PMEM_CONFIGPATHSIZE);
        //tbuf_group.ffmpeg_path = (s8 *)heapbuf_alloc_region(&p_memory, PMEM_FFMPEGPATHSIZE);

        sprintf(tbuf_group.config_path, 
                "%sPRESETFILE", tbuf_group.working_directory);
        //sprintf(tbuf_group.ffmpeg_path, 
        //        "%sffmpeg\\ffmpeg.exe", tbuf_group.working_directory);
    }

    preset_table p_table = {0};
    p_table.capacity = MAX_PRESETS;
    p_table.name_array = (s8 *)heapbuf_alloc_region(&p_memory, PRESETNAME_PITCH*MAX_PRESETS);
    memset(p_table.name_array, 0, PRESETNAME_PITCH*MAX_PRESETS);
    p_table.command_table = (s8 **)heapbuf_alloc_region(&p_memory, MAX_PRESETS);
    load_startup_files(&tbuf_group, &p_table);

#if _2PACMPEG_DEBUG
    printf("-- TRACELOG START --\nmemory used:%.2f/%.2f MiB\nworking_directory:%s\nconfig_path:%s\nffmpeg_path:%s\n", 
            ((f32)(((u64)p_memory.write_ptr - (u64)p_memory.memory )) / 1024.0f / 1024.0f), 
            ((f32)p_memory.capacity) / 1024.0f / 1024.0f,
            tbuf_group.working_directory,
            tbuf_group.config_path,
            tbuf_group.ffmpeg_path);
#endif

    while(!glfwWindowShouldClose(rt_vars.win_ptr)) {
        update_window(&tbuf_group, &p_table, 
                        &rt_vars, &thread_info);
        // HYPERBRUH
        usleep(17);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(rt_vars.win_ptr);
    glfwTerminate();

    if(rt_vars.ffmpeg_is_running) {
        //TODO
    }

    return EXIT_SUCCESS;
}
