/* nuklear - 1.32.0 - public domain */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>
#include <time.h>

#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear.h"
#include "nuklear_glfw_gl3.h"

#define WINDOW_WIDTH 600
#define WINDOW_HEIGHT 650

#define MAX_VERTEX_BUFFER 512 * 1024
#define MAX_ELEMENT_BUFFER 128 * 1024

#define TASKS_NUMBER 16

// Muestra el ejemplo de todos los widgets
int show_overview = 0;

typedef struct _task
{
    int empty;
    nk_bool completed;
    char text[512];
} Task;

Task *tareas;

int tasks_number = TASKS_NUMBER;

int add_note(Task* tasks, char* text)
{
    for (int i = 0; i < tasks_number; i++)
    {
        if (tasks[i].empty)
        {
            tasks[i].empty = 0;
            tasks[i].completed = 0;
            
            strncpy(tasks[i].text, text, 500);

            return 1;
        }
    }

    return 0;
}



/* ===============================================================
 *
 *                          EXAMPLE
 *
 * ===============================================================*/
/* This are some code examples to provide a small overview of what can be
 * done with this library. To try out an example uncomment the defines */
//#define INCLUDE_ALL
#define INCLUDE_STYLE
/*#define INCLUDE_CALCULATOR */
/*#define INCLUDE_CANVAS */
#define INCLUDE_OVERVIEW
/*#define INCLUDE_NODE_EDITOR */

#ifdef INCLUDE_ALL
  #define INCLUDE_STYLE
  #define INCLUDE_CALCULATOR
  #define INCLUDE_CANVAS
  #define INCLUDE_OVERVIEW
  #define INCLUDE_NODE_EDITOR
#endif

#ifdef INCLUDE_STYLE
#include <demo/common/style.c>
#endif
#ifdef INCLUDE_CALCULATOR
#include <demo/common/calculator.c>
#endif
#ifdef INCLUDE_CANVAS
#include <demo/common/canvas.c>
#endif
#ifdef INCLUDE_OVERVIEW
#include <demo/common/overview.c>
#endif
#ifdef INCLUDE_NODE_EDITOR
#include <demo/common/node_editor.c>
#endif


void draw_list(struct nk_context* ctx);

static void error_callback(int e, const char *d)
{printf("Error %d: %s\n", e, d);}

#if 1
int main(void)
{
    /* Platform */
    struct nk_glfw glfw = { 0 };
    static GLFWwindow* win;
    int width = 0, height = 0;
    struct nk_context* ctx;
    struct nk_colorf bg;

    /* GLFW */
    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
        fprintf(stdout, "[GFLW] failed to init!\n");
        exit(1);
    }

    // NOTE: Mi PC solo soporta hasta OpenGL 3.1 (Deberia funcionar con 3.0 hacia arriba)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    win = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Demo Nuklear - Lista de tareas ", NULL, NULL);
    glfwMakeContextCurrent(win);
    glfwGetWindowSize(win, &width, &height);

    /* OpenGL */
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    glewExperimental = 1;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to setup GLEW\n");
        exit(1);
    }

    ctx = nk_glfw3_init(&glfw, win, NK_GLFW3_INSTALL_CALLBACKS);
    /* Load Fonts: if none of these are loaded a default font will be used  */
    /* Load Cursor: if you uncomment cursor loading please hide the cursor */
    //{
    struct nk_font_atlas* atlas;
    nk_glfw3_font_stash_begin(&glfw, &atlas);
#if defined(_DEBUG)
    struct nk_font* droid = nk_font_atlas_add_from_file(atlas, "../fonts/DroidSans.ttf", 48, 0);
    struct nk_font* roboto = nk_font_atlas_add_from_file(atlas, "../fonts/Roboto-Regular.ttf", 24, 0);
#endif

