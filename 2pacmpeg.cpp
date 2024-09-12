
/*
FIXME: so.. fvcking.. many.. #ifs... (refactor logging)
TODO: go through and remove spaghetti code
TODO: fix the shit where getting the length doesn't work for MKV format
NOTE: the hardcoding of the commands is a little janky
TODO: update forward declarations
*/

#include "2pacmpeg.h"

inline void *
heapbuf_alloc_region(program_memory *pool, u64 region_size) 
{
    void *result = 0;
    u64 free_memory = ((u64)pool->memory + pool->capacity) -
                           (u64)pool->write_ptr;
    if(region_size <= free_memory) {
        result = pool->write_ptr;
        pool->write_ptr = (void *)((u64)pool->write_ptr + region_size);
    }

    return result;
}

INTERNAL last_diagnostic_type 
log_diagnostic(s8 *message,
                last_diagnostic_type type,
                text_buffer_group *tbuf_group) 
{
    LOCAL_STATIC last_diagnostic_type last_diagnostic = undefined;
    
    if(message && tbuf_group) {
        strncpy(tbuf_group->diagnostic_buffer,
                message,
                PMEM_DIAGNOSTICBUFFERSIZE);
        tbuf_group->diagnostic_buffer[strlen(tbuf_group->diagnostic_buffer)] = 0x0;
    }

    if(type != undefined) {last_diagnostic = type;}
    //last_diagnostic = type;
 
    return last_diagnostic;
}

INTERNAL void 
show_diagnostic(text_buffer_group *tbuf_group) 
{
    if(tbuf_group->diagnostic_buffer[0]) {
        switch(log_diagnostic(0, last_diagnostic_type::undefined, 0)) {
        case error: {
            ImGui::PushStyleColor(ImGuiCol_Text, 
                                IM_COL32(0xff, 0, 0, 0xFF));
        } break;

        case info: {
            ImGui::PushStyleColor(ImGuiCol_Text, 
                                IM_COL32(0, 0xff, 0, 0xFF));
        } break;

        case undefined:
        default: {
            ImGui::PushStyleColor(ImGuiCol_Text, 
                            IM_COL32(0xcc, 0xcc, 0xcc, 0xFF));
        } break;
        }
    }

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
    ImGui::Text(tbuf_group->diagnostic_buffer);

    if(tbuf_group->diagnostic_buffer[0]) {
        ImGui::PopStyleColor();
    }
}

INTERNAL void 
load_startup_files(text_buffer_group *tbuf_group, 
                    preset_table *p_table) 
{
    u64 config_size;

    if(platform_read_file(tbuf_group->config_path, 
            tbuf_group->config_buffer, &config_size)) {
#if _2PACMPEG_DEBUG
    #if _2PACMPEG_WIN32
        OutputDebugStringA("[info]: loaded startup file.\n");
    #else
        printf("[info]: loaded startup file.\n");
    #endif
#endif
        // can cause weird shit if the user is trying to be retarded
        s8 *default_dir_ptr = strchr(tbuf_group->config_buffer,
                                    TOKEN_OUTPUTDIR);
        if(default_dir_ptr && (*(default_dir_ptr + 1) != '\n')) {
            strncpy(tbuf_group->default_path_buffer,
                    default_dir_ptr + 1,
                    command_length(default_dir_ptr + 1));
        }

        s8 *name_ptr = tbuf_group->config_buffer;
        s8 *cmd_ptr = name_ptr;
        while(p_table->entry_amount < p_table->capacity) {
            name_ptr = strchr(name_ptr, TOKEN_PRESETNAME);
            if(name_ptr) {
                cmd_ptr = strchr(++name_ptr, TOKEN_PRESETCMD);
                if(cmd_ptr) {
                    insert_preset_name(p_table, name_ptr, 
                                    cmd_ptr - name_ptr, 
                                    p_table->entry_amount);
                    p_table->command_table[p_table->entry_amount] = ++cmd_ptr;

                    ++p_table->entry_amount;
                } 
                else {
#if _2PACMPEG_DEBUG
    #if _2PACMPEG_WIN32
                    OutputDebugStringA("[exception]: expected preset command token after name token, found 2 names back-to-back.\n");
    #else
                    fprintf(stderr, "[exception]: expected preset command token after name token, found 2 names back-to-back.\n");
    #endif
#endif

                    break;
                }
            } 
            else {
#if _2PACMPEG_DEBUG
    #if _2PACMPEG_WIN32
                OutputDebugStringA("[info]: finished parsing preset file.\n");
    #else
                printf("[info]: finished parsing preset file.\n");
    #endif
#endif
                break;
            }
        }

        s8 _temp_buf[256];
        snprintf(_temp_buf, 256,
                "[info]: found %i presets.",
                p_table->entry_amount);
        log_diagnostic(_temp_buf,
                        last_diagnostic_type::info,
                        tbuf_group);
    } 
    else {
        log_diagnostic("[info]: preset file doesn't exist or couldn't be loaded.",
                        last_diagnostic_type::undefined,
                        tbuf_group);

#if _2PACMPEG_DEBUG
    #if _2PACMPEG_WIN32
        OutputDebugStringA("[info]: error loading startup file.\n");
    #else
        printf("[info]: error loading startup file.\n");
    #endif
#endif
    }
}

