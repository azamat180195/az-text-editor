/*
 * AZ Editor - Professional text editor for Linux
 * Ultra-light, fast, and feature-rich
 * Similar to nano but better
 * 
 * Compile: gcc az.c -o az -lncurses
 * Usage: ./az [filename]
 */

#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <signal.h>
#include <ctype.h>
#include <unistd.h>
#include <stdarg.h>
#include <time.h>

#define VERSION "1.8.0"
#define TAB_SIZE 4
#define MAX_UNDO 100
#define LINE_NUMBER_WIDTH 5
#define DEBUG_LOG "/tmp/az_debug.log"

/* Debug logging */
FILE *debug_fp = NULL;

void debug_log(const char *fmt, ...) {
    if (!debug_fp) {
        debug_fp = fopen(DEBUG_LOG, "a");
    }
    if (debug_fp) {
        va_list args;
        va_start(args, fmt);
        time_t now = time(NULL);
        fprintf(debug_fp, "[%ld] ", now);
        vfprintf(debug_fp, fmt, args);
        fprintf(debug_fp, "\n");
        fflush(debug_fp);
        va_end(args);
    }
}

/* Line structure */
typedef struct Line {
    char *data;
    size_t len;
    size_t capacity;
    struct Line *next;
    struct Line *prev;
} Line;

/* Syntax error info */
typedef struct {
    int line;       /* Error line number (1-based) */
    int col_start;  /* Error start column (0-based) */
    int col_end;    /* Error end column (0-based) */
    char msg[128];  /* Error message */
} SyntaxError;

/* Editor state */
typedef struct {
    Line *first_line;
    Line *current_line;
    int cursor_x;
    int cursor_y;
    int preferred_x;  /* For PageUp/Down */
    int offset_y;
    int screen_width;
    int screen_height;
    int edit_width;
    int edit_height;
    char *filename;
    int modified;
    int total_lines;
    char message[256];
    int message_timeout;
    
    /* Syntax error */
    SyntaxError syntax_error;
    
    /* Selection */
    int sel_active;
    int sel_start_y, sel_start_x;
    int sel_end_y, sel_end_x;
    
    /* Mouse */
    int mouse_pressed;
    
    /* Clipboard */
    char **clipboard;
    int clipboard_lines;
    
    /* Cut buffer (nano-style) */
    char **cut_buffer;
    int cut_buffer_lines;
    
    /* Undo/Redo - properly implemented */
    struct {
        char **lines;
        int num_lines;
        int cursor_x;
        int cursor_y;
    } undo_stack[MAX_UNDO];
    int undo_count;
    
    struct {
        char **lines;
        int num_lines;
        int cursor_x;
        int cursor_y;
    } redo_stack[MAX_UNDO];
    int redo_count;
} Editor;

/* Function declarations */
void init_editor(Editor *ed, const char *filename);
void cleanup_editor(Editor *ed);
void load_file(Editor *ed, const char *filename);
void save_file(Editor *ed);
void draw_screen(Editor *ed);
void handle_input(Editor *ed, int ch);
void insert_char(Editor *ed, char c);
void delete_char(Editor *ed);
void backspace_char(Editor *ed);
void insert_newline(Editor *ed);
void move_cursor(Editor *ed, int dy, int dx);
void page_up(Editor *ed);
void page_down(Editor *ed);
void set_message(Editor *ed, const char *msg);
void copy_selection(Editor *ed);
void cut_selection(Editor *ed);
void paste_clipboard(Editor *ed);
void cut_line(Editor *ed);
void uncut_text(Editor *ed);
void select_all(Editor *ed);
void delete_selection(Editor *ed);
void handle_mouse(Editor *ed);
Line* get_line_at(Editor *ed, int y);
void check_syntax_error(Editor *ed);
void handle_tab(Editor *ed);
void search_text(Editor *ed);
void replace_text(Editor *ed);
void save_undo(Editor *ed);
void perform_undo(Editor *ed);
char* safe_strndup(const char *s, size_t n);

/* Safe string duplicate with length limit */
char* safe_strndup(const char *s, size_t n) {
    size_t len = strlen(s);
    if (len > n) len = n;
    char *result = malloc(len + 1);
    if (result) {
        memcpy(result, s, len);
        result[len] = '\0';
    }
    return result;
}

/* Initialize editor */
void init_editor(Editor *ed, const char *filename) {
    debug_log("=== AZ Editor Started ===");
    debug_log("Filename: %s", filename ? filename : "NULL");
    memset(ed, 0, sizeof(Editor));
    
    /* Create first line */
    ed->first_line = calloc(1, sizeof(Line));
    ed->first_line->capacity = 128;
    ed->first_line->data = calloc(ed->first_line->capacity, 1);
    ed->current_line = ed->first_line;
    ed->total_lines = 1;
    
    if (filename) {
        ed->filename = strdup(filename);
        load_file(ed, filename);
    }
    
    /* Initialize ncurses */
    initscr();
    debug_log("ncurses initialized");
    
    raw();  /* Use raw() to get ALL keys including Ctrl */
    debug_log("raw() called");
    
    keypad(stdscr, TRUE);
    debug_log("keypad enabled");
    
    noecho();
    debug_log("noecho set");
    
    /* Disable flow control so Ctrl+S/Q work */
    intrflush(stdscr, FALSE);
    debug_log("intrflush disabled");
    
    timeout(50);
    debug_log("timeout set to 50ms");
    
    /* Colors */
    if (has_colors()) {
        start_color();
        use_default_colors();
        init_pair(1, COLOR_CYAN, -1);     /* Line numbers */
        init_pair(2, COLOR_GREEN, -1);    /* Comments */
        init_pair(3, COLOR_YELLOW, -1);   /* Strings */
        init_pair(4, COLOR_MAGENTA, -1);  /* Keywords */
        init_pair(5, COLOR_BLACK, COLOR_WHITE);  /* Selection */
        init_pair(6, COLOR_WHITE, COLOR_BLUE);   /* Status bar */
        init_pair(7, COLOR_RED, -1);      /* Errors */
    }
    
    /* Mouse support */
    mousemask(ALL_MOUSE_EVENTS | REPORT_MOUSE_POSITION, NULL);
    mouseinterval(0);
    
    /* Enable mouse motion events in terminal */
    printf("\033[?1003h");  /* Enable any-event mouse tracking */
    fflush(stdout);
    
    /* Get screen size */
    getmaxyx(stdscr, ed->screen_height, ed->screen_width);
    ed->edit_height = ed->screen_height - 2;
    ed->edit_width = ed->screen_width - LINE_NUMBER_WIDTH - 1;
    
    /* We need to ignore signals for Ctrl keys to work */
    signal(SIGINT, SIG_IGN);   /* Ignore Ctrl+C */
    signal(SIGTSTP, SIG_IGN);  /* Ignore Ctrl+Z */
    signal(SIGQUIT, SIG_IGN);  /* Ignore Ctrl+\ */
    debug_log("Signals ignored");
}

/* Load file */
void load_file(Editor *ed, const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) return;
    
    /* Free existing lines */
    Line *line = ed->first_line;
    while (line) {
        Line *next = line->next;
        free(line->data);
        free(line);
        line = next;
    }
    
    ed->first_line = NULL;
    ed->current_line = NULL;
    ed->total_lines = 0;
    
    char buffer[8192];
    Line *last_line = NULL;
    
    while (fgets(buffer, sizeof(buffer), f)) {
        /* Remove newline */
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') {
            buffer[len-1] = '\0';
            len--;
        }
        if (len > 0 && buffer[len-1] == '\r') {
            buffer[len-1] = '\0';
            len--;
        }
        
        /* Create new line */
        Line *new_line = calloc(1, sizeof(Line));
        new_line->capacity = len + 128;
        new_line->data = malloc(new_line->capacity);
        strcpy(new_line->data, buffer);
        new_line->len = len;
        
        if (!ed->first_line) {
            ed->first_line = new_line;
            ed->current_line = new_line;
        } else {
            last_line->next = new_line;
            new_line->prev = last_line;
        }
        last_line = new_line;
        ed->total_lines++;
    }
    
    fclose(f);
    
    if (!ed->first_line) {
        ed->first_line = calloc(1, sizeof(Line));
        ed->first_line->capacity = 128;
        ed->first_line->data = calloc(ed->first_line->capacity, 1);
        ed->current_line = ed->first_line;
        ed->total_lines = 1;
    }
}

