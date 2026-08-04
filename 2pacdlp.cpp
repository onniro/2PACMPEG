
/*
File: 2pacdlp.cpp
Date: Sun 12 Apr 2026 02:12:15 PM EEST

yt-dlp front-end "extension" added in 3.0
*/

#include "2pacmpeg.h"

#define _2PACDLP_OPTS_STR_SIZE       (8192)
#define _2PACDLP_DL_SECT_STR_SIZE    (256)

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

struct tupacdlp_options {
    bool disable_video;
    bool disable_audio;
    video_format selected_video_format;
    audio_format selected_audio_format;
    char dl_sections_buffer[_2PACDLP_DL_SECT_STR_SIZE];
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
}

static void start_download(runtime_vars *rt_vars,
                        text_buffer_group *tbuf_group,
                        platform_thread_info *thread_info,
                        tupacdlp_options *options) {
    tbuf_group->diagnostic_buffer[0] = 0;
    if (rt_vars->ffmpeg_is_running) {
        if (platform_kill_process(thread_info))
        { rt_vars->ffmpeg_is_running = false; }
    }

    memset(tbuf_group->command_buffer, 0, strlen(tbuf_group->command_buffer));
    memset(tbuf_group->stdout_buffer, 0, strlen(tbuf_group->stdout_buffer));
    memset(tbuf_group->stdout_line_buffer, 0, strlen(tbuf_group->stdout_line_buffer));

    make_options_string(options);

#if _2PACMPEG_WIN32
    snprintf(tbuf_group->command_buffer,
            PMEM_COMMANDBUFFERSIZE,
            "%s\\ffmpeg\\yt-dlp.exe --ffmpeg-location \"%s\\ffmpeg\\ffmpeg.exe \" %s \"%s\" -o \"%s\"",
            tbuf_group->working_directory,
            tbuf_group->working_directory,
            options->options_string,
            tbuf_group->download_url_buffer,
            tbuf_group->download_outpath_buffer);
#elif _2PACMPEG_LINUX
    snprintf(tbuf_group->command_buffer,
            PMEM_COMMANDBUFFERSIZE,
            "yt-dlp %s \"%s\" -o \"%s\"",
            options->options_string,
            tbuf_group->download_url_buffer,
            tbuf_group->download_outpath_buffer);
#endif
    thread_info->prog_enum = program_enum_ytdlp;
    platform_execute_command(tbuf_group, thread_info, rt_vars, true);
}

static void do_2pacdlp(text_buffer_group *tbuf_group, 
                    preset_table *p_table, 
                    runtime_vars *rt_vars, 
                    platform_thread_info *thread_info) {
    LOCAL_STATIC tupacdlp_options options = {
        .selected_video_format = video_format_auto,
        .selected_audio_format = audio_format_auto
    };
    //char ytdlp_exists = platform_check_ytdlp_existence(tbuf_group);
    saved_paths_array *paths_array = &rt_vars->paths_array;
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
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0.5f));
        for (int format_index = 0;
            format_index < (int)video_format__LAST;
            ++format_index) {
            if (ImGui::Button(video_format_strings[format_index], 
                ImVec2(ImGui::GetContentRegionAvail().x, 20))) {
                options.selected_video_format = (video_format)format_index;
            }
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        ImGui::EndCombo();
    }

    ImGui::SameLine();
    ImGui::PushItemWidth(100);
    if (ImGui::BeginCombo("audio format##audio_format",
        audio_format_strings[options.selected_audio_format])) {
        ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2(0, 0.5f));
        for (int format_index = 0;
            format_index < (int)audio_format__LAST;
            ++format_index) {
            if (ImGui::Button(audio_format_strings[format_index], 
                ImVec2(ImGui::GetContentRegionAvail().x, 20))) {
                options.selected_audio_format = (audio_format)format_index;
            }
        }
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();
        ImGui::EndCombo();
    }
    ImGui::SameLine();

    ImGui::PopItemWidth();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 0xFF, 0, 0xFF));
    ImGui::Text("!");
    ImGui::PopStyleColor();
    ImGui::SetItemTooltip("note: 2PACDLP does not consider whether or not the selected \n"
                        "video/audio formats make sense with each other.");

    
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
                        "this also works with the name of a chapter if the video has any\n"
                        "a negative time range may also be used and it will be calculated from the end");

    if (ImGui::Button("download##2pacdlp_download"))
    { start_download(rt_vars, tbuf_group, thread_info, &options); }
    ImGui::SameLine();
    if (ImGui::Button("clear output##2pacdlp_clear_output")) {
        tbuf_group->diagnostic_buffer[0] = 0;
        memset(tbuf_group->stdout_buffer, 0, strlen(tbuf_group->stdout_buffer));
        memset(tbuf_group->stdout_line_buffer, 0, strlen(tbuf_group->stdout_line_buffer));
    }

    ImGui::SameLine(ImGui::GetColumnWidth() - ImGui::CalcTextSize("kill").x - 15.0f);
    //reminder that this just kills which ever program is running since there can only be one at a time
    if (ImGui::Button("kill##2pacdlp_kill")) {
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
