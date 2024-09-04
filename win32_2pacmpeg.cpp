
#include "windows.h"
#include "shobjidl.h"
#include "shlwapi.h"

#include "stdio.h"

#include "thangz.h"
#include "2pacmpeg.h"
#include "win32_2pacmpeg.h"

#include "2pacmpeg.cpp"

INTERNAL void *
platform_make_heap_buffer(program_memory *target, u64 pool_size) {
    target->memory = VirtualAlloc(0, pool_size, 
                                   MEM_RESERVE | MEM_COMMIT,
                                   PAGE_READWRITE);
    target->write_ptr = target->memory;
    target->capacity = pool_size;

    return target->memory;
}

INTERNAL void
platform_init_threading(platform_thread_info *thread_info) {
    thread_info->cmd_stream_attribs.nLength = sizeof(SECURITY_ATTRIBUTES);
    thread_info->cmd_stream_attribs.bInheritHandle = TRUE;

    thread_info->cmd_stream_processinfo = {0};

    if(CreatePipe(&thread_info->read_handle, 
                &thread_info->write_handle,
                &thread_info->cmd_stream_attribs, 0)) {
        thread_info->cmd_stream_startupinfo.cb = sizeof(STARTUPINFO);
        thread_info->cmd_stream_startupinfo.dwFlags = STARTF_USESTDHANDLES;
        thread_info->cmd_stream_startupinfo.hStdInput = INVALID_HANDLE_VALUE;
        thread_info->cmd_stream_startupinfo.hStdOutput = thread_info->write_handle;
        thread_info->cmd_stream_startupinfo.hStdError = thread_info->write_handle;
    }
#if _2PACMPEG_DEBUG
    else {
        OutputDebugStringA("[error]: CreatePipe failed.\n");
    }
#endif
}

DWORD __stdcall
platform_thread_read_stdout(void *thread_args_voidptr) {
    win32_thread_args *thread_args = (win32_thread_args *)thread_args_voidptr;

    for(;;) {
        DWORD line_buffer_size;
        if(ReadFile(thread_args->_thread_info->read_handle,
                thread_args->_tbuf_group->stdout_line_buffer,
                PMEM_STDOUTLINEBUFFERSIZE, &line_buffer_size, 0)) {
            strcat(thread_args->_tbuf_group->stdout_buffer,
                thread_args->_tbuf_group->stdout_line_buffer);
        } 
        else {
            break;
        }
    }

    return 0;
}

DWORD __stdcall 
platform_thread_wait_for_exit(void *thread_args_voidptr) {
    win32_thread_args *thread_args = (win32_thread_args *)thread_args_voidptr;

    thread_args->_thread_info->stdio_thread_handle = CreateThread(0, 0, 
                                                        platform_thread_read_stdout,
                                                        thread_args_voidptr, 0, 0);

    if(CreateProcessA(0, thread_args->_tbuf_group->command_buffer,
            0, 0, TRUE, CREATE_NO_WINDOW, 0, 0, 
            &thread_args->_thread_info->cmd_stream_startupinfo,
            &thread_args->_thread_info->cmd_stream_processinfo)) {
        thread_args->_rt_vars->ffmpeg_is_running = true;
        log_diagnostic("[info]: FFmpeg started...",
                            last_diagnostic_type::info,
                            thread_args->_tbuf_group);

        WaitForSingleObject(
                thread_args->_thread_info->cmd_stream_processinfo.hProcess, 
                INFINITE);

        thread_args->_rt_vars->ffmpeg_is_running = false;
        log_diagnostic("[info]: FFmpeg finished.",
                            last_diagnostic_type::info,
                            thread_args->_tbuf_group);
    } 
    else {
        log_diagnostic("[fatal error]: FFmpeg failed to start.",
                            last_diagnostic_type::error,
                            thread_args->_tbuf_group);
    }

    CloseHandle(thread_args->_thread_info->cmd_stream_processinfo.hThread);
    CloseHandle(thread_args->_thread_info->cmd_stream_processinfo.hProcess);
    CloseHandle(thread_args->_thread_info->read_handle);
    CloseHandle(thread_args->_thread_info->write_handle);
    CloseHandle(thread_args->_thread_info->stdio_thread_handle);
    CloseHandle(thread_args->_thread_info->wait_thread_handle);

    return 0;
}