/* Save file */
void save_file(Editor *ed) {
    if (!ed->filename) {
        /* Prompt for filename */
        echo();
        curs_set(2);
        mvprintw(ed->screen_height - 1, 0, "Fayl nomi: ");
        clrtoeol();
        refresh();
        
        char filename[256];
        getnstr(filename, sizeof(filename) - 1);
        
        noecho();
        curs_set(1);
        
        if (strlen(filename) > 0) {
            ed->filename = strdup(filename);
        } else {
            set_message(ed, "Cancelled");
            return;
        }
    }
    
    FILE *f = fopen(ed->filename, "w");
    if (!f) {
        set_message(ed, "Error: Cannot open file");
        return;
    }
    
    Line *line = ed->first_line;
    while (line) {
        fprintf(f, "%s\n", line->data);
        line = line->next;
    }
    
    fclose(f);
    ed->modified = 0;
    
    char msg[256];
    snprintf(msg, sizeof(msg), "Saqlandi: %s", ed->filename);
    set_message(ed, msg);
}

/* Get line at position */
Line* get_line_at(Editor *ed, int y) {
    Line *line = ed->first_line;
    for (int i = 0; i < y && line; i++) {
        line = line->next;
    }
    return line;
}

/* Check syntax errors for all file types - with detailed error info */
void check_syntax_error(Editor *ed) {
    /* Clear previous error */
    ed->syntax_error.line = 0;
    ed->syntax_error.col_start = 0;
    ed->syntax_error.col_end = 0;
    ed->syntax_error.msg[0] = '\0';
    
    if (!ed->filename) return;
    
    const char *ext = strrchr(ed->filename, '.');
    if (!ext) return;
    
    Line *line = ed->first_line;
    int line_num = 1;
    
    /* JSON validation */
    if (strcmp(ext, ".json") == 0) {
        int brace_count = 0, bracket_count = 0;
        int in_string = 0;
        
        line = ed->first_line;
        line_num = 1;
        while (line) {
            for (size_t i = 0; i < line->len; i++) {
                char c = line->data[i];
                if (c == '"' && (i == 0 || line->data[i-1] != '\\')) {
                    in_string = !in_string;
                }
                if (!in_string) {
                    if (c == '{') brace_count++;
                    if (c == '}') {
                        brace_count--;
                        if (brace_count < 0) {
                            ed->syntax_error.line = line_num;
                            ed->syntax_error.col_start = i;
                            ed->syntax_error.col_end = i + 1;
                            snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                                    "Extra '}' - no opening brace");
                            return;
                        }
                    }
                    if (c == '[') bracket_count++;
                    if (c == ']') {
                        bracket_count--;
                        if (bracket_count < 0) {
                            ed->syntax_error.line = line_num;
                            ed->syntax_error.col_start = i;
                            ed->syntax_error.col_end = i + 1;
                            snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                                    "Extra ']' - no opening bracket");
                            return;
                        }
                    }
                }
            }
            line = line->next;
            line_num++;
        }
        if (brace_count != 0) {
            ed->syntax_error.line = line_num - 1;
            ed->syntax_error.col_start = 0;
            ed->syntax_error.col_end = 0;
            snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                    "Unclosed '{' - %d open brace(s)", brace_count);
            return;
        }
        if (bracket_count != 0) {
            ed->syntax_error.line = line_num - 1;
            ed->syntax_error.col_start = 0;
            ed->syntax_error.col_end = 0;
            snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                    "Unclosed '[' - %d open bracket(s)", bracket_count);
            return;
        }
    }
    
    /* YAML validation */
    if (strcmp(ext, ".yml") == 0 || strcmp(ext, ".yaml") == 0) {
        line = ed->first_line;
        line_num = 1;
        while (line) {
            /* Check for tab characters (YAML doesn't allow tabs) */
            for (size_t i = 0; i < line->len; i++) {
                if (line->data[i] == '\t') {
                    ed->syntax_error.line = line_num;
                    ed->syntax_error.col_start = i;
                    ed->syntax_error.col_end = i + 1;
                    snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                            "YAML: TAB not allowed - use spaces");
                    return;
                }
            }
            /* Check for unbalanced brackets */
            int brace = 0, bracket = 0;
            for (size_t i = 0; i < line->len; i++) {
                if (line->data[i] == '{') brace++;
                if (line->data[i] == '}') brace--;
                if (line->data[i] == '[') bracket++;
                if (line->data[i] == ']') bracket--;
                if (brace < 0 || bracket < 0) {
                    ed->syntax_error.line = line_num;
                    ed->syntax_error.col_start = i;
                    ed->syntax_error.col_end = i + 1;
                    snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                            "Bracket balance broken");
                    return;
                }
            }
            if (brace != 0 || bracket != 0) {
                ed->syntax_error.line = line_num;
                ed->syntax_error.col_start = 0;
                ed->syntax_error.col_end = line->len;
                snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                        "Bracket not closed");
                return;
            }
            line = line->next;
            line_num++;
        }
    }
    
    /* Python validation */
    if (strcmp(ext, ".py") == 0) {
        line = ed->first_line;
        line_num = 1;
        while (line) {
            /* Check for basic indentation errors (mixing tabs/spaces) */
            int has_tab = 0, has_space = 0;
            int first_tab = -1, first_space = -1;
            for (size_t i = 0; i < line->len && (line->data[i] == ' ' || line->data[i] == '\t'); i++) {
                if (line->data[i] == '\t') {
                    has_tab = 1;
                    if (first_tab == -1) first_tab = i;
                }
                if (line->data[i] == ' ') {
                    has_space = 1;
                    if (first_space == -1) first_space = i;
                }
            }
            if (has_tab && has_space) {
                int error_pos = (first_tab > first_space) ? first_tab : first_space;
                ed->syntax_error.line = line_num;
                ed->syntax_error.col_start = error_pos;
                ed->syntax_error.col_end = error_pos + 1;
                snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                        "Mixed TAB and spaces - use one");
                return;
            }
            line = line->next;
            line_num++;
        }
    }
    
    /* HTML/XML validation */
    if (strcmp(ext, ".html") == 0 || strcmp(ext, ".xml") == 0 || strcmp(ext, ".htm") == 0) {
        int tag_depth = 0;
        line = ed->first_line;
        line_num = 1;
        while (line) {
            for (size_t i = 0; i < line->len; i++) {
                if (line->data[i] == '<' && i + 1 < line->len) {
                    if (line->data[i+1] != '/' && line->data[i+1] != '!' && line->data[i+1] != '?') {
                        /* Opening tag */
                        tag_depth++;
                    } else if (line->data[i+1] == '/') {
                        /* Closing tag */
                        tag_depth--;
                        if (tag_depth < 0) {
                            ed->syntax_error.line = line_num;
                            ed->syntax_error.col_start = i;
                            ed->syntax_error.col_end = i + 2;
                            snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                                    "Extra closing tag - no opening tag");
                            return;
                        }
                    }
                }
            }
            line = line->next;
            line_num++;
        }
        if (tag_depth != 0) {
            ed->syntax_error.line = line_num - 1;
            ed->syntax_error.col_start = 0;
            ed->syntax_error.col_end = 0;
            snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                    "Unclosed tag - %d open tag(s)", tag_depth);
            return;
        }
    }
    
    /* Java/C/C++/Go validation */
    if (strcmp(ext, ".java") == 0 || strcmp(ext, ".c") == 0 || 
        strcmp(ext, ".cpp") == 0 || strcmp(ext, ".go") == 0 ||
        strcmp(ext, ".h") == 0 || strcmp(ext, ".hpp") == 0) {
        int brace_count = 0;
        int in_string = 0, in_comment = 0;
        line = ed->first_line;
        line_num = 1;
        while (line) {
            for (size_t i = 0; i < line->len; i++) {
                char c = line->data[i];
                /* Skip strings */
                if (c == '"' && (i == 0 || line->data[i-1] != '\\')) {
                    in_string = !in_string;
                }
                if (!in_string) {
                    /* Skip comments */
                    if (c == '/' && i + 1 < line->len && line->data[i+1] == '/') break;
                    if (c == '/' && i + 1 < line->len && line->data[i+1] == '*') in_comment = 1;
                    if (c == '*' && i + 1 < line->len && line->data[i+1] == '/') in_comment = 0;
                    
                    if (!in_comment) {
                        if (c == '{') brace_count++;
                        if (c == '}') {
                            brace_count--;
                            if (brace_count < 0) {
                                ed->syntax_error.line = line_num;
                                ed->syntax_error.col_start = i;
                                ed->syntax_error.col_end = i + 1;
                                snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                                        "Ortiqcha '}' - ochilmagan jingalak qavs");
                                return;
                            }
                        }
                    }
                }
            }
            line = line->next;
            line_num++;
        }
        if (brace_count != 0) {
            ed->syntax_error.line = line_num - 1;
            ed->syntax_error.col_start = 0;
            ed->syntax_error.col_end = 0;
            snprintf(ed->syntax_error.msg, sizeof(ed->syntax_error.msg), 
                    "Unclosed '{' - %d open brace(s)", brace_count);
            return;
        }
    }
}

