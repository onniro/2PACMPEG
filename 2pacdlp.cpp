
/*
File: 2pacdlp.cpp
Date: Sun 12 Apr 2026 02:12:15 PM EEST

yt-dlp front-end "extension" added in 3.0
*/

#include "2pacmpeg.h"

#define _2PACDLP_OPTS_STR_SIZE              (10240)
#define _2PACDLP_DL_SECT_STR_SIZE           (256)
#define _2PACDLP_ADVANCED_OPT_STR_SIZE      (4096)

enum video_format {
    video_format_auto = 0,
    video_format_avi,
    video_format_flv,
    video_format_gif,
    video_format_mkv, 
    video_format_mov, 
    video_format_mp4, 
    video_format_webm, 

    video_format__LAST
};

enum audio_format {
    audio_format_auto = 0,
    audio_format_aac, 
    audio_format_aiff, 
    audio_format_alac, 
    audio_format_flac, 
    audio_format_m4a, 
    audio_format_mka, 
    audio_format_mp3, 
    audio_format_ogg, 
    audio_format_opus, 
    audio_format_vorbis, 
    audio_format_wav,

    audio_format__LAST
};

enum cookie_source_browser {
    browser_none = 0,
    browser_brave,
    browser_chrome,
    browser_chromium,
    browser_edge,
    browser_firefox,
    browser_opera,
    browser_safari,
    browser_vivaldi,
    browser_whale,

    browser__LAST
};

char video_format_strings[][8] = {
    "auto", "avi", "flv", "gif",
    "mkv ", "mov ", "mp4 ", "webm"
};

char audio_format_strings[][8] = {
    "auto", "aac ", "aiff ",
    "alac", "flac ", "m4a ",
    "mka ", "mp3 ", "ogg ",
    "opus ", "vorbis ", "wav"
};

char browser_strings[][12] = {
    "none", "brave",
    "chrome", "chromium",
    "edge", "firefox",
    "opera", "safari",
    "vivaldi", "whale"
};

struct tupacdlp_options {
    bool disable_video;
    bool disable_audio;
    video_format selected_video_format;
    audio_format selected_audio_format;
    cookie_source_browser selected_browser;
    text_buffer_group *tbuf_group_ptr;
    char dl_sections_buffer[_2PACDLP_DL_SECT_STR_SIZE];
    char advanced_options[_2PACDLP_ADVANCED_OPT_STR_SIZE];
    char options_string[_2PACDLP_OPTS_STR_SIZE];
};

static void make_options_string(tupacdlp_options *options) {
    char *str = options->options_string;
    str[0] = 0;
    int str_length = 0, str_max = _2PACDLP_OPTS_STR_SIZE;

    if (options->disable_video) {
        str_length += snprintf(str + str_length, str_max - str_length, " -x ");
    } else if (options->disable_audio) {
        str_length += snprintf(str + str_length, str_max - str_length, " -f bv ");
    }

    if (!options->disable_video &&
        (options->selected_video_format != video_format_auto)) {
        str_length += snprintf(str + str_length,
                            str_max - str_length,
                            " --recode-video %s ",
                            video_format_strings[options->selected_video_format]);
    } if (!options->disable_audio &&
        (options->selected_audio_format != audio_format_auto)) {
        str_length += snprintf(str + str_length,
                            str_max - str_length,
                            " --audio-format %s ",
                            audio_format_strings[options->selected_audio_format]);
    }

    if (options->dl_sections_buffer[0]) {
        str_length += snprintf(str + str_length,
                            str_max - str_length,
                            " --download-sections \"%s\" ",
                            options->dl_sections_buffer);
    }

    if (options->selected_browser != browser_none) {
        str_length += snprintf(str + str_length,
                            str_max - str_length,
                            " --cookies-from-browser %s ",
                            browser_strings[options->selected_browser]);
    }

    if (options->advanced_options[0]) {
        str_length += snprintf(str + str_length,
                            str_max - str_length,
                            " %s ",
                            options->advanced_options);
    }
}

