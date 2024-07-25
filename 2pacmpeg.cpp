
/*
TODO:
finish the default output path stuff

TODO: 
improve ux

TODO: 
make it possible to edit preset names and shit 
*/

#include "2pacmpeg.h"

// TODO: rename? since it's not really a "callback" anymore
INTERNAL last_diagnostic_type
diagnostic_callback(s8 *message = 0, 
                    last_diagnostic_type type = undefined,
                    text_buffer_group *tbuf_group = 0) {
    LOCAL_STATIC last_diagnostic_type last_diagnostic = undefined;
    
    if(message && tbuf_group) {
        strncpy(tbuf_group->diagnostic_buffer,
                message,
                PMEM_DIAGNOSTICBUFFERSIZE);
        tbuf_group->diagnostic_buffer[strlen(tbuf_group->diagnostic_buffer)] = 0x0;
    }

    if(type != undefined) {
        last_diagnostic = type; 
    }
 
    return last_diagnostic;
}

inline void *
heapbuf_alloc_region(program_memory *pool, u64 region_size) {
    void *result = 0;
    u64 free_memory = ((u64)pool->memory + pool->capacity) -
                           (u64)pool->write_ptr;
    if(region_size <= free_memory) {
        result = pool->write_ptr;
        pool->write_ptr = (void *)((u64)pool->write_ptr + region_size);
    }

    return result;
}

INTERNAL void
save_default_output_path(text_buffer_group *tbuf_group,
                        preset_table *p_table) {
    s8 *default_dir_begin;

    if(platform_file_exists(tbuf_group->config_path)) {
        default_dir_begin = strchr(tbuf_group->config_buffer, TOKEN_OUTPUTDIR);

        if(default_dir_begin) {
            int existing_dir_len = command_length(default_dir_begin);

            strncpy(tbuf_group->temp_buffer, (s8 *)((u64)tbuf_group->config_buffer + (u64)default_dir_begin),
                    PMEM_CONFIGBUFFERSIZE - existing_dir_len);
            tbuf_group->temp_buffer[strlen(tbuf_group->temp_buffer)] = 0x0;

            snprintf(tbuf_group->config_buffer, PMEM_CONFIGBUFFERSIZE,
                    "%c%s\n%s\0",
                    TOKEN_OUTPUTDIR, tbuf_group->default_path_buffer,
                    tbuf_group->temp_buffer);
        }
        else { // (edge case)
            strncpy(tbuf_group->temp_buffer, tbuf_group->config_buffer,
                    PMEM_CONFIGBUFFERSIZE);
            tbuf_group->temp_buffer[strlen(tbuf_group->temp_buffer)] = 0x0;

            snprintf(tbuf_group->config_buffer, PMEM_CONFIGBUFFERSIZE,
                    "%c%s\n%s\0",
                    TOKEN_OUTPUTDIR, tbuf_group->default_path_buffer,
                    tbuf_group->temp_buffer);
        }
    }
    else {
        snprintf(tbuf_group->config_buffer, PMEM_CONFIGBUFFERSIZE,
                "%c%s\n\0",
                TOKEN_OUTPUTDIR, tbuf_group->default_path_buffer);
    }
}

INTERNAL bool32
serialize_preset(s8 *preset_name, s8 *preset_command,
                text_buffer_group *tbuf_group) {

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
        diagnostic_callback("[info]: configuration updated. (preset added).",
                            last_diagnostic_type::info,
                            tbuf_group);
#if _2PACMPEG_DEBUG
        OutputDebugStringA("[info]: configuration updated. (preset added).\n");
#endif
    } 
    else {
        char __diagnostic[128];
        sprintf(__diagnostic, 
                "[file write error]: could not write preset file %i",
                GetLastError());
        diagnostic_callback(__diagnostic,
                            last_diagnostic_type::error,
                            tbuf_group);
    }

    return result;
}

// NOTE: preset_name can be a pointer to a buffer that might not be null-terminated
inline void
insert_preset_name(preset_table *p_table, s8 *preset_name,
                int preset_name_length, int insert_index) {
    // NOTE: 64 byte pitch for names
    int insert_offset = PRESETNAME_PITCH*insert_index;

    strncpy(p_table->name_array + insert_offset,
            preset_name, preset_name_length);
}

inline int
command_length(s8 *command_begin) {
    int result = -1;

    s8 *command_end = strchr(command_begin, (s8)'\n');
    if(!command_end) {
        s8 *command_end = strchr(command_begin, 
                                (s8)'\0');
    }
    if(command_end) {
        result = command_end - command_begin;
    }

    return result;
}