INTERNAL bool32
platform_kill_process(platform_thread_info *thread_info) {
    bool32 result = false;
    if(TerminateProcess(thread_info->cmd_stream_processinfo.hProcess, 
            PROCESS_TERMINATE)) {
        result = true;
    }

    return result;
}

INTERNAL void
platform_ffmpeg_execute_command(text_buffer_group *tbuf_group,
                                platform_thread_info *thread_info,
                                runtime_vars *rt_vars) {
#if _2PACMPEG_DEBUG
    memset(tbuf_group->temp_buffer, 0, 
            strlen(tbuf_group->temp_buffer));
    sprintf(tbuf_group->temp_buffer,
            "[info]: tbuf_group->command_buffer:\n%s\n", 
            tbuf_group->command_buffer);
    OutputDebugStringA(tbuf_group->temp_buffer);
#endif
    platform_init_threading(thread_info);

    LOCAL_STATIC win32_thread_args thread_args;
    thread_args._tbuf_group = tbuf_group;
    thread_args._thread_info = thread_info;
    thread_args._rt_vars = rt_vars;

    thread_info->wait_thread_handle = CreateThread(0, 0, 
                                        platform_thread_wait_for_exit, 
                                        (void *)&thread_args, 0, 0);
}

#if 1
INTERNAL wchar_t *
platform_file_input_dialog(wchar_t *output_buffer) {
    HRESULT result = CoInitializeEx(0, COINIT_APARTMENTTHREADED  
                                       | COINIT_DISABLE_OLE1DDE);
    if(SUCCEEDED(result)) {
        IFileOpenDialog *file_dialog;
        result = CoCreateInstance(CLSID_FileOpenDialog, 0, CLSCTX_ALL,
                                IID_IFileOpenDialog, 
                                (void **)&file_dialog);

        if(SUCCEEDED(result) && SUCCEEDED(result = file_dialog->Show(0))) {
            IShellItem *shell_item;
            result = file_dialog->GetResult(&shell_item);

            if(SUCCEEDED(result)) {
                PWSTR file_path;
                result = shell_item->GetDisplayName(SIGDN_FILESYSPATH,
                                                    &file_path);
                if(SUCCEEDED(result)) {
                    wcscpy(output_buffer, file_path);
                    CoTaskMemFree(file_path);
                }
                shell_item->Release();
            }
        }
        file_dialog->Release();
    }
    CoUninitialize();

    return output_buffer;
}
#endif

INTERNAL s8 *
platform_get_working_directory(s8 *destination, DWORD buffer_size) {
    s8 *result = 0;
    DWORD path_length = GetModuleFileNameA(0, destination, buffer_size);

    if(path_length) {
        result = destination;
        for(DWORD char_index = path_length - 1;
                destination[char_index] != '\\';
                --char_index) {
            destination[char_index] = '\0';
        }
    }

    return result;
}

inline bool32
platform_file_exists(s8 *file_path) {
    bool32 result = false;
    if(PathFileExistsA(file_path)) {
        result = true;
    }

    return result;
}

inline bool32
platform_directory_exists(s8 *directory_name) {
    bool32 result = false;
    if(PathIsDirectoryA(directory_name)) {
        result = true;
    }

    return result;
}

INTERNAL bool32
platform_read_file(s8 *file_path, s8 *destination, u64 *dest_size) {
    bool32 result = false;
    HANDLE file_handle = CreateFileA(file_path, GENERIC_READ,
                                   FILE_SHARE_READ, 0, OPEN_EXISTING,
                                   0, 0);

    if(file_handle != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER file_size;
        if(GetFileSizeEx(file_handle, &file_size)) {
            *dest_size = file_size.QuadPart;
            DWORD bytes_read;

            if(ReadFile(file_handle, destination, *dest_size, 
                                    &bytes_read, 0)) {
                result = true;
            }
        } 
    } 
#if _2PACMPEG_DEBUG
    else {
        char __diagnostic[128];
        snprintf(__diagnostic, 128,
                "[file read error]: could not read file. error code: %i", 
                GetLastError());
        OutputDebugStringA(__diagnostic);
    }
#endif
    CloseHandle(file_handle);

    return result;
}