/* Set message */
void set_message(Editor *ed, const char *msg) {
    strncpy(ed->message, msg, sizeof(ed->message) - 1);
    ed->message_timeout = 4;  /* 4 frames ~= 200ms */
}

/* Draw screen */
void draw_screen(Editor *ed) {
    erase();
    
    /* Draw text area with word wrap */
    int screen_row = 0;
    Line *line = get_line_at(ed, ed->offset_y);
    int line_num = ed->offset_y;
    
    while (screen_row < ed->edit_height && line) {
        /* Calculate wrapped lines */
        int line_len = line->len;
        int wraps = (line_len + ed->edit_width - 1) / ed->edit_width;
        if (wraps < 1) wraps = 1;
        
        for (int wrap = 0; wrap < wraps && screen_row < ed->edit_height; wrap++) {
            /* Line number (only on first wrap) */
            if (wrap == 0) {
                attron(COLOR_PAIR(1) | A_BOLD);
                mvprintw(screen_row, 0, "%4d ", line_num + 1);
                attroff(COLOR_PAIR(1) | A_BOLD);
            } else {
                mvprintw(screen_row, 0, "     ");
            }
            
            /* Text */
            int start = wrap * ed->edit_width;
            int end = start + ed->edit_width;
            if (end > line_len) end = line_len;
            
            for (int i = start; i < end; i++) {
                int is_selected = 0;
                int is_error = 0;
                
                /* Check if this character is selected */
                if (ed->sel_active) {
                    int sy = ed->sel_start_y, sx = ed->sel_start_x;
                    int ey = ed->sel_end_y, ex = ed->sel_end_x;
                    
                    /* Normalize selection */
                    if (sy > ey || (sy == ey && sx > ex)) {
                        int tmp;
                        tmp = sy; sy = ey; ey = tmp;
                        tmp = sx; sx = ex; ex = tmp;
                    }
                    
                    if (line_num >= sy && line_num <= ey) {
                        if (sy == ey) {
                            is_selected = (i >= sx && i < ex);
                        } else if (line_num == sy) {
                            is_selected = (i >= sx);
                        } else if (line_num == ey) {
                            is_selected = (i < ex);
                        } else {
                            is_selected = 1;
                        }
                    }
                }
                
                /* Check if this character is in error position */
                if (ed->syntax_error.line > 0 && (line_num + 1) == ed->syntax_error.line) {
                    if (ed->syntax_error.col_end > ed->syntax_error.col_start) {
                        /* Specific position error */
                        if (i >= ed->syntax_error.col_start && i < ed->syntax_error.col_end) {
                            is_error = 1;
                        }
                    } else {
                        /* Whole line error - highlight first 10 chars or whole line */
                        if (i < 10 || i < (int)line->len) {
                            is_error = 1;
                        }
                    }
                }
                
                /* Apply highlighting */
                if (is_error) {
                    attron(COLOR_PAIR(3) | A_UNDERLINE);  /* Red underline */
                } else if (is_selected) {
                    attron(COLOR_PAIR(5));
                }
                
                mvaddch(screen_row, LINE_NUMBER_WIDTH + (i - start), line->data[i]);
                
                /* Remove highlighting */
                if (is_error) {
                    attroff(COLOR_PAIR(3) | A_UNDERLINE);
                } else if (is_selected) {
                    attroff(COLOR_PAIR(5));
                }
            }
            
            screen_row++;
        }
        
        line = line->next;
        line_num++;
    }
    
    /* Status bar */
    int status_line = ed->screen_height - 2;
    attron(COLOR_PAIR(6) | A_BOLD);
    for (int i = 0; i < ed->screen_width; i++) {
        mvaddch(status_line, i, ' ');
    }
    
    char status_left[128];
    snprintf(status_left, sizeof(status_left), " %c %s ", 
             ed->modified ? '*' : ' ',
             ed->filename ? ed->filename : "New file");
    mvprintw(status_line, 0, "%s", status_left);
    
    /* Detailed position info with percentage */
    char status_center[80];
    int percent = (ed->total_lines > 0) ? ((ed->cursor_y + 1) * 100 / ed->total_lines) : 0;
    snprintf(status_center, sizeof(status_center), "Line %d/%d (%d%%), Col %d ",
             ed->cursor_y + 1, ed->total_lines, percent, ed->cursor_x + 1);
    int center_x = (ed->screen_width - strlen(status_center)) / 2;
    mvprintw(status_line, center_x, "%s", status_center);
    
    /* Check syntax errors - display in center-right */
    check_syntax_error(ed);
    if (ed->syntax_error.line > 0) {
        attron(COLOR_PAIR(3));  /* Red color */
        char error_display[100];
        snprintf(error_display, sizeof(error_display), "⚠ Q%d: %s", 
                 ed->syntax_error.line, ed->syntax_error.msg);
        
        /* Truncate if too long */
        int max_len = ed->screen_width / 2 - 2;  /* Use half screen */
        if ((int)strlen(error_display) > max_len && max_len > 15) {
            error_display[max_len - 3] = '.';
            error_display[max_len - 2] = '.';
            error_display[max_len - 1] = '.';
            error_display[max_len] = '\0';
        }
        
        /* Position in center-right area */
        int error_x = ed->screen_width / 2 + 5;
        if (error_x < 0) error_x = 0;
        if (error_x + (int)strlen(error_display) >= ed->screen_width) {
            error_x = ed->screen_width - strlen(error_display) - 1;
        }
        mvprintw(status_line, error_x, "%s", error_display);
        attroff(COLOR_PAIR(3));
    }
    
    attroff(COLOR_PAIR(6) | A_BOLD);
    
    /* Help line */
    int help_line = ed->screen_height - 1;
    if (ed->message_timeout > 0) {
        mvprintw(help_line, 0, "%s", ed->message);
        ed->message_timeout--;
    } else {
        mvprintw(help_line, 0, "^S:Save  ^Q:Quit  ^Z:Undo  ^F:Find  ^R:Replace  ^K:Cut  ^U:Paste  RClick:Paste");
    }
    
    /* Position cursor - with word wrap consideration */
    int cursor_screen_y = 0;
    Line *cline = ed->first_line;
    for (int i = 0; i < ed->offset_y && cline; i++) {
        cline = cline->next;
    }
    
    for (int i = ed->offset_y; i < ed->cursor_y && cline; i++) {
        int wraps = (cline->len + ed->edit_width - 1) / ed->edit_width;
        if (wraps < 1) wraps = 1;
        cursor_screen_y += wraps;
        cline = cline->next;
    }
    
    int cursor_wrap = ed->cursor_x / ed->edit_width;
    int cursor_wrap_x = ed->cursor_x % ed->edit_width;
    cursor_screen_y += cursor_wrap;
    
    if (cursor_screen_y < ed->edit_height) {
        move(cursor_screen_y, LINE_NUMBER_WIDTH + cursor_wrap_x);
    }
    
    refresh();
}