//not really sure how slow this can get
inline void 
adjust_pointer_table(preset_table *p_table, 
                    text_buffer_group *tbuf_group, 
                    int rm_index = 0, 
                    int subtract_from_ceil = 0) 
{
    for(int move_index = rm_index;
            move_index < p_table->entry_amount - subtract_from_ceil;
            ++move_index) {
        if(!move_index) {
            p_table->command_table[0] = 
                strchr(tbuf_group->config_buffer, TOKEN_PRESETCMD);

            if(p_table->command_table[0]) {
                p_table->command_table[0] += 1;
            }
        } 
        else {
            p_table->command_table[move_index] = 
                    strchr(p_table->command_table[move_index - 1], 
                            TOKEN_PRESETCMD);

            if(p_table->command_table[move_index]) {
                p_table->command_table[move_index] += 1;
            }
        }
    }
}

INTERNAL void 
save_default_output_path(text_buffer_group *tbuf_group, preset_table *p_table) 
{
    s8 *default_dir_begin;

    if(platform_file_exists(tbuf_group->config_path)) {
        default_dir_begin = strchr(tbuf_group->config_buffer, TOKEN_OUTPUTDIR);

        if(default_dir_begin) {
            int existing_dir_len = command_length(default_dir_begin) + 1;

            strncpy(tbuf_group->temp_buffer, 
                    (s8 *)((u64)tbuf_group->config_buffer + existing_dir_len),
                    PMEM_TEMPBUFFERSIZE);
            tbuf_group->temp_buffer[strlen(tbuf_group->temp_buffer)] = 0x0;

            snprintf(tbuf_group->config_buffer, PMEM_CONFIGBUFFERSIZE,
                    "%c%s\n%s\0",
                    TOKEN_OUTPUTDIR, tbuf_group->default_path_buffer,
                    tbuf_group->temp_buffer);
        }
        else {
            strncpy(tbuf_group->temp_buffer, tbuf_group->config_buffer,
                    PMEM_TEMPBUFFERSIZE);
            tbuf_group->temp_buffer[strlen(tbuf_group->temp_buffer)] = 0x0;

            snprintf(tbuf_group->config_buffer, PMEM_CONFIGBUFFERSIZE,
                    "%c%s\n%s\0",
                    TOKEN_OUTPUTDIR, tbuf_group->default_path_buffer,
                    tbuf_group->temp_buffer);

            adjust_pointer_table(p_table, tbuf_group);
        }
    }
    else {
        snprintf(tbuf_group->config_buffer, PMEM_CONFIGBUFFERSIZE,
                "%c%s\n\0",
                TOKEN_OUTPUTDIR, tbuf_group->default_path_buffer);
    }

    if(platform_write_file(tbuf_group->config_path, 
                        (void *)tbuf_group->config_buffer,
                        strlen(tbuf_group->config_buffer))) {
        log_diagnostic("[info]: configuration updated. (default output folder saved).\n",
                        last_diagnostic_type::info,
                        tbuf_group);
    }
    else {
        log_diagnostic("[file write error]: updating configuration failed.\n",
                        last_diagnostic_type::error,
                        tbuf_group);
    }
}

INTERNAL bool32 
serialize_preset(s8 *preset_name, 
                s8 *preset_command,
                text_buffer_group *tbuf_group) 
{
    memset(tbuf_group->temp_buffer, 0, strlen(tbuf_group->temp_buffer));

    snprintf(tbuf_group->temp_buffer,
            PMEM_TEMPBUFFERSIZE,
            "%c%s%c%s\n\0",
            TOKEN_PRESETNAME, preset_name,
            TOKEN_PRESETCMD, preset_command);
    strncat(tbuf_group->config_buffer, tbuf_group->temp_buffer, 
            PMEM_CONFIGBUFFERSIZE);

    bool32 result = platform_write_file(tbuf_group->config_path, 
                        (void *)tbuf_group->config_buffer,
                        strlen(tbuf_group->config_buffer));
    if(result) {
        log_diagnostic("[info]: configuration updated. (preset added).",
                        last_diagnostic_type::info,
                        tbuf_group);
#if _2PACMPEG_DEBUG
    #if _2PACMPEG_WIN32
        OutputDebugStringA("[info]: configuration updated. (preset added).\n");
    #else
        printf("[info]: configuration updated. (preset added).\n");
    #endif
#endif
    } 
    else {
        //?????????++
#if _2PACMPEG_WIN32
        char __diagnostic[128];
        sprintf(__diagnostic, 
                "[file write error]: could not write preset file %i",
                GetLastError());
        log_diagnostic(__diagnostic,
                        last_diagnostic_type::error,
                        tbuf_group);
#else
        log_diagnostic("[file write error]: could not write preset file.",
                        last_diagnostic_type::error,
                        tbuf_group);
#endif
    }

    return result;
}