static void start_download(runtime_vars *rt_vars,
                        text_buffer_group *tbuf_group,
                        platform_thread_info *thread_info,
                        tupacdlp_options *options) {
    text_buffer_group *tbg = tbuf_group;
    tbg->diagnostic_buffer[0] = 0;
    if (rt_vars->ffmpeg_is_running) {
        if (platform_kill_process(thread_info))
        { rt_vars->ffmpeg_is_running = false; }
    }

    memset(tbg->command_buffer, 0, strlen(tbg->command_buffer));
    memset(tbg->stdout_buffer, 0, strlen(tbg->stdout_buffer));
    memset(tbg->stdout_line_buffer, 0, strlen(tbg->stdout_line_buffer));

    make_options_string(options);

    LOCAL_STATIC char outpath_buffer[PATH_MAX];
    if (platform_directory_exists(tbg->download_outpath_buffer)) {
        snprintf(outpath_buffer, PATH_MAX, "%s/%%(title)s.%%(ext)s", 
                tbg->download_outpath_buffer);
    } else if (!tbg->download_outpath_buffer[0]) {
        snprintf(outpath_buffer, PATH_MAX, "%s/%%(title)s.%%(ext)s", 
                tbg->working_directory);
    } else {
        strncpy(outpath_buffer, tbg->download_outpath_buffer, PATH_MAX - 1);
    }

#if _2PACMPEG_WIN32
    snprintf(tbg->command_buffer,
            PMEM_COMMANDBUFFERSIZE,
            "%s\\ffmpeg\\yt-dlp.exe --ffmpeg-location \"%s\\ffmpeg\\ffmpeg.exe \" %s \"%s\" -o \"%s\"",
            tbg->working_directory,
            tbg->working_directory,
            options->options_string,
            tbg->download_url_buffer,
            outpath_buffer);
#elif _2PACMPEG_LINUX
    snprintf(tbuf_group->command_buffer,
            PMEM_COMMANDBUFFERSIZE,
            "yt-dlp %s \"%s\" -o \"%s\"",
            options->options_string,
            tbg->download_url_buffer,
            outpath_buffer);
#endif
    thread_info->prog_enum = program_enum_ytdlp;
    platform_execute_command(tbuf_group, thread_info, rt_vars, true);
}

//xaxaxaxaxa
#define DO_COMBOBOX(titles_array, button_count, enum2set, enum_type) { \
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0)); \
    ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0.5f)); \
    for (int button_index = 0; \
        button_index < button_count; \
        ++button_index) { \
        if (ImGui::Button(titles_array[button_index],  \
            ImVec2(ImGui::GetContentRegionAvail().x, 20))) { \
            enum2set = (enum_type)button_index; \
        } \
    } \
    ImGui::PopStyleVar(); \
    ImGui::PopStyleColor(); \
} _2pacmpeg_nop()