/* Save state for undo */
void save_undo(Editor *ed) {
    if (ed->undo_count >= MAX_UNDO) {
        /* Free oldest undo */
        for (int i = 0; i < ed->undo_stack[0].num_lines; i++) {
            free(ed->undo_stack[0].lines[i]);
        }
        free(ed->undo_stack[0].lines);
        
        /* Shift stack */
        for (int i = 0; i < MAX_UNDO - 1; i++) {
            ed->undo_stack[i] = ed->undo_stack[i + 1];
        }
        ed->undo_count--;
    }
    
    /* Save current state */
    ed->undo_stack[ed->undo_count].num_lines = ed->total_lines;
    ed->undo_stack[ed->undo_count].cursor_x = ed->cursor_x;
    ed->undo_stack[ed->undo_count].cursor_y = ed->cursor_y;
    ed->undo_stack[ed->undo_count].lines = malloc(sizeof(char*) * ed->total_lines);
    
    Line *line = ed->first_line;
    for (int i = 0; i < ed->total_lines && line; i++) {
        ed->undo_stack[ed->undo_count].lines[i] = strdup(line->data);
        line = line->next;
    }
    
    ed->undo_count++;
    debug_log("Undo saved: %d states", ed->undo_count);
}

/* Perform undo */
void perform_undo(Editor *ed) {
    if (ed->undo_count == 0) {
        set_message(ed, "Undo: Nothing to undo");
        debug_log("Undo: empty stack");
        return;
    }
    
    ed->undo_count--;
    
    /* Free current lines */
    Line *line = ed->first_line;
    while (line) {
        Line *next = line->next;
        free(line->data);
        free(line);
        line = next;
    }
    
    /* Restore from undo stack */
    ed->total_lines = ed->undo_stack[ed->undo_count].num_lines;
    ed->cursor_x = ed->undo_stack[ed->undo_count].cursor_x;
    ed->cursor_y = ed->undo_stack[ed->undo_count].cursor_y;
    ed->preferred_x = ed->cursor_x;
    
    ed->first_line = NULL;
    Line *last = NULL;
    
    for (int i = 0; i < ed->total_lines; i++) {
        Line *new_line = calloc(1, sizeof(Line));
        new_line->data = strdup(ed->undo_stack[ed->undo_count].lines[i]);
        new_line->len = strlen(new_line->data);
        new_line->capacity = new_line->len + 128;
        new_line->data = realloc(new_line->data, new_line->capacity);
        
        if (!ed->first_line) {
            ed->first_line = new_line;
        } else {
            last->next = new_line;
            new_line->prev = last;
        }
        last = new_line;
    }
    
    ed->current_line = get_line_at(ed, ed->cursor_y);
    ed->modified = 1;
    
    char msg[64];
    snprintf(msg, sizeof(msg), "Undo: %d ta orqaga", ed->undo_count);
    set_message(ed, msg);
    debug_log("Undo performed: now %d states", ed->undo_count);
}

/* Insert character */
void insert_char(Editor *ed, char c) {
    Line *line = ed->current_line;
    
    /* Save undo only at word boundaries */
    static int last_was_space = 1;
    if (last_was_space || ed->undo_count == 0) {
        save_undo(ed);
        last_was_space = 0;
    }
    if (c == ' ' || c == '\t') {
        last_was_space = 1;
    }
    
    /* Expand buffer if needed */
    if (line->len + 2 >= line->capacity) {
        line->capacity *= 2;
        line->data = realloc(line->data, line->capacity);
    }
    
    /* Delete selection first if active */
    if (ed->sel_active) {
        delete_selection(ed);
        line = ed->current_line;
    }
    
    /* Insert character */
    memmove(&line->data[ed->cursor_x + 1], &line->data[ed->cursor_x], 
            line->len - ed->cursor_x + 1);
    line->data[ed->cursor_x] = c;
    line->len++;
    ed->cursor_x++;
    ed->preferred_x = ed->cursor_x;
    ed->modified = 1;
    
    ed->sel_active = 0;
}

/* Handle tab */
void handle_tab(Editor *ed) {
    /* Delete selection first if active */
    if (ed->sel_active) {
        delete_selection(ed);
    }
    
    /* Insert spaces */
    for (int i = 0; i < TAB_SIZE; i++) {
        insert_char(ed, ' ');
    }
}

/* Backspace */
void backspace_char(Editor *ed) {
    save_undo(ed);
    
    if (ed->sel_active) {
        delete_selection(ed);
        return;
    }
    
    if (ed->cursor_x > 0) {
        Line *line = ed->current_line;
        memmove(&line->data[ed->cursor_x - 1], &line->data[ed->cursor_x],
                line->len - ed->cursor_x + 1);
        line->len--;
        ed->cursor_x--;
        ed->preferred_x = ed->cursor_x;
        ed->modified = 1;
    } else if (ed->cursor_y > 0) {
        /* Merge with previous line */
        Line *prev = ed->current_line->prev;
        int prev_len = prev->len;
        
        /* Expand previous line buffer */
        while (prev->len + ed->current_line->len + 1 >= prev->capacity) {
            prev->capacity *= 2;
            prev->data = realloc(prev->data, prev->capacity);
        }
        
        strcat(prev->data, ed->current_line->data);
        prev->len += ed->current_line->len;
        
        /* Remove current line */
        if (ed->current_line->next) {
            ed->current_line->next->prev = prev;
        }
        prev->next = ed->current_line->next;
        
        free(ed->current_line->data);
        free(ed->current_line);
        
        ed->current_line = prev;
        ed->cursor_y--;
        ed->cursor_x = prev_len;
        ed->preferred_x = ed->cursor_x;
        ed->total_lines--;
        ed->modified = 1;
    }
}

/* Delete character */
void delete_char(Editor *ed) {
    save_undo(ed);
    
    if (ed->sel_active) {
        delete_selection(ed);
        return;
    }
    
    Line *line = ed->current_line;
    
    if (ed->cursor_x < line->len) {
        memmove(&line->data[ed->cursor_x], &line->data[ed->cursor_x + 1],
                line->len - ed->cursor_x);
        line->len--;
        ed->modified = 1;
    } else if (line->next) {
        /* Merge with next line */
        Line *next = line->next;
        
        while (line->len + next->len + 1 >= line->capacity) {
            line->capacity *= 2;
            line->data = realloc(line->data, line->capacity);
        }
        
        strcat(line->data, next->data);
        line->len += next->len;
        
        line->next = next->next;
        if (next->next) {
            next->next->prev = line;
        }
        
        free(next->data);
        free(next);
        ed->total_lines--;
        ed->modified = 1;
    }
}

/* Insert newline */
void insert_newline(Editor *ed) {
    save_undo(ed);
    
    if (ed->sel_active) {
        delete_selection(ed);
    }
    
    Line *line = ed->current_line;
    Line *new_line = calloc(1, sizeof(Line));
    new_line->capacity = 128;
    new_line->data = malloc(new_line->capacity);
    
    /* Split line at cursor */
    strcpy(new_line->data, &line->data[ed->cursor_x]);
    new_line->len = line->len - ed->cursor_x;
    line->data[ed->cursor_x] = '\0';
    line->len = ed->cursor_x;
    
    /* Insert new line */
    new_line->next = line->next;
    new_line->prev = line;
    if (line->next) {
        line->next->prev = new_line;
    }
    line->next = new_line;
    
    ed->current_line = new_line;
    ed->cursor_y++;
    ed->cursor_x = 0;
    ed->preferred_x = 0;
    ed->total_lines++;
    ed->modified = 1;
}