#if defined(NDEBUG)
    struct nk_font* droid = nk_font_atlas_add_from_file(atlas, "fonts/DroidSans.ttf", 48, 0);
    struct nk_font* roboto = nk_font_atlas_add_from_file(atlas, "fonts/Roboto-Regular.ttf", 24, 0);
#endif

    // NOTE: Si no se cargan las fuentes da errores extraños en otra parte nada que ver.
    // NOTE: Pase todo 1 dia averiguando porque crasheaba en Release y no Debug, era que no cargaba las fuentes
    if (!droid || !roboto)
    {
        fprintf(stderr, "Error al abrir las fuentes\n");
        exit(1);
    }

    /*struct nk_font *future = nk_font_atlas_add_from_file(atlas, "../../../extra_font/kenvector_future_thin.ttf", 13, 0);*/
    /*struct nk_font *clean = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyClean.ttf", 12, 0);*/
    /*struct nk_font *tiny = nk_font_atlas_add_from_file(atlas, "../../../extra_font/ProggyTiny.ttf", 10, 0);*/
    /*struct nk_font *cousine = nk_font_atlas_add_from_file(atlas, "../../../extra_font/Cousine-Regular.ttf", 13, 0);*/
    nk_glfw3_font_stash_end(&glfw);
    /*nk_style_load_all_cursors(ctx, atlas->cursors);*/
    nk_style_set_font(ctx, &roboto->handle);
    //}