//NOTE: preset_name might not be null-terminated
inline void 
insert_preset_name(preset_table *p_table, s8 *preset_name,
                int preset_name_length, int insert_index) 
{
    int insert_offset = PRESETNAME_PITCH*insert_index;

    strncpy(p_table->name_array + insert_offset,
            preset_name, preset_name_length);
}

inline int 
command_length(s8 *command_begin) 
{
    int result = -1;
    s8 *command_end = strchr(command_begin, (s8)'\n');

    if(!command_end) {
        s8 *command_end = strchr(command_begin, (s8)'\0');
    }
    if(command_end) {
        result = command_end - command_begin;
    }

    return result;
}

INTERNAL void 
remove_preset(preset_table *p_table, 
            text_buffer_group *tbuf_group, 
            int rm_index) 
{
    //CLEANUP
    s8 *whole_preset = (p_table->command_table[rm_index] - (strlen(p_table->name_array + (rm_index * PRESETNAME_PITCH)))) - 2;
    u32 preset_length = command_length(whole_preset) + 1; // +1 for \n

    //NOTE: this shit will never happen and i dont know why i wrote it but might as well leave it in as an easter egg lmao
    if(preset_length == -1) {
        log_diagnostic("[config error]: something weird happened.",
                        last_diagnostic_type::error,
                        tbuf_group);
#if _2PACMPEG_WIN32
        MessageBoxA(0, 
                    "Failed to retrieve the length of the command.\n(You can continue using the program normally, but the preset could not be deleted)",
                    "CONFIG ERROR",
                    MB_OK|MB_ICONERROR);
#endif

        return;
    }

    if(rm_index != p_table->entry_amount - 1) {
        u32 cmdbuf_end_bytes = strlen(p_table->command_table[rm_index + 1]) + 
                                    strlen(p_table->name_array + ((rm_index + 1) * PRESETNAME_PITCH)) + 2;
        memset(whole_preset, 0, preset_length);
        memcpy(whole_preset, whole_preset + preset_length, cmdbuf_end_bytes);
        memset(whole_preset + cmdbuf_end_bytes, 0, preset_length);

        void *namearr_shift_location = p_table->name_array + (rm_index * PRESETNAME_PITCH);
        void *namearr_last_elem_ptr = p_table->name_array + ((p_table->entry_amount - 1) * PRESETNAME_PITCH);
        u32 namearr_end_bytes = (u32)((u64)namearr_last_elem_ptr - (u64)namearr_shift_location);

        memset(namearr_shift_location, 0, strlen((char *)namearr_shift_location));
        memcpy(namearr_shift_location, 
                (void *)((u64)namearr_shift_location + PRESETNAME_PITCH),
                namearr_end_bytes);
        memset((void *)((u64)namearr_last_elem_ptr), 0, namearr_end_bytes);

        adjust_pointer_table(p_table, tbuf_group, rm_index, 1);

        p_table->command_table[p_table->entry_amount - 1] = 0;
    } 
    else {
        memset(whole_preset, 0, preset_length);

        s8 *rm_preset_name = p_table->name_array + (rm_index*PRESETNAME_PITCH);
        memset(rm_preset_name, 0, strlen(rm_preset_name));
        p_table->command_table[rm_index] = 0;
    }

    --p_table->entry_amount;

    if(platform_write_file(tbuf_group->config_path,
                        (void *)tbuf_group->config_buffer,
                        strlen(tbuf_group->config_buffer))) {
        log_diagnostic("[info]: configuration updated. (preset deleted)",
                        last_diagnostic_type::info,
                        tbuf_group);
    } 
    else {
#if _2PACMPEG_WIN32
        char __diagnostic[512];
        snprintf(__diagnostic, 512,
                "[error]: updating configuration failed with code %i",
                GetLastError());
        log_diagnostic(__diagnostic,
                        last_diagnostic_type::error,
                        tbuf_group);
#else
        strcpy("[error]: updating configuration failed",
                tbuf_group->diagnostic_buffer);
        tbuf_group->diagnostic_buffer[strlen(tbuf_group->diagnostic_buffer)] = 0x0;
        log_diagnostic(tbuf_group->diagnostic_buffer,
                        last_diagnostic_type::error,
                        tbuf_group);
#endif
    }
}