/* Move cursor */
void move_cursor(Editor *ed, int dy, int dx) {
    if (dy < 0 && ed->cursor_y > 0) {
        ed->current_line = ed->current_line->prev;
        ed->cursor_y--;
        ed->cursor_x = ed->preferred_x;
        if (ed->cursor_x > ed->current_line->len) {
            ed->cursor_x = ed->current_line->len;
        }
    } else if (dy > 0 && ed->current_line->next) {
        ed->current_line = ed->current_line->next;
        ed->cursor_y++;
        ed->cursor_x = ed->preferred_x;
        if (ed->cursor_x > ed->current_line->len) {
            ed->cursor_x = ed->current_line->len;
        }
    }
    
    if (dx < 0 && ed->cursor_x > 0) {
        ed->cursor_x--;
        ed->preferred_x = ed->cursor_x;
    } else if (dx < 0 && ed->cursor_y > 0) {
        ed->current_line = ed->current_line->prev;
        ed->cursor_y--;
        ed->cursor_x = ed->current_line->len;
        ed->preferred_x = ed->cursor_x;
    }
    
    if (dx > 0 && ed->cursor_x < ed->current_line->len) {
        ed->cursor_x++;
        ed->preferred_x = ed->cursor_x;
    } else if (dx > 0 && ed->current_line->next) {
        ed->current_line = ed->current_line->next;
        ed->cursor_y++;
        ed->cursor_x = 0;
        ed->preferred_x = 0;
    }
    
    /* Adjust scroll */
    if (ed->cursor_y < ed->offset_y) {
        ed->offset_y = ed->cursor_y;
    }
    while (ed->cursor_y - ed->offset_y >= ed->edit_height) {
        ed->offset_y++;
    }
    
    ed->sel_active = 0;
}

/* Page Up */
void page_up(Editor *ed) {
    int move = ed->edit_height;
    for (int i = 0; i < move && ed->cursor_y > 0; i++) {
        if (ed->current_line->prev) {
            ed->current_line = ed->current_line->prev;
            ed->cursor_y--;
        }
    }
    
    /* Adjust scroll to keep cursor in view */
    if (ed->cursor_y < ed->offset_y) {
        ed->offset_y = ed->cursor_y;
    }
    
    /* Keep cursor_x within line bounds */
    ed->cursor_x = ed->preferred_x;
    if (ed->cursor_x > ed->current_line->len) {
        ed->cursor_x = ed->current_line->len;
    }
    ed->sel_active = 0;
}

/* Page Down */
void page_down(Editor *ed) {
    int move = ed->edit_height;
    for (int i = 0; i < move && ed->current_line->next; i++) {
        ed->current_line = ed->current_line->next;
        ed->cursor_y++;
    }
    
    /* Adjust scroll to keep cursor in view */
    while (ed->cursor_y - ed->offset_y >= ed->edit_height) {
        ed->offset_y++;
    }
    
    /* Keep cursor_x within line bounds */
    ed->cursor_x = ed->preferred_x;
    if (ed->cursor_x > ed->current_line->len) {
        ed->cursor_x = ed->current_line->len;
    }
    ed->sel_active = 0;
}

/* Select all */
void select_all(Editor *ed) {
    ed->sel_active = 1;
    ed->sel_start_y = 0;
    ed->sel_start_x = 0;
    
    Line *last = ed->first_line;
    int last_y = 0;
    while (last->next) {
        last = last->next;
        last_y++;
    }
    ed->sel_end_y = last_y;
    ed->sel_end_x = last->len;
    
    set_message(ed, "All selected");
}

/* Delete selection */
void delete_selection(Editor *ed) {
    if (!ed->sel_active) return;
    
    int sy = ed->sel_start_y, sx = ed->sel_start_x;
    int ey = ed->sel_end_y, ex = ed->sel_end_x;
    
    /* Normalize */
    if (sy > ey || (sy == ey && sx > ex)) {
        int tmp;
        tmp = sy; sy = ey; ey = tmp;
        tmp = sx; sx = ex; ex = tmp;
    }
    
    Line *start_line = get_line_at(ed, sy);
    Line *end_line = get_line_at(ed, ey);
    
    if (sy == ey) {
        /* Single line */
        memmove(&start_line->data[sx], &start_line->data[ex],
                start_line->len - ex + 1);
        start_line->len -= (ex - sx);
    } else {
        /* Multiple lines */
        start_line->data[sx] = '\0';
        start_line->len = sx;
        
        while (start_line->len + strlen(&end_line->data[ex]) + 1 >= start_line->capacity) {
            start_line->capacity *= 2;
            start_line->data = realloc(start_line->data, start_line->capacity);
        }
        strcat(start_line->data, &end_line->data[ex]);
        start_line->len += strlen(&end_line->data[ex]);
        
        /* Delete intermediate lines */
        Line *temp = start_line->next;
        while (temp && temp != end_line->next) {
            Line *next = temp->next;
            free(temp->data);
            free(temp);
            temp = next;
            ed->total_lines--;
        }
        
        start_line->next = end_line->next;
        if (end_line->next) {
            end_line->next->prev = start_line;
        }
    }
    
    ed->current_line = start_line;
    ed->cursor_y = sy;
    ed->cursor_x = sx;
    ed->preferred_x = sx;
    ed->sel_active = 0;
    ed->modified = 1;
}

/* Copy selection */
void copy_selection(Editor *ed) {
    if (!ed->sel_active) {
        set_message(ed, "Select text first");
        return;
    }
    
    /* Free old clipboard */
    for (int i = 0; i < ed->clipboard_lines; i++) {
        free(ed->clipboard[i]);
    }
    free(ed->clipboard);
    
    int sy = ed->sel_start_y, sx = ed->sel_start_x;
    int ey = ed->sel_end_y, ex = ed->sel_end_x;
    
    /* Normalize */
    if (sy > ey || (sy == ey && sx > ex)) {
        int tmp;
        tmp = sy; sy = ey; ey = tmp;
        tmp = sx; sx = ex; ex = tmp;
    }
    
    ed->clipboard_lines = ey - sy + 1;
    ed->clipboard = malloc(sizeof(char*) * ed->clipboard_lines);
    
    Line *line = get_line_at(ed, sy);
    for (int i = 0; i < ed->clipboard_lines && line; i++) {
        if (sy == ey) {
            ed->clipboard[i] = safe_strndup(&line->data[sx], ex - sx);
        } else if (i == 0) {
            ed->clipboard[i] = strdup(&line->data[sx]);
        } else if (i == ed->clipboard_lines - 1) {
            ed->clipboard[i] = safe_strndup(line->data, ex);
        } else {
            ed->clipboard[i] = strdup(line->data);
        }
        line = line->next;
    }
    
    char msg[64];
    snprintf(msg, sizeof(msg), "Nusxalandi: %d qator", ed->clipboard_lines);
    set_message(ed, msg);
}

/* Cut selection */
void cut_selection(Editor *ed) {
    copy_selection(ed);
    delete_selection(ed);
}