INTERNAL void 
remove_preset(preset_table *p_table, text_buffer_group *tbuf_group, 
                                                    int rm_index) {
    // @CLEANUP
    s8 *whole_preset = (p_table->command_table[rm_index] - (strlen(p_table->name_array + (rm_index * PRESETNAME_PITCH)))) - 2;
    u32 preset_length = command_length(whole_preset) + 1; // +1 for \n

    if(preset_length == -1) {
        diagnostic_callback("[config error]: something weird happened.",
                            last_diagnostic_type::error,
                            tbuf_group);
        MessageBoxA(0, 
                    "Failed to retrieve the length of the command being deleted.\nPLz report this to the genius who wrote this trash\n(You can continue using this program, but the preset could not be deleted)",
                    "CONFIG ERROR",
                    MB_OK|MB_ICONERROR);

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

        // TODO: i dont really know how slow this can get
        // but try to see if it can be done faster
        for(int move_index = rm_index;
                move_index < p_table->entry_amount - 1;
                ++move_index) {
            if(!move_index) {
                p_table->command_table[0] = 
                    strchr(tbuf_group->config_buffer, TOKEN_PRESETCMD) + 1;
            } 
            else {
                p_table->command_table[move_index] = 
                        strchr(p_table->command_table[move_index - 1], 
                            TOKEN_PRESETCMD) + 1;
            }
        }

        p_table->command_table[p_table->entry_amount - 1] = 0;
    } 
    else {
        memset(whole_preset, 0, preset_length);

        s8 *rm_preset_name = p_table->name_array + (rm_index * PRESETNAME_PITCH);
        memset(rm_preset_name, 0, strlen(rm_preset_name));
        p_table->command_table[rm_index] = 0;
    }

    --p_table->entry_amount;

    if(platform_write_file(tbuf_group->config_path,
                        (void *)tbuf_group->config_buffer,
                        strlen(tbuf_group->config_buffer))) {
        diagnostic_callback("[info]: configuration updated. (preset deleted)",
                            last_diagnostic_type::info,
                            tbuf_group);
#if defined(_2PACMPEG_DEBUG)
        OutputDebugStringA("[info]: configuration updated. (preset deleted)\n");
#endif
    }
    else {
        char __diagnostic[512];
        snprintf(__diagnostic, 512,
                "[error]: updating configuration failed with code %i",
                GetLastError());
        diagnostic_callback(__diagnostic,
                            last_diagnostic_type::error,
                            tbuf_group);
    }
}