// NOTE: a little slow i think
inline bool32 
check_duplicate_presetname(preset_table *p_table, s8 *p_name) 
{
    for(int name_index = 0;
            name_index < p_table->entry_amount;
            ++name_index) {
        if(strcmp(p_name, p_table->name_array + (name_index * 
                                PRESETNAME_PITCH)) == 0) {
            return true;
        }
    }

    return false;
}

inline void 
strip_end_filename(s8 *file_path) 
{
    int length = strlen(file_path);

    for(int char_index = length - 1;
            char_index >= 0;
            --char_index) {
#if _2PACMPEG_WIN32
        if(file_path[char_index] == '\\') { 
#else
        if(file_path[char_index] == '/') {
#endif
            break;
        }

        file_path[char_index] = 0;
    }
}

INTERNAL void 
argument_options_calculate_bitrate(text_buffer_group *tbuf_group,
                                runtime_vars *rt_vars,
                                platform_thread_info *thread_info,
                                char *target_filesize_buffer,
                                char *bitrate_buf) 
{
#define BITRATE_RESULT_BUFSIZE 128
    LOCAL_STATIC char bitrate_result_buffer[BITRATE_RESULT_BUFSIZE] = {0};

    ImGui::PushItemWidth(48.0f);

    ImGui::Text("target file size (MB):");
    ImGui::SameLine();
    ImGui::InputText("##target_filesize", target_filesize_buffer, SMALL_TEXTBUF_SIZE);

    ImGui::SameLine();
    if(ImGui::Button("calculate bitrate##calculate_bitrate")) {
        if(!rt_vars->ffmpeg_is_running) {
            if(tbuf_group->input_path_buffer[0]) {
                if(target_filesize_buffer[0]) {
                    memset(tbuf_group->command_buffer, 0, strlen(tbuf_group->command_buffer));
                    memset(tbuf_group->ffprobe_buffer, 0, strlen(tbuf_group->ffprobe_buffer));
                    memset(bitrate_buf, 0, strlen(bitrate_buf));

                    snprintf(tbuf_group->command_buffer, PMEM_COMMANDBUFFERSIZE,
#if _2PACMPEG_WIN32
                            "%sffmpeg\\ffprobe -v error -select_streams v:0 -show_entries stream=duration -of default=noprint_wrappers=1:nokey=1 \"%s\"",
                            tbuf_group->working_directory, 
#else
                            "ffprobe -v error -select_streams v:0 -show_entries stream=duration -of default=noprint_wrappers=1:nokey=1 \"%s\"",
#endif
                            tbuf_group->input_path_buffer);

                    thread_info->prog_enum = ffprobe;
                    rt_vars->ffmpeg_is_running = true; //NOTE: has to be set beforehand, otherwise the main thread will continue too early
                    platform_ffmpeg_execute_command(tbuf_group, thread_info, rt_vars);

                    //TODO: try not to block the main thread
                    while(rt_vars->ffmpeg_is_running);

                    f32 media_duration = atof(tbuf_group->ffprobe_buffer);
#if 0
                    char _thing[256];
                    sprintf(_thing, 
                            "media_duration=%f\nffmpeg_is_running=%i\nffprobe_buffer:%s\n",
                            media_duration, rt_vars->ffmpeg_is_running, tbuf_group->ffprobe_buffer);
                    OutputDebugStringA(_thing);
#endif
                    if(media_duration) {
                        f32 bitrate_kb = (atof(target_filesize_buffer)*8000.0f) /
                                            media_duration;
                        sprintf(bitrate_result_buffer, 
                                "-b:v %.0fk", bitrate_kb);

                        log_diagnostic("[info]: ffprobe finished.", last_diagnostic_type::info, tbuf_group);
                    }
                    else {
                        tbuf_group->ffprobe_buffer[0] = 0x0;

                        log_diagnostic("failed to retrieve media stream duration.", last_diagnostic_type::error, tbuf_group);
                    }
                }
                else {
                    log_diagnostic("no target file size set.", last_diagnostic_type::error, tbuf_group);
                }
            }
            else {
                log_diagnostic("no input file specified.", last_diagnostic_type::error, tbuf_group);
            }
        }
        else {
            log_diagnostic("wait for running process to finish.", last_diagnostic_type::error, tbuf_group);
        }
    }

    ImGui::SameLine();
    ImGui::Text("result (KiB/s):");
    ImGui::SameLine();
    ImGui::PopItemWidth();

    ImGui::PushItemWidth(128.0f);
    ImGui::InputText("##bitrate_result", 
                    bitrate_result_buffer,
                    BITRATE_RESULT_BUFSIZE,
                    ImGuiInputTextFlags_ReadOnly);

    ImGui::PopItemWidth();
}

//ffprobe <input> -show_entries format=nb_streams -v 0 -of compact=p=0:nk=1
INTERNAL void 
argument_options_count_audio_tracks(text_buffer_group *tbuf_group,
                                    runtime_vars *rt_vars,
                                    platform_thread_info *thread_info,
                                    char *target_filesize_buffer,
                                    char *bitrate_buf) 
{
#define AUDIO_TRACK_COUNT_BUFSIZE 16
    LOCAL_STATIC char audio_track_count_buf[AUDIO_TRACK_COUNT_BUFSIZE] = {0};

    if(ImGui::Button("count audio tracks##count_audio_tracks")) {
        if(!rt_vars->ffmpeg_is_running) {
            if(tbuf_group->input_path_buffer[0]) {
                memset(tbuf_group->command_buffer, 0, strlen(tbuf_group->command_buffer));
                memset(tbuf_group->ffprobe_buffer, 0, strlen(tbuf_group->ffprobe_buffer));

                snprintf(
                        tbuf_group->command_buffer, PMEM_COMMANDBUFFERSIZE,
#if _2PACMPEG_WIN32
                       "%sffmpeg\\ffprobe \"%s\" -show_entries format=nb_streams -v 0 -of compact=p=0:nk=1",
                       tbuf_group->working_directory, tbuf_group->input_path_buffer
#else
                       "ffprobe \"%s\" -show_entries format=nb_streams -v 0 -of compact=p=0:nk=1",
                        tbuf_group->input_path_buffer
#endif
                );

                thread_info->prog_enum = ffprobe;
                rt_vars->ffmpeg_is_running = true;
                platform_ffmpeg_execute_command(tbuf_group, thread_info, rt_vars);

                while(rt_vars->ffmpeg_is_running);

                if(atoi(tbuf_group->ffprobe_buffer)) {
                    strncpy(audio_track_count_buf, 
                            tbuf_group->ffprobe_buffer,
                            AUDIO_TRACK_COUNT_BUFSIZE);

                    log_diagnostic("[info]: ffprobe finished.", last_diagnostic_type::info, tbuf_group);
                }
                else {
                    tbuf_group->ffprobe_buffer[0] = 0x0;

                    log_diagnostic("failed to retrieve audio track count.", last_diagnostic_type::error, tbuf_group);
                }
            }
            else {
                log_diagnostic("no input file specified.", last_diagnostic_type::error, tbuf_group);
            }
        }
        else {
            log_diagnostic("wait for running process to finish.", last_diagnostic_type::error, tbuf_group);
        }
    }

    ImGui::SameLine();
    ImGui::Text("result:");
    ImGui::SameLine();
    ImGui::PushItemWidth(64.0f);

    ImGui::InputText("##audio_track_count_result",
                    audio_track_count_buf, AUDIO_TRACK_COUNT_BUFSIZE,
                    ImGuiInputTextFlags_ReadOnly);

    ImGui::SameLine();
    if(ImGui::Button("merge##merge_tracks_button")) {
        int user_cmdbuf_len = strlen(tbuf_group->user_cmd_buffer);

        if((tbuf_group->user_cmd_buffer[user_cmdbuf_len] != ' ') &&
                (user_cmdbuf_len < PMEM_USRCOMMANDBUFFERSIZE && 
                user_cmdbuf_len > 0)) {
            tbuf_group->user_cmd_buffer[user_cmdbuf_len] = ' ';
            ++user_cmdbuf_len;
            tbuf_group->user_cmd_buffer[user_cmdbuf_len] = 0x0;
        }

        int audio_track_count = atoi(audio_track_count_buf);
        if(audio_track_count) {
            snprintf(tbuf_group->user_cmd_buffer + user_cmdbuf_len, 
                    PMEM_USRCOMMANDBUFFERSIZE - user_cmdbuf_len,
                    "-ac %i -filter_complex amerge=inputs=%i",
                    audio_track_count, audio_track_count);
        }
    }

    ImGui::PopItemWidth();
}

INTERNAL void 
argument_options(text_buffer_group *tbuf_group,
                runtime_vars *rt_vars,
                platform_thread_info *thread_info,
                char *target_filesize_buffer,
                char *bitrate_buf) 
{
    ImGui::PushStyleColor(ImGuiCol_ChildBg, IM_COL32(0x22, 0x22, 0x33, 0xFF));

    ImGui::BeginChild("##arg_options", ImVec2(0.0f, 0.0f), 
                    ImGuiChildFlags_AutoResizeY|ImGuiChildFlags_NavFlattened);
    //padding
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0f);

    argument_options_calculate_bitrate(tbuf_group,
                                        rt_vars,
                                        thread_info,
                                        target_filesize_buffer,
                                        bitrate_buf);

    argument_options_count_audio_tracks(tbuf_group,
                                        rt_vars,
                                        thread_info,
                                        target_filesize_buffer,
                                        bitrate_buf);

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3.0f); //yea its mismatched just let it happen baby
    ImGui::EndChild();
    ImGui::PopStyleColor();
}