#ifdef INCLUDE_STYLE
//set_style(ctx, THEME_WHITE);
/*set_style(ctx, THEME_RED);*/
/*set_style(ctx, THEME_BLUE);*/
//set_style(ctx, THEME_DARK);
#endif

    // Init tasks structures
    if (tareas != NULL)
    {
        fprintf(stderr, "Lista ya inicializada\n");
        free(tareas);

        return;
    }

    tareas = (Task*)malloc(sizeof(Task) * TASKS_NUMBER);

    if (!tareas)
    {
        fprintf(stderr, "Error al reservar memoria para la lista\n");
        return;
    }

    for (int i = 0; i < TASKS_NUMBER; i++)
    {
        tareas[i].empty = 1;
        tareas[i].completed = nk_false;
        tareas[i].text[0] = '\0';
    }



    bg.r = 0.10f, bg.g = 0.18f, bg.b = 0.24f, bg.a = 1.0f;
    while (!glfwWindowShouldClose(win))
    {
        /* Input */
        glfwPollEvents();
        nk_glfw3_new_frame(&glfw);

        /* GUI */
        struct nk_rect window_rect = nk_rect(0, 0, width, height);
        if (nk_begin(ctx, "Lista", window_rect, 0))
        {
            char buffer[64];

            nk_layout_row_static(ctx, 25, 200, 1);
            if (nk_button_label(ctx, "Overview"))
                show_overview = !show_overview;

            // Tiempo combo box
            {
                static int time_selected = 0;
                static int date_selected = 0;
                static struct tm sel_time;
                static struct tm sel_date;
                if (!time_selected || !date_selected) {
                    /* keep time and date updated if nothing is selected */
                    time_t cur_time = time(0);
                    struct tm* n = localtime(&cur_time);
                    if (!time_selected)
                        memcpy(&sel_time, n, sizeof(struct tm));
                    if (!date_selected)
                        memcpy(&sel_date, n, sizeof(struct tm));
                }

                nk_layout_row_static(ctx, 25, 200, 1);
                /* time combobox */
                sprintf(buffer, "%02d:%02d:%02d", sel_time.tm_hour, sel_time.tm_min, sel_time.tm_sec);
#if 0
                if (nk_combo_begin_label(ctx, buffer, nk_vec2(200, 250))) {
                    time_selected = 1;
                    nk_layout_row_dynamic(ctx, 25, 1);
                    sel_time.tm_sec = nk_propertyi(ctx, "#S:", 0, sel_time.tm_sec, 60, 1, 1);
                    sel_time.tm_min = nk_propertyi(ctx, "#M:", 0, sel_time.tm_min, 60, 1, 1);
                    sel_time.tm_hour = nk_propertyi(ctx, "#H:", 0, sel_time.tm_hour, 23, 1, 1);
                    nk_combo_end(ctx);
            }
#endif

        }


            // Fin del tiempo

            // Draw time
            nk_style_set_font(ctx, &droid->handle);
            nk_layout_row_dynamic(ctx, 100, 1);
            nk_label(ctx, buffer, NK_TEXT_CENTERED);
            nk_style_set_font(ctx, &roboto->handle);

            // Dibuja el input de tarea
            nk_layout_row_dynamic(ctx, 38, 1);
            //nk_label(ctx, "Ingresa una tarea:", NK_TEXT_LEFT);
            static char text[512] = { 0 };
            static int text_len[9];
            nk_edit_string(ctx, NK_EDIT_SIMPLE, text, &text_len[0], 500, nk_filter_default);

            nk_layout_row_dynamic(ctx, 30, 1);
            if (nk_button_label(ctx, "+"))
                //if (nk_button_label(ctx, "+ Agregar tarea"))
            {
                fprintf(stdout, "button pressed\n");
                if (text[0])
                    add_note(tareas, text);
            }

            nk_layout_row_dynamic(ctx, 30, 1);

            // Dibujar la lista
            draw_list(ctx);

    }
        nk_end(ctx);

        /* -------------- EXAMPLES ---------------- */
#ifdef INCLUDE_CALCULATOR
        calculator(ctx);
#endif
#ifdef INCLUDE_CANVAS
        canvas(ctx);
#endif
#ifdef INCLUDE_OVERVIEW
        if (show_overview)
            overview(ctx);
#endif
#ifdef INCLUDE_NODE_EDITOR
        node_editor(ctx);
#endif
        /* ----------------------------------------- */

        /* Draw */
        glfwGetWindowSize(win, &width, &height);
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glClearColor(bg.r, bg.g, bg.b, bg.a);
        /* IMPORTANT: `nk_glfw_render` modifies some global OpenGL state
         * with blending, scissor, face culling, depth test and viewport and
         * defaults everything back into a default state.
         * Make sure to either a.) save and restore or b.) reset your own state after
         * rendering the UI. */
        nk_glfw3_render(&glfw, NK_ANTI_ALIASING_ON, MAX_VERTEX_BUFFER, MAX_ELEMENT_BUFFER);
        glfwSwapBuffers(win);
}
    nk_glfw3_shutdown(&glfw);
    glfwTerminate();
    return 0;
}
#endif // 0


void draw_list(struct nk_context* ctx)
{
    static int group_titlebar = nk_false;
    static int group_border = nk_true;
    static int group_no_scrollbar = nk_false;
    static int group_width = 320;
    static int group_height = 380;
    static int active = 0;

    nk_flags group_flags = 0;
    if (group_border) group_flags |= NK_WINDOW_BORDER;
    if (group_no_scrollbar) group_flags |= NK_WINDOW_NO_SCROLLBAR;
    if (group_titlebar) group_flags |= NK_WINDOW_TITLE;

    //nk_layout_row_dynamic(ctx, 30, 3);
    //nk_checkbox_label(ctx, "Titlebar", &group_titlebar);
    //nk_checkbox_label(ctx, "Border", &group_border);
    //nk_checkbox_label(ctx, "No Scrollbar", &group_no_scrollbar);

    /*
    nk_layout_row_begin(ctx, NK_STATIC, 22, 3);
    nk_layout_row_push(ctx, 50);
    nk_label(ctx, "size:", NK_TEXT_LEFT);
    nk_layout_row_push(ctx, 130);
    nk_property_int(ctx, "#Width:", 100, &group_width, 500, 10, 1);
    nk_layout_row_push(ctx, 130);
    nk_property_int(ctx, "#Height:", 100, &group_height, 500, 10, 1);
    nk_layout_row_end(ctx);
    */

    //nk_layout_row_static(ctx, (float)group_height, group_width, 2);
    nk_layout_row_dynamic(ctx, (float)group_height, 1);
    if (nk_group_begin(ctx, "Tareas", group_flags)) {
        int i = 0;
        static int selected[16];
        //nk_layout_row_static(ctx, 18, 250, 1);
        nk_layout_row_dynamic(ctx, 30, 1);

        for (i = 0; i < tasks_number; ++i)
        {
         //nk_selectable_label(ctx, (selected[i]) ? "Selected" : "Unselected", NK_TEXT_CENTERED, &selected[i]);
            if (!tareas[i].empty)
                nk_checkbox_label(ctx, tareas[i].text, &tareas[i].completed);
        }
        nk_group_end(ctx);
    }
}