// NOTE: a little slow
inline bool32
check_duplicate_presetname(preset_table *p_table, s8 *p_name) {
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

// TODO: align ui elements (mostly text fields) in this function properly
INTERNAL void
basic_controls_update(text_buffer_group *tbuf_group, preset_table *p_table, 
        runtime_vars *rt_vars, platform_thread_info *thread_info) {
    LOCAL_STATIC s8 preset_name_buffer[PRESETNAME_PITCH - 1] = {0}; // -1 for '\0'
    LOCAL_STATIC last_diagnostic_type diagnostic_type = undefined;

    ImGui::PushItemWidth(ImGui::GetColumnWidth(-1) - 15.0f);

    if(ImGui::Button("select input file")) {
        tbuf_group->diagnostic_buffer[0] = 0x0;

        platform_file_input_dialog(tbuf_group->wchar_input_buffer);
        wcstombs(tbuf_group->input_path_buffer,
                tbuf_group->wchar_input_buffer, PMEM_INPUTPATHBUFFERSIZE);
    }

    ImGui::SameLine();
    if(ImGui::Button("clear##clear_path")) {
        tbuf_group->diagnostic_buffer[0] = 0x0;

        memset(tbuf_group->input_path_buffer, 0, 
                strlen(tbuf_group->input_path_buffer));
    }

    ImGui::Text("input file path:");
    ImGui::InputText("##input_file_name", tbuf_group->input_path_buffer,
                                            PMEM_INPUTPATHBUFFERSIZE);
    ImGui::Text("args for FFmpeg:");
    ImGui::InputText("##ffmpeg_args", tbuf_group->user_cmd_buffer, PMEM_USRCOMMANDBUFFERSIZE);

    if(ImGui::Button("add current args to presets")) {
        tbuf_group->diagnostic_buffer[0] = 0x0;

        if(p_table->entry_amount < MAX_PRESETS) {
            if(strlen(preset_name_buffer) > 0) {
                if(!check_duplicate_presetname(p_table, preset_name_buffer)) {
                    // @CLEANUP
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

                    diagnostic_callback("[info]: preset saved.", 
                                        last_diagnostic_type::info,
                                        tbuf_group);
                }
                else {
                    diagnostic_callback("preset name already exists.", 
                                        last_diagnostic_type::error,
                                        tbuf_group);
                }
            } 
            else {
                diagnostic_callback("preset must have a name.", 
                                    last_diagnostic_type::error,
                                    tbuf_group);
            }
        } 
        else {
            diagnostic_callback("maximum preset amount exceeded (lol).", 
                                last_diagnostic_type::error,
                                tbuf_group);
        }
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

    if(ImGui::Button("to default output directory")) {
        if(tbuf_group->default_path_buffer[0]) {
            if(tbuf_group->output_path_buffer[0]) {
                tbuf_group->temp_buffer[0] = 0x0;
                strncpy(tbuf_group->temp_buffer,
                        tbuf_group->output_path_buffer,
                        PMEM_OUTPUTPATHBUFFERSIZE);
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
            diagnostic_callback("default output directory not set",
                                last_diagnostic_type::error,
                                tbuf_group);
        }
    }

    ImGui::SameLine();
    if(ImGui::Button("set as default directory")) {
        save_default_output_path(tbuf_group, p_table);
    }

    ImGui::Text("default output directory:");
    ImGui::InputText("##default_output_path",
                    tbuf_group->default_path_buffer,
                    PMEM_OUTPUTPATHBUFFERSIZE);

    if(ImGui::Button("start")) {
        tbuf_group->diagnostic_buffer[0] = 0x0;

        if(!rt_vars->ffmpeg_is_running) {
            if(tbuf_group->input_path_buffer[0]) {
                memset(tbuf_group->command_buffer, 0, strlen(tbuf_group->command_buffer));
                memset(tbuf_group->stdout_buffer, 0, strlen(tbuf_group->stdout_buffer));
                memset(tbuf_group->stdout_line_buffer, 0, strlen(tbuf_group->stdout_line_buffer));

                // NOTE: -y to always overwrite existing files, as per the request of FagBlazt
                snprintf(tbuf_group->command_buffer,
                        PMEM_COMMANDBUFFERSIZE,
                        "%s -y -hide_banner -i \"%s\" %s \"%s\"",
                        tbuf_group->ffmpeg_path,
                        tbuf_group->input_path_buffer,
                        tbuf_group->user_cmd_buffer,
                        tbuf_group->output_path_buffer);

                platform_ffmpeg_execute_command(tbuf_group,
                                                thread_info,
                                                rt_vars);
            }
            else {
                diagnostic_callback("no input file specified.",
                                    last_diagnostic_type::error,
                                    tbuf_group);
            }
        }
        else {
            diagnostic_callback("FFmpeg is already running.",
                                last_diagnostic_type::error,
                                tbuf_group);
        }
    }

    ImGui::SameLine();
    if(ImGui::Button("clear##clear_output")) {
        tbuf_group->diagnostic_buffer[0] = 0x0;

        memset(tbuf_group->stdout_buffer, 0, strlen(tbuf_group->stdout_buffer));
        memset(tbuf_group->stdout_line_buffer, 0, strlen(tbuf_group->stdout_line_buffer));
    }

    // ImGui::SameLine(ImGui::GetColumnWidth() - 150);
    ImGui::SameLine(ImGui::GetColumnWidth() - 
                    ImGui::CalcTextSize("kill FFmpeg").x - 15.0f);
    if(ImGui::Button("kill FFmpeg")) {
        tbuf_group->diagnostic_buffer[0] = 0x0;

        // NOTE: this is some windows-specific shit
        if(rt_vars->ffmpeg_is_running) {
            if(TerminateProcess(thread_info->cmd_stream_processinfo.hProcess, 
                                                PROCESS_TERMINATE)) {
                rt_vars->ffmpeg_is_running = false;

                diagnostic_callback("[info]: FFmpeg terminated.",
                                    last_diagnostic_type::info,
                                    tbuf_group);
            }
        }
        else {
            diagnostic_callback("FFmpeg is not running.",
                                last_diagnostic_type::error,
                                tbuf_group);
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
            preset_table *p_table, runtime_vars *rt_vars) {
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
        runtime_vars *rt_vars, platform_thread_info *thread_info) {
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

    switch(diagnostic_callback()) {
    case error: {
        if(tbuf_group->diagnostic_buffer[0]) {
            ImGui::PushStyleColor(ImGuiCol_Text, 
                                IM_COL32(0xff, 0, 0, 0xFF));
        }
    } break;

    case info: {
        if(tbuf_group->diagnostic_buffer[0]) {
            ImGui::PushStyleColor(ImGuiCol_Text, 
                                IM_COL32(0, 0xff, 0, 0xFF));
        }
    } break;

    case undefined:
    default: {
        ImGui::PushStyleColor(ImGuiCol_Text, 
                        IM_COL32(0xcc, 0xcc, 0xcc, 0xFF));
    } break;
    }

    ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5.0f);
    ImGui::Text(tbuf_group->diagnostic_buffer);

    if(tbuf_group->diagnostic_buffer[0]) {
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleColor();
    ImGui::PopStyleColor();
    ImGui::PopStyleColor();

    //////////////////////

    ImGui::End();
    ImGui::Render();
    glfwGetFramebufferSize(rt_vars->win_ptr, &rt_vars->win_width, 
                                            &rt_vars->win_height);
    // glClearColor(0, 0, 0, 0xff);
    // glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(rt_vars->win_ptr);
}