INTERNAL void 
add_args_to_presets(text_buffer_group *tbuf_group,
                    preset_table *p_table,
                    char *preset_name_buffer) 
{
    tbuf_group->diagnostic_buffer[0] = 0x0;

    if(p_table->entry_amount < MAX_PRESETS) {
        if(strlen(preset_name_buffer) > 0) {
            if(!check_duplicate_presetname(p_table, preset_name_buffer)) {
                //CLEANUP
                s8 *new_cmd_start = tbuf_group->config_buffer +
                                    strlen(tbuf_group->config_buffer);

                int preset_name_len = strlen(preset_name_buffer);

                serialize_preset(preset_name_buffer, 
                        tbuf_group->user_cmd_buffer,
                        tbuf_group);
                insert_preset_name(p_table, preset_name_buffer,
                                    preset_name_len, p_table->entry_amount);

                p_table->command_table[p_table->entry_amount] = new_cmd_start + preset_name_len + 2;
                ++p_table->entry_amount;

                log_diagnostic("[info]: preset saved.", last_diagnostic_type::info, tbuf_group);
            }
            else {
                log_diagnostic("preset name already exists.", last_diagnostic_type::error, tbuf_group);
            }
        } 
        else {
            log_diagnostic("preset must have a name.", last_diagnostic_type::error, tbuf_group);
        }
    } 
    else {
        log_diagnostic("maximum preset amount exceeded (lmao how).", last_diagnostic_type::error, tbuf_group);
    }
}