/* Paste clipboard */
void paste_clipboard(Editor *ed) {
    if (ed->clipboard_lines == 0) {
        set_message(ed, "Clipboard empty");
        return;
    }
    
    if (ed->sel_active) {
        delete_selection(ed);
    }
    
    Line *line = ed->current_line;
    
    if (ed->clipboard_lines == 1) {
        /* Single line paste */
        size_t paste_len = strlen(ed->clipboard[0]);
        while (line->len + paste_len + 1 >= line->capacity) {
            line->capacity *= 2;
            line->data = realloc(line->data, line->capacity);
        }
        
        memmove(&line->data[ed->cursor_x + paste_len], &line->data[ed->cursor_x],
                line->len - ed->cursor_x + 1);
        memcpy(&line->data[ed->cursor_x], ed->clipboard[0], paste_len);
        line->len += paste_len;
        ed->cursor_x += paste_len;
        ed->preferred_x = ed->cursor_x;
    } else {
        /* Multi-line paste - implementation similar to single line */
        /* For brevity, simplified version */
        for (int i = 0; i < ed->clipboard_lines; i++) {
            if (i > 0) insert_newline(ed);
            for (char *p = ed->clipboard[i]; *p; p++) {
                insert_char(ed, *p);
            }
        }
    }
    
    ed->modified = 1;
    set_message(ed, "Pasted");
}

/* Cut line (Nano-style Ctrl+K) */
void cut_line(Editor *ed) {
    Line *line = ed->current_line;
    
    /* Add to cut buffer */
    ed->cut_buffer = realloc(ed->cut_buffer, sizeof(char*) * (ed->cut_buffer_lines + 1));
    ed->cut_buffer[ed->cut_buffer_lines] = strdup(&line->data[ed->cursor_x]);
    ed->cut_buffer_lines++;
    
    /* Cut from cursor to end of line */
    line->data[ed->cursor_x] = '\0';
    line->len = ed->cursor_x;
    
    /* If line is empty, delete it */
    if (line->len == 0 && line->next) {
        Line *next = line->next;
        if (line->prev) {
            line->prev->next = next;
            next->prev = line->prev;
            ed->current_line = line->prev;
        } else {
            ed->first_line = next;
            next->prev = NULL;
            ed->current_line = next;
        }
        free(line->data);
        free(line);
        ed->total_lines--;
        ed->cursor_x = 0;
        ed->cursor_y = (ed->cursor_y > 0) ? ed->cursor_y - 1 : 0;
    }
    
    ed->preferred_x = ed->cursor_x;
    ed->modified = 1;
    set_message(ed, "Line cut (Ctrl+U to paste)");
}

/* Uncut text (Nano-style Ctrl+U) */
void uncut_text(Editor *ed) {
    if (ed->cut_buffer_lines == 0) {
        set_message(ed, "Cut buffer empty");
        return;
    }
    
    for (int i = 0; i < ed->cut_buffer_lines; i++) {
        for (char *p = ed->cut_buffer[i]; *p; p++) {
            insert_char(ed, *p);
        }
        if (i < ed->cut_buffer_lines - 1) {
            insert_newline(ed);
        }
    }
    
    /* Clear cut buffer */
    for (int i = 0; i < ed->cut_buffer_lines; i++) {
        free(ed->cut_buffer[i]);
    }
    free(ed->cut_buffer);
    ed->cut_buffer = NULL;
    ed->cut_buffer_lines = 0;
    
    set_message(ed, "Pasted");
}

/* Handle input */
void handle_input(Editor *ed, int ch) {
    debug_log("handle_input: ch=%d (0x%02x) %s", ch, ch, 
              ch >= 32 && ch < 127 ? (char[]){ch, 0} : "");
    
    switch (ch) {
        case 19: /* Ctrl+S */
            debug_log("ACTION: Ctrl+S - save_file()");
            save_file(ed);
            break;
            
        case 17: /* Ctrl+Q */
            if (ed->modified) {
                set_message(ed, "Unsaved! Ctrl+S to save or Ctrl+Q again");
                draw_screen(ed);
                timeout(-1);
                int ch2 = getch();
                timeout(50);
                if (ch2 == 17) {
                    cleanup_editor(ed);
                    endwin();
                    exit(0);
                }
            } else {
                cleanup_editor(ed);
                endwin();
                exit(0);
            }
            break;
            
        case 1: /* Ctrl+A */
            select_all(ed);
            break;
            
        case 3: /* Ctrl+C */
            copy_selection(ed);
            break;
            
        case 24: /* Ctrl+X */
            cut_selection(ed);
            break;
            
        case 22: /* Ctrl+V */
            paste_clipboard(ed);
            break;
            
        case 11: /* Ctrl+K */
            cut_line(ed);
            break;
            
        case 21: /* Ctrl+U */
            uncut_text(ed);
            break;
            
        case 26: /* Ctrl+Z - Undo */
            debug_log("ACTION: Ctrl+Z - undo");
            perform_undo(ed);
            break;
            
        case 6: /* Ctrl+F - Search */
            debug_log("ACTION: Ctrl+F - search");
            search_text(ed);
            break;
            
        case 18: /* Ctrl+R - Replace */
            debug_log("ACTION: Ctrl+R - replace");
            replace_text(ed);
            break;
            
        case 25: /* Ctrl+Y - Redo */
            debug_log("ACTION: Ctrl+Y - redo (not implemented)");
            set_message(ed, "Redo: Coming soon");
            break;
            
        case KEY_BACKSPACE:
        case 127:
        case 8:
            backspace_char(ed);
            break;
            
        case KEY_DC:
            delete_char(ed);
            break;
            
        case '\n':
        case '\r':
        case KEY_ENTER:
            insert_newline(ed);
            break;
            
        case '\t':
            handle_tab(ed);
            break;
            
        case KEY_UP:
            move_cursor(ed, -1, 0);
            ed->sel_active = 0;  /* Clear selection on cursor move */
            break;
            
        case KEY_DOWN:
            move_cursor(ed, 1, 0);
            ed->sel_active = 0;  /* Clear selection on cursor move */
            break;
            
        case KEY_LEFT:
            move_cursor(ed, 0, -1);
            ed->sel_active = 0;  /* Clear selection on cursor move */
            break;
            
        case KEY_RIGHT:
            move_cursor(ed, 0, 1);
            ed->sel_active = 0;  /* Clear selection on cursor move */
            break;
            
        case KEY_HOME:
            ed->cursor_x = 0;
            ed->preferred_x = 0;
            ed->sel_active = 0;
            break;
            
        case KEY_END:
            ed->cursor_x = ed->current_line->len;
            ed->preferred_x = ed->cursor_x;
            ed->sel_active = 0;
            break;
            
        case KEY_PPAGE: /* Page Up */
            page_up(ed);
            break;
            
        case KEY_NPAGE: /* Page Down */
            page_down(ed);
            break;
            
        case KEY_MOUSE:
            handle_mouse(ed);
            break;
            
        default:
            if (ch >= 32 && ch < 127) {
                insert_char(ed, (char)ch);
            }
            break;
    }
}