static void do_2pacdlp(text_buffer_group *tbuf_group, 
                    preset_table *p_table, 
                    runtime_vars *rt_vars, 
                    platform_thread_info *thread_info) {
    LOCAL_STATIC tupacdlp_options options = {
        .selected_video_format = video_format_auto,
        .selected_audio_format = audio_format_auto,
        .tbuf_group_ptr = tbuf_group,
    };
    //char ytdlp_exists = platform_check_ytdlp_existence(tbuf_group);
    saved_paths_array *paths_array = &rt_vars->paths_array;

    {
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - 15.0f);
        ImGui::Text("url to download:");
        ImGui::InputText("##download url",
                tbuf_group->download_url_buffer,
                PMEM_URLBUFFERSIZE);
        ImGui::PopItemWidth();
#if _2PACMPEG_WIN32 && 0
        if (ImGui::Button("select output folder")) {
            tbuf_group->diagnostic_buffer[0] = 0;
            tbuf_group->wchar_input_buffer[0] = 0;
            platform_file_input_dialog(tbuf_group->wchar_input_buffer);
            wcstombs(tbuf_group->download_outpath_buffer,
                     tbuf_group->wchar_input_buffer, 
                     wcslen(tbuf_group->wchar_input_buffer));
        }
#endif
    }

    {
        ImGui::Text("output path:");
        ImGui::SetItemTooltip("note: in 2PACDLP, file extension (e.g. '.mp3') isn't needed in output file name");

        if (ImGui::BeginCombo("##saved_paths", "",
            ImGuiComboFlags_NoPreview)) {
            //ImGui::PushItemWidth(200);
            ImGui::Text("saved paths:");
            ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
            ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0.5f));
            char *current, btn_text[PATH_MAX + 8];
            for (int path_index = 0; path_index < paths_array->num_paths; ++path_index) {
                current = paths_array->paths[path_index];
                snprintf(btn_text, sizeof(btn_text), "%s##saved_path%d", current, path_index);

                if (ImGui::Button(btn_text))
                { strncpy(tbuf_group->download_outpath_buffer, current, PATH_MAX); }

                if (ImGui::BeginPopupContextItem(btn_text)) {
                    if (ImGui::Button("remove##2pacdlp_remove_path"))
                    { remove_saved_path(path_index, rt_vars); }
                    ImGui::EndPopup();
                }
            }
            //ImGui::PopItemWidth();
            ImGui::PopStyleVar();
            ImGui::PopStyleColor();
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x - ImGui::CalcTextSize("save").x - 30);
        ImGui::InputText("##download output path",
                tbuf_group->download_outpath_buffer,
                PMEM_OUTPUTPATHBUFFERSIZE);
        ImGui::SameLine();
        if (ImGui::Button("save"))
        { save_path(tbuf_group->download_outpath_buffer, rt_vars); }
        ImGui::PopItemWidth();
    }

    {
        if (ImGui::Checkbox("disable video", &options.disable_video)) {
            if (options.disable_audio) { options.disable_audio = false; }
        }
        ImGui::SameLine();
        if (ImGui::Checkbox("disable audio", &options.disable_audio)) {
            if (options.disable_video) { options.disable_video = false; }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        if (ImGui::BeginCombo("video format##video_format",
            video_format_strings[options.selected_video_format])) {
            DO_COMBOBOX(video_format_strings,
                    (int)video_format__LAST,
                    options.selected_video_format,
                    video_format);
            ImGui::EndCombo();
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(100);
        if (ImGui::BeginCombo("audio format##audio_format",
            audio_format_strings[options.selected_audio_format])) {
            DO_COMBOBOX(audio_format_strings,
                    (int)audio_format__LAST,
                    options.selected_audio_format,
                    audio_format);
            ImGui::EndCombo();
        }
    }
    
    {
        ImGui::PushItemWidth(200);
        ImGui::InputText("download section##download_sections",
                options.dl_sections_buffer,
                _2PACDLP_DL_SECT_STR_SIZE);
        ImGui::PopItemWidth();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0xFF, 0, 0xFF));
        ImGui::Text("!");
        ImGui::PopStyleColor();
        ImGui::SetItemTooltip("for downloading a time range, prefix the range with * (e.g. *10:10-11:11)\n"
                            "this also works with the name of a chapter if the video has any\n");

        ImGui::SameLine();
        if (ImGui::BeginCombo("browser for cookies##browser4cookies",
            browser_strings[options.selected_browser])) {
            DO_COMBOBOX(browser_strings,
                    (int)browser__LAST,
                    options.selected_browser,
                    cookie_source_browser);
            ImGui::EndCombo();
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0xFF, 0, 0xFF));
        ImGui::Text("!");
        ImGui::PopStyleColor();
        ImGui::SetItemTooltip("if the service you are downloading from requires logging in\n"
                            "to access the content you wish to download, try selecting\n"
                            "a browser with which you are logged into the service from this combo box");

        ImGui::SameLine();
        LOCAL_STATIC char advanced_options_win_open = false;
        if (ImGui::Button("enter advanced options##advanced_options_button") &&
            !advanced_options_win_open) {
            advanced_options_win_open = true;
            float w = 600, h = 60; 
            ImGui::SetNextWindowSize(ImVec2(w, h));
            ImVec2 winpos = ImVec2((rt_vars->win_width/2) - (w/2),
                                   (rt_vars->win_height/2) - (h/2));
            ImGui::SetNextWindowPos(winpos);
        }

        if (advanced_options_win_open) {
            if (ImGui::Begin("enter advanced options##advanced_options_window", 0,
                ImGuiWindowFlags_NoScrollbar
                |ImGuiWindowFlags_NoResize
                |ImGuiWindowFlags_NoCollapse)) {
                ImGui::SetKeyboardFocusHere(0);
                if (ImGui::IsKeyPressed(ImGuiKey_Escape) ||
                    (!ImGui::IsWindowHovered() &&
                    ImGui::IsMouseClicked(ImGuiMouseButton_Left)))
                { advanced_options_win_open = false; }

                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
                ImGui::InputText("##advanced_options_input",
                        options.advanced_options,
                        _2PACDLP_ADVANCED_OPT_STR_SIZE - 1);
                ImGui::End();
            }
        }
    }

    {
        if (ImGui::Button("start download##2pacdlp_download"))
        { start_download(rt_vars, tbuf_group, thread_info, &options); }
        ImGui::SameLine();
        if (ImGui::Button("clear output##2pacdlp_clear_output")) {
            tbuf_group->diagnostic_buffer[0] = 0;
            memset(tbuf_group->stdout_buffer, 0, strlen(tbuf_group->stdout_buffer));
            memset(tbuf_group->stdout_line_buffer, 0, strlen(tbuf_group->stdout_line_buffer));
        }

        ImGui::SameLine(ImGui::GetColumnWidth() - ImGui::CalcTextSize("kill yt-dlp").x - 15.0f);
        //reminder that this just kills which ever program is running since there can only be one at a time
        if (ImGui::Button("kill yt-dlp##2pacdlp_kill")) {
            tbuf_group->diagnostic_buffer[0] = 0x0;
            if (rt_vars->ffmpeg_is_running) {
                if (platform_kill_process(thread_info)) {
                    rt_vars->ffmpeg_is_running = false;
                    log_diagnostic("[info]: yt-dlp killed.", last_diagnostic_type::info, tbuf_group);
                } else { 
                    log_diagnostic("[bug]: yt-dlp could not be killed for an unknown reason.", 
                            last_diagnostic_type::error, 
                            tbuf_group); 
                }
            } else { 
                log_diagnostic("yt-dlp is not running.", 
                        last_diagnostic_type::error, 
                        tbuf_group); 
            }
        }
    }

    {
        ImGui::Text("output:");

        ImGui::PushItemWidth(ImGui::GetColumnWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(10, 10, 10, 0xFF));
        ImGui::PushStyleColor(ImGuiCol_Border, IM_COL32(100, 100, 100, 0xFF));
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 200, 0, 0xFF));

        ImGui::InputTextMultiline("##ytdlp_output", tbuf_group->stdout_buffer, 
                                  PMEM_STDOUTBUFFERSIZE, 
                                  ImVec2((f32)(ImGui::GetColumnWidth() - 15.0f),
                                         (f32)(rt_vars->win_height - ImGui::GetCursorPosY()) - 25.0f), 
                                  ImGuiInputTextFlags_ReadOnly);
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();

        if (rt_vars->ffmpeg_is_running) {
            ImGui::BeginChild("##ytdlp_output");
            ImGui::SetScrollHereY(1.0f);
            ImGui::EndChild();
        }
        ImGui::PopItemWidth();
    }
}