INTERNAL void 
menu_start_ffmpeg(text_buffer_group *tbuf_group,
                runtime_vars *rt_vars,
                platform_thread_info *thread_info) 
{
    tbuf_group->diagnostic_buffer[0] = 0x0;

    if(!rt_vars->ffmpeg_is_running) {
        if(tbuf_group->input_path_buffer[0]) {
            memset(tbuf_group->command_buffer, 0, strlen(tbuf_group->command_buffer));
            memset(tbuf_group->stdout_buffer, 0, strlen(tbuf_group->stdout_buffer));
            memset(tbuf_group->stdout_line_buffer, 0, strlen(tbuf_group->stdout_line_buffer));

            if(!strchr(
                    tbuf_group->output_path_buffer, 
#if _2PACMPEG_WIN32
                    '\\'
#else
                    '/'
#endif
                    ) && tbuf_group->default_path_buffer[0]) {
                snprintf(tbuf_group->command_buffer,
                        PMEM_COMMANDBUFFERSIZE,
#if _2PACMPEG_WIN32
                        "%sffmpeg\\ffmpeg.exe -y -hide_banner -i \"%s\" %s \"%s\\%s\"",
#else
                        "ffmpeg -y -hide_banner -i \"%s\" %s \"%s\\%s\"",
#endif
                        tbuf_group->working_directory,
                        tbuf_group->input_path_buffer,
                        tbuf_group->user_cmd_buffer,
                        tbuf_group->default_path_buffer,
                        tbuf_group->output_path_buffer);
#if _2PACMPEG_DEBUG
    #if _2PACMPEG_WIN32
                OutputDebugStringA(tbuf_group->command_buffer);
                OutputDebugStringA("\n");
    #else
                printf("%s\n", tbuf_group->command_buffer);
    #endif
#endif
            }
            else {
                snprintf(tbuf_group->command_buffer,
                        PMEM_COMMANDBUFFERSIZE,
#if _2PACMPEG_WIN32
                        "%s\\ffmpeg\\ffmpeg.exe -y -hide_banner -i \"%s\" %s \"%s\"",
                        tbuf_group->working_directory,
#else
                        "ffmpeg -y -hide_banner -i \"%s\" %s \"%s\"",
#endif
                        tbuf_group->input_path_buffer,
                        tbuf_group->user_cmd_buffer,
                        tbuf_group->output_path_buffer);
            }

            thread_info->prog_enum = ffmpeg;
            platform_ffmpeg_execute_command(tbuf_group,
                                            thread_info,
                                            rt_vars);
        }
        else {
            log_diagnostic("no input file specified.",
                                last_diagnostic_type::error,
                                tbuf_group);
        }
    }
    else {
        log_diagnostic("FFmpeg is already running.",
                            last_diagnostic_type::error,
                            tbuf_group);
    }
}