/* Handle mouse */
void handle_mouse(Editor *ed) {
    MEVENT event;
    if (getmouse(&event) == OK) {
        debug_log("MOUSE: x=%d y=%d bstate=0x%08lx", event.x, event.y, event.bstate);
        
        /* Check if clicking on status bar with syntax error */
        int status_line = ed->screen_height - 2;
        if (event.y == status_line && (event.bstate & BUTTON1_PRESSED)) {
            if (ed->syntax_error.line > 0) {
                /* Jump to error line */
                int target_line = ed->syntax_error.line - 1;  /* Convert to 0-based */
                if (target_line >= 0 && target_line < ed->total_lines) {
                    ed->cursor_y = target_line;
                    ed->current_line = get_line_at(ed, target_line);
                    ed->cursor_x = ed->syntax_error.col_start;
                    if (ed->cursor_x < 0) ed->cursor_x = 0;
                    if (ed->current_line && (size_t)ed->cursor_x > ed->current_line->len) {
                        ed->cursor_x = ed->current_line->len;
                    }
                    ed->preferred_x = ed->cursor_x;
                    
                    /* Adjust scroll if needed */
                    if (ed->cursor_y < ed->offset_y) {
                        ed->offset_y = ed->cursor_y;
                    } else if (ed->cursor_y >= ed->offset_y + ed->edit_height) {
                        ed->offset_y = ed->cursor_y - ed->edit_height + 1;
                    }
                    
                    debug_log("MOUSE: clicked error status, jumped to line %d", ed->syntax_error.line);
                    set_message(ed, "Jumped to error line");
                }
            }
            return;  /* Don't process further */
        }
        
        if (event.y < ed->edit_height && event.x >= LINE_NUMBER_WIDTH) {
            int line_num = ed->offset_y + event.y;
            int col = event.x - LINE_NUMBER_WIDTH;
            
            if (line_num >= ed->total_lines) {
                line_num = ed->total_lines - 1;
            }
            if (line_num < 0) line_num = 0;
            
            /* Handle right-click paste */
            if (event.bstate & BUTTON3_PRESSED) {
                /* Right click - paste at cursor position */
                Line *target = get_line_at(ed, line_num);
                if (target) {
                    ed->current_line = target;
                    ed->cursor_y = line_num;
                    
                    /* Adjust column */
                    if ((size_t)col > target->len) {
                        col = (int)target->len;
                    }
                    if (col < 0) col = 0;
                    
                    ed->cursor_x = col;
                    ed->preferred_x = col;
                    
                    /* Paste clipboard */
                    paste_clipboard(ed);
                    debug_log("MOUSE: right-click paste at line=%d col=%d", line_num, col);
                }
                return;
            }
            
            /* Handle button events */
            if (event.bstate & BUTTON1_PRESSED) {
                /* Click - set cursor position and start selection */
                Line *target = get_line_at(ed, line_num);
                if (target) {
                    ed->current_line = target;
                    ed->cursor_y = line_num;
                    
                    /* Adjust column for line length */
                    if ((size_t)col > target->len) {
                        col = (int)target->len;
                    }
                    if (col < 0) col = 0;
                    
                    ed->cursor_x = col;
                    ed->preferred_x = col;
                    
                    debug_log("MOUSE: BUTTON1_PRESSED at line=%d col=%d", line_num, col);
                    ed->mouse_pressed = 1;
                    ed->sel_start_y = line_num;
                    ed->sel_start_x = col;
                    ed->sel_end_y = line_num;
                    ed->sel_end_x = col;
                    ed->sel_active = 0;  /* Don't activate until drag */
                }
            } else if (event.bstate & BUTTON1_RELEASED) {
                /* Release - finalize selection */
                if (ed->mouse_pressed) {
                    Line *target = get_line_at(ed, line_num);
                    if (target) {
                        /* Adjust column for line length */
                        if ((size_t)col > target->len) {
                            col = (int)target->len;
                        }
                        if (col < 0) col = 0;
                        
                        ed->sel_end_y = line_num;
                        ed->sel_end_x = col;
                        
                        /* Only activate selection if dragged */
                        if (ed->sel_start_y != ed->sel_end_y || ed->sel_start_x != ed->sel_end_x) {
                            ed->sel_active = 1;
                            debug_log("MOUSE: selection activated from (%d,%d) to (%d,%d)", 
                                     ed->sel_start_y, ed->sel_start_x, ed->sel_end_y, ed->sel_end_x);
                            
                            /* Auto-copy to clipboard */
                            copy_selection(ed);
                            set_message(ed, "Copied (mouse selection)");
                            debug_log("MOUSE: auto-copied selection to clipboard");
                        } else {
                            ed->sel_active = 0;  /* Just click, no selection */
                            debug_log("MOUSE: just click, no selection");
                        }
                    }
                    ed->mouse_pressed = 0;
                } else {
                    /* Released but wasn't pressed - clear any lingering selection */
                    ed->sel_active = 0;
                    debug_log("MOUSE: released without press, clearing selection");
                }
            } else if ((event.bstate & REPORT_MOUSE_POSITION) && ed->mouse_pressed) {
                /* Dragging - update selection only, NOT cursor */
                Line *target = get_line_at(ed, line_num);
                if (target) {
                    /* Adjust column for line length */
                    if ((size_t)col > target->len) {
                        col = (int)target->len;
                    }
                    if (col < 0) col = 0;
                    
                    ed->sel_end_y = line_num;
                    ed->sel_end_x = col;
                    ed->sel_active = 1;
                    
                    /* Redraw for real-time feedback */
                    draw_screen(ed);
                    debug_log("MOUSE: drag selection updated (real-time)");
                }
            }
        }
    }
}

/* Search text */
void search_text(Editor *ed) {
    debug_log("search_text: starting");
    
    /* Input mode - NO echo to prevent backspace artifacts */
    curs_set(2);
    
    char query[256] = {0};
    int input_pos = 0;
    int ch;
    
    /* Show initial prompt */
    mvprintw(ed->screen_height - 1, 0, "^F Qidirish  ^C Bekor: ");
    clrtoeol();
    refresh();
    
    /* Manual input reading */
    while (1) {
        ch = getch();
        
        /* Check for exit keys */
        if (ch == '\n' || ch == '\r') {
            break;  /* Enter pressed */
        } else if (ch == 27 || ch == 3) {  /* ESC or Ctrl+C */
            curs_set(1);
            set_message(ed, "Cancelled");
            debug_log("search: cancelled by %s", ch == 27 ? "ESC" : "Ctrl+C");
            return;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (input_pos > 0) {
                input_pos--;
                query[input_pos] = '\0';
                /* Redraw entire line */
                mvprintw(ed->screen_height - 1, 0, "^F Qidirish  ^C Bekor: %s ", query);
                clrtoeol();
                refresh();
            }
        } else if (ch >= 32 && ch < 127 && input_pos < 255) {
            query[input_pos++] = ch;
            query[input_pos] = '\0';
            /* Redraw to show new character */
            mvprintw(ed->screen_height - 1, 0, "^F Qidirish  ^C Bekor: %s", query);
            refresh();
        }
    }
    
    /* Return to normal mode */
    curs_set(1);
    
    if (strlen(query) == 0) {
        set_message(ed, "Bekor qilindi");
        debug_log("search: cancelled");
        return;
    }
    
    debug_log("search: query='%s'", query);
    
    /* Count occurrences first */
    int count = 0;
    Line *line = ed->first_line;
    int line_num = 0;
    
    while (line) {
        char *ptr = line->data;
        while ((ptr = strstr(ptr, query)) != NULL) {
            count++;
            ptr++;
        }
        line = line->next;
        line_num++;
    }
    
    if (count == 0) {
        set_message(ed, "Not found");
        debug_log("search: not found");
        return;
    }
    
    /* Show count */
    char msg[128];
    snprintf(msg, sizeof(msg), "Topildi: %d ta", count);
    set_message(ed, msg);
    debug_log("search: found %d occurrences", count);
    
    /* Find first occurrence from current position */
    line = ed->current_line;
    line_num = ed->cursor_y;
    
    /* Search from cursor position in current line */
    char *pos = strstr(&line->data[ed->cursor_x + 1], query);
    if (pos) {
        ed->cursor_x = pos - line->data;
        ed->preferred_x = ed->cursor_x;
        ed->sel_start_y = line_num;
        ed->sel_start_x = ed->cursor_x;
        ed->sel_end_y = line_num;
        ed->sel_end_x = ed->cursor_x + strlen(query);
        ed->sel_active = 1;
        debug_log("search: found at line=%d col=%d", line_num, ed->cursor_x);
        return;
    }
    
    /* Search from next lines */
    line = line->next;
    line_num++;
    
    while (line) {
        pos = strstr(line->data, query);
        if (pos) {
            ed->current_line = line;
            ed->cursor_y = line_num;
            ed->cursor_x = pos - line->data;
            ed->preferred_x = ed->cursor_x;
            ed->sel_start_y = line_num;
            ed->sel_start_x = ed->cursor_x;
            ed->sel_end_y = line_num;
            ed->sel_end_x = ed->cursor_x + strlen(query);
            ed->sel_active = 1;
            debug_log("search: found at line=%d col=%d", line_num, ed->cursor_x);
            return;
        }
        line = line->next;
        line_num++;
    }
    
    /* Search from beginning */
    line = ed->first_line;
    line_num = 0;
    
    while (line && line_num < ed->cursor_y) {
        pos = strstr(line->data, query);
        if (pos) {
            ed->current_line = line;
            ed->cursor_y = line_num;
            ed->cursor_x = pos - line->data;
            ed->preferred_x = ed->cursor_x;
            ed->sel_start_y = line_num;
            ed->sel_start_x = ed->cursor_x;
            ed->sel_end_y = line_num;
            ed->sel_end_x = ed->cursor_x + strlen(query);
            ed->sel_active = 1;
            debug_log("search: found at line=%d col=%d (wrapped)", line_num, ed->cursor_x);
            return;
        }
        line = line->next;
        line_num++;
    }
}

