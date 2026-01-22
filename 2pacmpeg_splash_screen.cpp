
/*
File: 2pacmpeg_splash_screen.cpp
Date: Wed 21 Jan 2026 08:58:40 PM EET

Special splash screen stuff requested by karka
*/

#define STB_IMAGE_IMPLEMENTATION 1
#include "stb_image.h"

static struct Logo_Bitmap {
    uint32_t bytes;
    int width;
    int height;
    int chan;
    uint8_t *data;
    GLuint ogl_tex_id;
    char is_initialized;
} global_logo_bitmap = {0};

#define SPLASH_SCREEN_DEFAULT_FRAMES (120)

static void init_splash(runtime_vars *rt_vars) {
    Logo_Bitmap *lb = &global_logo_bitmap;
    //char path[PATH_MAX];
#if 0
    snprintf(path, sizeof(path), "%s/../misc/FFMPAC_DE.png",
            rt_vars->tbuf_group_ptr->working_directory);
    snprintf(path, sizeof(path), "%s", rt_vars->cmd_opts_ptr->splash_image_path);
#else
    char *path = rt_vars->cmd_opts_ptr->splash_image_path;
    cmd_options *cmd_opts = rt_vars->cmd_opts_ptr;
    if (!cmd_opts->splash_image_path ||
        !platform_file_exists(cmd_opts->splash_image_path))
    { return; }
    
#endif

    lb->data = stbi_load(path,
                    &lb->width,
                    &lb->height,
                    &lb->chan,
                    STBI_rgb_alpha);
    if (!lb->data) {
        //fprintf(stderr, "error: couldn't load logo\n");
        return;
    }

    lb->ogl_tex_id = 0;
    glGenTextures(1, &lb->ogl_tex_id);
    glBindTexture(GL_TEXTURE_2D, lb->ogl_tex_id);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0,
                GL_RGBA,
                lb->width,
                lb->height,
                0,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                lb->data);
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(lb->data);

    lb->is_initialized = 1;
}

static void do_splash_screen(runtime_vars *rt_vars) {
    Logo_Bitmap *lb = &global_logo_bitmap;
    char text[] = "2PACMPEG - 2PAC 4 LYFE!!!!!!!!!!!!!!!!!!!!!!";
    ImVec2 tsize = ImGui::CalcTextSize(text);
    ImVec2 pos = ImVec2((rt_vars->win_width/2) - (tsize.x/2),
                        ((rt_vars->win_height/2) - (tsize.y/2)) - 100);
    ImGui::SetCursorPos(pos);
    ImGui::Text(text);

    ImGui::SetCursorPosX((rt_vars->win_width/2) - (lb->width/2));
    ImGui::Image((ImTextureID)(intptr_t)lb->ogl_tex_id,
            ImVec2(lb->width, lb->height),
            ImVec2(0, 0),
            ImVec2(1, 1),
            ImVec4(1, 1, 1, 1),
            ImVec4(0, 0, 0, 0));
}