INTERNAL void 
basic_controls_update(text_buffer_group *tbuf_group, 
                    preset_table *p_table, 
                    runtime_vars *rt_vars, 
                    platform_thread_info *thread_info) 
{
    LOCAL_STATIC s8 preset_name_buffer[PRESETNAME_PITCH - 1] = {0};
    LOCAL_STATIC s8 target_filesize_buffer[SMALL_TEXTBUF_SIZE];
    LOCAL_STATIC s8 bitrate_buffer[SMALL_TEXTBUF_SIZE];
    LOCAL_STATIC last_diagnostic_type diagnostic_type = undefined;
    LOCAL_STATIC bool32 arg_options_visible = false;

    ImGui::PushItemWidth(ImGui::GetColumnWidth(-1) - 15.0f);

#if _2PACMPEG_WIN32
    //NOTE: cant do something like this on x11 so no need to even put it on the screen
    if(ImGui::Button("select input file")) {
        tbuf_group->diagnostic_buffer[0] = 0x0;

        platform_file_input_dialog(tbuf_group->wchar_input_buffer);
        wcstombs(tbuf_group->input_path_buffer,
                tbuf_group->wchar_input_buffer, 
                PMEM_INPUTPATHBUFFERSIZE);
    }
    ImGui::SameLine();
#endif

    if(ImGui::Button("clear##clear_path")) {
        tbuf_group->diagnostic_buffer[0] = 0x0;

        memset(tbuf_group->input_path_buffer, 0, 
                strlen(tbuf_group->input_path_buffer));
    }

    ImGui::Text("input file path:");
    ImGui::InputText("##input_file_name", 
                    tbuf_group->input_path_buffer,
                    PMEM_INPUTPATHBUFFERSIZE);

    if(ImGui::Button("toggle argument options##arg_options")) {
        arg_options_visible = ~arg_options_visible;
    }

    if(arg_options_visible) {
        argument_options(tbuf_group, rt_vars, thread_info, 
                        target_filesize_buffer, bitrate_buffer);
    }

    ImGui::Text("args for FFmpeg:");
    ImGui::InputText("##ffmpeg_args", tbuf_group->user_cmd_buffer, PMEM_USRCOMMANDBUFFERSIZE);

    if(ImGui::Button("add current args to presets")) {
        add_args_to_presets(tbuf_group, p_table, preset_name_buffer);
    }

    ImGui::SameLine();
    ImGui::Text("preset name ->");
    ImGui::SameLine();
    ImGui::PushItemWidth(ImGui::GetColumnWidth() - ImGui::GetCursorPosX() - 7.0f);
    ImGui::InputText("##preset_name", preset_name_buffer, PRESETNAME_PITCH - 1);
    ImGui::PopItemWidth();

    ImGui::Text("path to output file:");
    ImGui::InputText("##output_path", 
                    tbuf_group->output_path_buffer,
                    PMEM_OUTPUTPATHBUFFERSIZE);

#define DEFAULT_OUTPUT_FOLDER_BUTTON 0
#if DEFAULT_OUTPUT_FOLDER_BUTTON
    if(ImGui::Button("to default output folder")) {
        if(tbuf_group->default_path_buffer[0]) {
            if(tbuf_group->output_path_buffer[0]) {
                tbuf_group->temp_buffer[0] = 0x0;
                strncpy(tbuf_group->temp_buffer,
                        tbuf_group->output_path_buffer,
                        PMEM_TEMPBUFFERSIZE);
                tbuf_group->temp_buffer[strlen(tbuf_group->temp_buffer)] = 0x0;

                snprintf(tbuf_group->output_path_buffer,
                        PMEM_OUTPUTPATHBUFFERSIZE,
                        "%s\\%s\0",
                        tbuf_group->default_path_buffer, 
                        tbuf_group->temp_buffer);
            } 
            else {
                strncpy(tbuf_group->output_path_buffer,
                        tbuf_group->default_path_buffer,
                        PMEM_OUTPUTPATHBUFFERSIZE);
            }
        } 
        else {
            log_diagnostic("default output directory not set",
                                last_diagnostic_type::error,
                                tbuf_group);
        }
    }
#endif

    ImGui::Text("default output folder:");

    ImGui::InputText("##default_output_path",
                    tbuf_group->default_path_buffer,
                    PMEM_OUTPUTPATHBUFFERSIZE);
    if(ImGui::Button("set as default folder")) {
        tbuf_group->diagnostic_buffer[0] = 0x0;

        save_default_output_path(tbuf_group, p_table);
    }

    if(ImGui::Button("start FFmpeg")) {
        menu_start_ffmpeg(tbuf_group, rt_vars, thread_info);
    }

    ImGui::SameLine();
    if(ImGui::Button("clear output##clear_output")) {
        tbuf_group->diagnostic_buffer[0] = 0x0;

        memset(tbuf_group->stdout_buffer, 0, strlen(tbuf_group->stdout_buffer));
        memset(tbuf_group->stdout_line_buffer, 0, strlen(tbuf_group->stdout_line_buffer));
    }

    // ImGui::SameLine(ImGui::GetColumnWidth() - 150);
    ImGui::SameLine(ImGui::GetColumnWidth() - 
                    ImGui::CalcTextSize("kill FFmpeg").x - 15.0f);
    if(ImGui::Button("kill FFmpeg")) {
        tbuf_group->diagnostic_buffer[0] = 0x0;

        if(rt_vars->ffmpeg_is_running) {
            if(platform_kill_process(thread_info)) {
                rt_vars->ffmpeg_is_running = false;

                log_diagnostic("[info]: FFmpeg terminated.", last_diagnostic_type::info, tbuf_group);
            }
        }
        else {
            log_diagnostic("FFmpeg is not running.", last_diagnostic_type::error, tbuf_group);
        }
    }

    ImGui::InputTextMultiline("##ffmpeg_output", tbuf_group->stdout_buffer, 
                            PMEM_STDOUTBUFFERSIZE, 
                            ImVec2((f32)(ImGui::GetColumnWidth() - 15.0f),
                                    (f32)(rt_vars->win_height - ImGui::GetCursorPosY()) - 25.0f), 
                            ImGuiInputTextFlags_ReadOnly);

    if(rt_vars->ffmpeg_is_running) {
        ImGui::BeginChild("##ffmpeg_output");
        ImGui::SetScrollHereY(1.0f);
        ImGui::EndChild();
    }

    ImGui::PopItemWidth();
}