#if 0
void show_date_combo(struct nk_context ctxt)
{
    nk_layout_row_static(ctx, 25, 200, 1);
    /* date combobox */
    sprintf(buffer, "%02d-%02d-%02d", sel_date.tm_mday, sel_date.tm_mon + 1, sel_date.tm_year + 1900);
    if (nk_combo_begin_label(ctx, buffer, nk_vec2(350, 400)))
    {
        int i = 0;
        const char* month[] = { "Enero", "Febrero", "Marzo",
            "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre",
            "Octubre", "Noviembre", "Diciembre" };
        const char* week_days[] = { "DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB" };
        const int month_days[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
        int year = sel_date.tm_year + 1900;
        int leap_year = (!(year % 4) && ((year % 100))) || !(year % 400);
        int days = (sel_date.tm_mon == 1) ?
            month_days[sel_date.tm_mon] + leap_year :
            month_days[sel_date.tm_mon];

        /* header with month and year */
        date_selected = 1;
        nk_layout_row_begin(ctx, NK_DYNAMIC, 20, 3);
        nk_layout_row_push(ctx, 0.05f);
        if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT)) {
            if (sel_date.tm_mon == 0) {
                sel_date.tm_mon = 11;
                sel_date.tm_year = NK_MAX(0, sel_date.tm_year - 1);
            }
            else sel_date.tm_mon--;
        }
        nk_layout_row_push(ctx, 0.9f);
        sprintf(buffer, "%s %d", month[sel_date.tm_mon], year);
        nk_label(ctx, buffer, NK_TEXT_CENTERED);
        nk_layout_row_push(ctx, 0.05f);
        if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT)) {
            if (sel_date.tm_mon == 11) {
                sel_date.tm_mon = 0;
                sel_date.tm_year++;
            }
            else sel_date.tm_mon++;
        }
        nk_layout_row_end(ctx);

        /* good old week day formula (double because precision) */
        {int year_n = (sel_date.tm_mon < 2) ? year - 1 : year;
        int y = year_n % 100;
        int c = year_n / 100;
        int y4 = (int)((float)y / 4);
        int c4 = (int)((float)c / 4);
        int m = (int)(2.6 * (double)(((sel_date.tm_mon + 10) % 12) + 1) - 0.2);
        int week_day = (((1 + m + y + y4 + c4 - 2 * c) % 7) + 7) % 7;

        /* weekdays  */
        nk_layout_row_dynamic(ctx, 35, 7);
        for (i = 0; i < (int)NK_LEN(week_days); ++i)
            nk_label(ctx, week_days[i], NK_TEXT_CENTERED);

        /* days  */
        if (week_day > 0) nk_spacing(ctx, week_day);
        for (i = 1; i <= days; ++i) {
            sprintf(buffer, "%d", i);
            if (nk_button_label(ctx, buffer)) {
                sel_date.tm_mday = i;
                nk_combo_close(ctx);
            }
        }}
        nk_combo_end(ctx);
    }
}
#endif