/* Replace text */
void replace_text(Editor *ed) {
    debug_log("replace_text: starting");
    
    /* Input mode - NO echo */
    curs_set(2);
    
    char query[256] = {0};
    int input_pos = 0;
    int ch;
    
    /* Get search query */
    mvprintw(ed->screen_height - 1, 0, "^R Qidirish  ^C Bekor: ");
    clrtoeol();
    refresh();
    
    while (1) {
        ch = getch();
        
        if (ch == '\n' || ch == '\r') {
            break;
        } else if (ch == 27 || ch == 3) {  /* ESC or Ctrl+C */
            curs_set(1);
            set_message(ed, "Cancelled");
            debug_log("replace: cancelled by %s", ch == 27 ? "ESC" : "Ctrl+C");
            return;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (input_pos > 0) {
                input_pos--;
                query[input_pos] = '\0';
                mvprintw(ed->screen_height - 1, 0, "^R Qidirish  ^C Bekor: %s ", query);
                clrtoeol();
                refresh();
            }
        } else if (ch >= 32 && ch < 127 && input_pos < 255) {
            query[input_pos++] = ch;
            query[input_pos] = '\0';
            mvprintw(ed->screen_height - 1, 0, "^R Qidirish  ^C Bekor: %s", query);
            refresh();
        }
    }
    
    if (strlen(query) == 0) {
        curs_set(1);
        set_message(ed, "Bekor qilindi");
        return;
    }
    
    /* Get replacement */
    mvprintw(ed->screen_height - 1, 0, "^R Almashtirish  ^C Bekor: ");
    clrtoeol();
    refresh();
    
    char replacement[256] = {0};
    input_pos = 0;
    
    while (1) {
        ch = getch();
        
        if (ch == '\n' || ch == '\r') {
            break;
        } else if (ch == 27 || ch == 3) {  /* ESC or Ctrl+C */
            curs_set(1);
            set_message(ed, "Cancelled");
            return;
        } else if (ch == KEY_BACKSPACE || ch == 127 || ch == 8) {
            if (input_pos > 0) {
                input_pos--;
                replacement[input_pos] = '\0';
                mvprintw(ed->screen_height - 1, 0, "^R Almashtirish  ^C Bekor: %s ", replacement);
                clrtoeol();
                refresh();
            }
        } else if (ch >= 32 && ch < 127 && input_pos < 255) {
            replacement[input_pos++] = ch;
            replacement[input_pos] = '\0';
            mvprintw(ed->screen_height - 1, 0, "^R Almashtirish  ^C Bekor: %s", replacement);
            refresh();
        }
    }
    
    curs_set(1);
    
    debug_log("replace: query='%s' replacement='%s'", query, replacement);
    
    /* Count occurrences */
    int count = 0;
    Line *line = ed->first_line;
    
    while (line) {
        char *ptr = line->data;
        while ((ptr = strstr(ptr, query)) != NULL) {
            count++;
            ptr++;
        }
        line = line->next;
    }
    
    if (count == 0) {
        set_message(ed, "Not found");
        debug_log("replace: not found");
        return;
    }
    
    /* Show count and ask */
    char msg[128];
    snprintf(msg, sizeof(msg), "%d ta topildi. Hammasi (a) / Bitta (1) / Bekor (ESC)?", count);
    mvprintw(ed->screen_height - 1, 0, "%s", msg);
    clrtoeol();
    refresh();
    
    /* Wait for choice - no timeout */
    nodelay(stdscr, FALSE);
    int choice = getch();
    nodelay(stdscr, TRUE);
    debug_log("replace: choice=%d (0x%02x)", choice, choice);
    
    if (choice == 27) { /* ESC */
        set_message(ed, "Bekor qilindi");
        return;
    }
    
    save_undo(ed);
    
    int replaced = 0;
    int query_len = strlen(query);
    int repl_len = strlen(replacement);
    
    if (choice == 'a' || choice == 'A') {
        /* Replace all */
        line = ed->first_line;
        
        while (line) {
            char *pos;
            while ((pos = strstr(line->data, query)) != NULL) {
                int offset = pos - line->data;
                
                /* Allocate new buffer */
                size_t new_len = line->len - query_len + repl_len;
                if (new_len + 1 >= line->capacity) {
                    line->capacity = new_len + 128;
                    line->data = realloc(line->data, line->capacity);
                }
                
                /* Move tail */
                memmove(pos + repl_len, pos + query_len, strlen(pos + query_len) + 1);
                /* Copy replacement */
                memcpy(pos, replacement, repl_len);
                line->len = new_len;
                
                replaced++;
                pos = line->data + offset + repl_len;
            }
            line = line->next;
        }
        
        snprintf(msg, sizeof(msg), "Almashtirildi: %d ta", replaced);
        set_message(ed, msg);
        ed->modified = 1;
        debug_log("replace: replaced %d occurrences", replaced);
    } else if (choice == '1') {
        /* Replace one - find first from cursor */
        line = ed->current_line;
        
        char *pos = strstr(&line->data[ed->cursor_x], query);
        if (pos) {
            int offset = pos - line->data;
            size_t new_len = line->len - query_len + repl_len;
            
            if (new_len + 1 >= line->capacity) {
                line->capacity = new_len + 128;
                line->data = realloc(line->data, line->capacity);
            }
            
            memmove(pos + repl_len, pos + query_len, strlen(pos + query_len) + 1);
            memcpy(pos, replacement, repl_len);
            line->len = new_len;
            
            ed->cursor_x = offset + repl_len;
            ed->preferred_x = ed->cursor_x;
            ed->modified = 1;
            
            set_message(ed, "Almashtirildi: 1 ta");
            debug_log("replace: replaced 1 occurrence");
        }
    }
}

/* Cleanup */
void cleanup_editor(Editor *ed) {
    /* Disable mouse motion tracking */
    printf("\033[?1003l");
    fflush(stdout);
    
    /* Clear any active selections */
    ed->sel_active = 0;
    ed->mouse_pressed = 0;
    
    debug_log("cleanup_editor: cleaning up");
    
    Line *line = ed->first_line;
    while (line) {
        Line *next = line->next;
        free(line->data);
        free(line);
        line = next;
    }
    
    if (ed->filename) free(ed->filename);
    
    for (int i = 0; i < ed->clipboard_lines; i++) {
        free(ed->clipboard[i]);
    }
    free(ed->clipboard);
    
    for (int i = 0; i < ed->cut_buffer_lines; i++) {
        free(ed->cut_buffer[i]);
    }
    free(ed->cut_buffer);
}

/* Main */
int main(int argc, char *argv[]) {
    Editor ed;
    const char *filename = (argc > 1) ? argv[1] : NULL;
    
    init_editor(&ed, filename);
    
    debug_log("Entering main loop");
    
    while (1) {
        draw_screen(&ed);
        int ch = getch();
        if (ch != ERR) {
            debug_log("Got key: %d", ch);
            handle_input(&ed, ch);
        }
    }
    
    cleanup_editor(&ed);
    endwin();
    
    return 0;
}