INTERNAL void 
preset_list_update(text_buffer_group *tbuf_group, 
                    preset_table *p_table, 
                    runtime_vars *rt_vars) 
{
    ImGui::Text("presets:");
    ImGui::BeginChild("argument_presets", ImVec2((f32)ImGui::GetColumnWidth(),
                                            (f32)rt_vars->win_height - 45.0f));

    for(int preset_index = 0;
            preset_index < p_table->entry_amount;
            ++preset_index) {
        if(ImGui::Button(p_table->name_array + (preset_index * PRESETNAME_PITCH), 
                ImVec2(ImGui::GetColumnWidth(-1) - 10.0f, 20.0f))) {

            memset(tbuf_group->user_cmd_buffer, 0, strlen(tbuf_group->user_cmd_buffer));

            int cmd_length = command_length(p_table->command_table[preset_index]);
            if(cmd_length != -1) {
                strncpy(tbuf_group->user_cmd_buffer,
                        p_table->command_table[preset_index],
                        cmd_length);
            }
        }

        if(ImGui::BeginPopupContextItem(p_table->name_array + 
                (preset_index * PRESETNAME_PITCH))) {
            // TODO: change the names
            if(ImGui::Button("remove")) {
                remove_preset(p_table, tbuf_group, preset_index);
            }

            ImGui::EndPopup();
        }
    }

    ImGui::EndChild();
}

INTERNAL void 
update_window(text_buffer_group *tbuf_group, preset_table *p_table, 
            runtime_vars *rt_vars, platform_thread_info *thread_info) 
{
    glClearColor(0, 0, 0, 0xff);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwPollEvents();

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);

    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 70, 0, 0xFF));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(0, 70, 0, 0xFF));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(0, 0, 0, 0xFF));

    ImGui::Begin(".", 0, ImGuiWindowFlags_NoTitleBar
                           | ImGuiWindowFlags_NoResize
                           | ImGuiWindowFlags_NoMove
                           | ImGuiWindowFlags_NoScrollbar
                           | ImGuiWindowFlags_NoSavedSettings
                           | ImGuiWindowFlags_NoDecoration);

#if defined(_2PACMPEG_IMGUI_METRICS)
    ImGui::ShowMetricsWindow();
#endif

    //////////////////////

    //this is pretty dumb
    LOCAL_STATIC bool32 first_loop = true;
    ImGui::Columns(2, "columns");
    if(first_loop) {
        first_loop = false;
        ImGui::SetColumnWidth(0, (f32)rt_vars->win_width * 0.75);
    }

    basic_controls_update(tbuf_group, p_table, rt_vars, thread_info);
    ImGui::NextColumn();
    preset_list_update(tbuf_group, p_table, rt_vars);

    ImGui::NextColumn();

    show_diagnostic(tbuf_group);

    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    //////////////////////

    ImGui::End();
    ImGui::Render();
    glfwGetFramebufferSize(rt_vars->win_ptr, &rt_vars->win_width, 
                                            &rt_vars->win_height);
    glClearColor(0, 0, 0, 0xff);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(rt_vars->win_ptr);
}