INTERNAL bool32
platform_write_file(s8 *file_path, void *in_buffer, u32 buffer_size) {
    bool32 result = false;
    HANDLE file_handle = CreateFileA(file_path, GENERIC_WRITE,
                                    FILE_SHARE_WRITE, 0, CREATE_ALWAYS, 0, 0);

    if(file_handle != INVALID_HANDLE_VALUE) {
        DWORD bytes_written;
        if(WriteFile(file_handle, in_buffer, buffer_size, 
                                  &bytes_written, 0)) {
            result = true;
        }
    }
#if defined(_2PACMPEG_DEBUG)
    else {
        char err_buf[128];
        sprintf(err_buf, "[error]: writing file failed with code %d.\n", GetLastError());
        OutputDebugStringA(err_buf);
    }
#endif
    CloseHandle(file_handle);

    return result;
}

int __stdcall
WinMain(HINSTANCE instance, HINSTANCE,
        char *cmd_args, int) {
#define SCHEDULER_MS_RESOLUTION 1
    if(timeBeginPeriod(SCHEDULER_MS_RESOLUTION) == TIMERR_NOERROR) {
#if _2PACMPEG_DEBUG
        OutputDebugStringA("[info]: set Windows scheduler granularity to 1 millisecond.\n");
#endif
    }

    if(!glfwInit()) {
        OutputDebugStringA("glfwInit() failed.\n");
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
        OutputDebugStringA("null pointer to GLFW window\n");
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

    if(!strstr(cmd_args, "--use-bitmap-font")) {
        //TODO resolve font path dynamically
        if(platform_file_exists("C:\\Windows\\Fonts\\lucon.ttf")) {
            rt_vars.default_font = ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\lucon.ttf",
                                        13.0f, 0, ImGui::GetIO().Fonts->GetGlyphRangesDefault());
        }
    }

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
        tbuf_group.ffmpeg_path = (s8 *)heapbuf_alloc_region(&p_memory, PMEM_FFMPEGPATHSIZE);

        sprintf(tbuf_group.config_path, 
                "%sPRESETFILE", tbuf_group.working_directory);
        sprintf(tbuf_group.ffmpeg_path, 
                "%sffmpeg\\ffmpeg.exe", tbuf_group.working_directory);
    }

    preset_table p_table = {0};
    p_table.capacity = MAX_PRESETS;
    p_table.name_array = (s8 *)heapbuf_alloc_region(&p_memory, PRESETNAME_PITCH*MAX_PRESETS);
    memset(p_table.name_array, 0, PRESETNAME_PITCH*MAX_PRESETS);
    p_table.command_table = (s8 **)heapbuf_alloc_region(&p_memory, MAX_PRESETS);
    load_startup_files(&tbuf_group, &p_table);

#if _2PACMPEG_RELEASE
    SetClassLongPtr(glfwGetWin32Window(rt_vars.win_ptr),GCLP_HICON,
                    (LONG_PTR)LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(__ICON_ID)));
#endif

#if defined(_2PACMPEG_DEBUG)
    OutputDebugStringA(" -- TRACELOG START -- \n");
    sprintf(tbuf_group.temp_buffer, 
            "memory used:%.2f/%.2f MiB\nworking_directory:%s\nconfig_path:%s\nffmpeg_path:%s\n", 
            ((f32)(((u64)p_memory.write_ptr - (u64)p_memory.memory )) / 1024.0f / 1024.0f), 
            ((f32)p_memory.capacity) / 1024.0f / 1024.0f,
            tbuf_group.working_directory,
            tbuf_group.config_path,
            tbuf_group.ffmpeg_path);

    OutputDebugStringA(tbuf_group.temp_buffer);
    memset(tbuf_group.temp_buffer, 0, strlen(tbuf_group.temp_buffer));
#endif

    while(!glfwWindowShouldClose(rt_vars.win_ptr)) {
        update_window(&tbuf_group, &p_table, 
                        &rt_vars, &thread_info);
        // HYPERBRUH
        Sleep(17);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(rt_vars.win_ptr);
    glfwTerminate();

    if(rt_vars.ffmpeg_is_running) {
        TerminateProcess(thread_info.cmd_stream_processinfo.hProcess, PROCESS_TERMINATE);
    }

    return 0;
}
