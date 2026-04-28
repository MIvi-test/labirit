#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include "graph.h"

static bool same_text(const char *left, const char *right)
{
    if (!left || !right)
    {
        return false;
    }

    while (*left && *right)
    {
        if (*left != *right)
        {
            return false;
        }
        left++;
        right++;
    }

    return *left == '\0' && *right == '\0';
}

MazeAlgorithm parse_algorithm_name(const char *name)
{
    if (!name || same_text(name, "prim"))
    {
        return MAZE_ALGO_PRIM;
    }
    if (same_text(name, "dfs"))
    {
        return MAZE_ALGO_DFS;
    }
    if (same_text(name, "growing_tree"))
    {
        return MAZE_ALGO_GROWING_TREE;
    }
    if (same_text(name, "watson"))
    {
        return MAZE_ALGO_WATSON;
    }
    if (same_text(name, "binary"))
    {
        return MAZE_ALGO_BINARY;
    }

    return MAZE_ALGO_PRIM;
}

const char *maze_algorithm_name(MazeAlgorithm algorithm)
{
    switch (algorithm)
    {
    case MAZE_ALGO_DFS:
        return "dfs";
    case MAZE_ALGO_GROWING_TREE:
        return "growing_tree";
    case MAZE_ALGO_WATSON:
        return "watson";
    case MAZE_ALGO_BINARY:
        return "binary";
    case MAZE_ALGO_PRIM:
    default:
        return "prim";
    }
}

void maze_run_algorithm(TABLE table, MazeAlgorithm algorithm)
{
    switch (algorithm)
    {
    case MAZE_ALGO_DFS:
        DFS_alg(table);
        break;
    case MAZE_ALGO_GROWING_TREE:
        growing_tree_alg(table);
        break;
    case MAZE_ALGO_WATSON:
        watson_alg(table);
        break;
    case MAZE_ALGO_BINARY:
        binary_algos(table);
        break;
    case MAZE_ALGO_PRIM:
    default:
        prim_alg(table);
        break;
    }
}

void print_maze_info(TABLE table, MazeAlgorithm algorithm)
{
    printf("algorithm: %s\n", maze_algorithm_name(algorithm));
    printf("size: %d x %d\n", table.columns, table.rows);
    printf("seed: %d\n\n", table.seed);
}

void print_maze(TABLE table)
{
    if (!table.data || table.rows <= 0 || table.columns <= 0)
    {
        printf("maze is empty\n");
        return;
    }

    printf("+");
    for (int x = 0; x < table.columns; x++)
    {
        printf("---+");
    }
    printf("\n");

    for (int y = 0; y < table.rows; y++)
    {
        printf("|");
        for (int x = 0; x < table.columns; x++)
        {
            printf("   ");
            if (table.data[y][x].wall.right)
            {
                printf("|");
            }
            else
            {
                printf(" ");
            }
        }
        printf("\n");

        printf("+");
        for (int x = 0; x < table.columns; x++)
        {
            if (table.data[y][x].wall.bottom)
            {
                printf("---+");
            }
            else
            {
                printf("   +");
            }
        }
        printf("\n");
    }
}

#ifdef _WIN32

typedef struct
{
    TABLE table;
    int columns;
    int rows;
    unsigned int seed;
    MazeAlgorithm algorithm;
    HWND algorithm_combo;
    HWND columns_edit;
    HWND rows_edit;
    HWND seed_edit;
    HWND apply_button;
    HWND regenerate_button;
} MazeAppState;

static MazeAppState g_app = {0};

enum
{
    IDC_ALGORITHM = 1001,
    IDC_COLUMNS = 1002,
    IDC_ROWS = 1003,
    IDC_SEED = 1004,
    IDC_APPLY = 1005,
    IDC_REGENERATE = 1006
};

static void maze_generate_current(void)
{
    if (g_app.table.data)
    {
        CLEAR_TABLE(&g_app.table);
    }

    g_app.table = INIT_TABLE((unsigned int)g_app.columns, (unsigned int)g_app.rows, g_app.seed);
    if (g_app.table.data)
    {
        maze_run_algorithm(g_app.table, g_app.algorithm);
    }
}

static const char *maze_short_help(void)
{
    return "Use the controls above, or press R to regenerate quickly";
}

static void maze_set_window_title(HWND hwnd)
{
    char title[256];
    snprintf(title,
             sizeof(title),
             "Labirit - %s - %dx%d - seed %u",
             maze_algorithm_name(g_app.algorithm),
             g_app.columns,
             g_app.rows,
             g_app.seed);
    SetWindowTextA(hwnd, title);
}

static void maze_set_edit_int(HWND edit, unsigned int value)
{
    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%u", value);
    SetWindowTextA(edit, buffer);
}

static unsigned int maze_get_edit_uint(HWND edit, unsigned int fallback)
{
    char buffer[32];
    if (!GetWindowTextA(edit, buffer, (int)sizeof(buffer)))
    {
        return fallback;
    }

    int value = atoi(buffer);
    if (value <= 0)
    {
        return fallback;
    }

    return (unsigned int)value;
}

static void maze_sync_controls_from_state(void)
{
    if (!g_app.algorithm_combo)
    {
        return;
    }

    SendMessageA(g_app.algorithm_combo, CB_SETCURSEL, (WPARAM)g_app.algorithm, 0);
    maze_set_edit_int(g_app.columns_edit, (unsigned int)g_app.columns);
    maze_set_edit_int(g_app.rows_edit, (unsigned int)g_app.rows);
    maze_set_edit_int(g_app.seed_edit, g_app.seed);
}

static void maze_sync_state_from_controls(void)
{
    LRESULT selected = SendMessageA(g_app.algorithm_combo, CB_GETCURSEL, 0, 0);
    if (selected >= 0 && selected <= MAZE_ALGO_BINARY)
    {
        g_app.algorithm = (MazeAlgorithm)selected;
    }

    g_app.columns = (int)maze_get_edit_uint(g_app.columns_edit, (unsigned int)g_app.columns);
    g_app.rows = (int)maze_get_edit_uint(g_app.rows_edit, (unsigned int)g_app.rows);
    g_app.seed = maze_get_edit_uint(g_app.seed_edit, g_app.seed);
}

static void maze_create_controls(HWND hwnd)
{
    HINSTANCE instance = GetModuleHandleA(NULL);

    CreateWindowA("STATIC", "Algorithm:", WS_CHILD | WS_VISIBLE,
                  20, 16, 60, 20, hwnd, NULL, instance, NULL);
    g_app.algorithm_combo = CreateWindowA("COMBOBOX", "",
                                          WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST,
                                          85, 12, 150, 200, hwnd, (HMENU)IDC_ALGORITHM, instance, NULL);
    SendMessageA(g_app.algorithm_combo, CB_ADDSTRING, 0, (LPARAM)"prim");
    SendMessageA(g_app.algorithm_combo, CB_ADDSTRING, 0, (LPARAM)"dfs");
    SendMessageA(g_app.algorithm_combo, CB_ADDSTRING, 0, (LPARAM)"growing_tree");
    SendMessageA(g_app.algorithm_combo, CB_ADDSTRING, 0, (LPARAM)"watson");
    SendMessageA(g_app.algorithm_combo, CB_ADDSTRING, 0, (LPARAM)"binary");

    CreateWindowA("STATIC", "Cols:", WS_CHILD | WS_VISIBLE,
                  250, 16, 35, 20, hwnd, NULL, instance, NULL);
    g_app.columns_edit = CreateWindowA("EDIT", "",
                                       WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER,
                                       290, 12, 60, 24, hwnd, (HMENU)IDC_COLUMNS, instance, NULL);

    CreateWindowA("STATIC", "Rows:", WS_CHILD | WS_VISIBLE,
                  365, 16, 38, 20, hwnd, NULL, instance, NULL);
    g_app.rows_edit = CreateWindowA("EDIT", "",
                                    WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER,
                                    408, 12, 60, 24, hwnd, (HMENU)IDC_ROWS, instance, NULL);

    CreateWindowA("STATIC", "Seed:", WS_CHILD | WS_VISIBLE,
                  483, 16, 38, 20, hwnd, NULL, instance, NULL);
    g_app.seed_edit = CreateWindowA("EDIT", "",
                                    WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP | ES_NUMBER,
                                    525, 12, 110, 24, hwnd, (HMENU)IDC_SEED, instance, NULL);

    g_app.apply_button = CreateWindowA("BUTTON", "Apply",
                                       WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                       650, 11, 90, 26, hwnd, (HMENU)IDC_APPLY, instance, NULL);
    g_app.regenerate_button = CreateWindowA("BUTTON", "Regenerate",
                                            WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                                            750, 11, 110, 26, hwnd, (HMENU)IDC_REGENERATE, instance, NULL);

    maze_sync_controls_from_state();
}

static void maze_apply_and_redraw(HWND hwnd)
{
    maze_sync_state_from_controls();
    maze_generate_current();
    maze_sync_controls_from_state();
    maze_set_window_title(hwnd);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void maze_draw(HWND hwnd, HDC hdc)
{
    RECT client_rect;
    GetClientRect(hwnd, &client_rect);
    FillRect(hdc, &client_rect, (HBRUSH)(COLOR_WINDOW + 1));

    if (!g_app.table.data || g_app.columns <= 0 || g_app.rows <= 0)
    {
        TextOutA(hdc, 20, 20, "Maze is empty", 13);
        return;
    }

    const int margin = 20;
    const int top_panel = 92;
    const int draw_width = (client_rect.right - client_rect.left) - margin * 2;
    const int draw_height = (client_rect.bottom - client_rect.top) - top_panel - margin;
    const int cell_size_x = draw_width / g_app.columns;
    const int cell_size_y = draw_height / g_app.rows;
    const int cell_size = (cell_size_x < cell_size_y) ? cell_size_x : cell_size_y;

    if (cell_size < 4)
    {
        TextOutA(hdc, 20, 20, "Window is too small for this maze", 31);
        return;
    }

    const int maze_width = cell_size * g_app.columns;
    const int maze_height = cell_size * g_app.rows;
    const int origin_x = margin;
    const int origin_y = top_panel;

    char info[256];
    snprintf(info,
             sizeof(info),
             "Algorithm: %s | Size: %dx%d | Seed: %u",
             maze_algorithm_name(g_app.algorithm),
             g_app.columns,
             g_app.rows,
             g_app.seed);
    TextOutA(hdc, margin, 52, info, (int)strlen(info));
    TextOutA(hdc, margin, 68, maze_short_help(), (int)strlen(maze_short_help()));

    HPEN maze_pen = CreatePen(PS_SOLID, 2, RGB(30, 30, 30));
    HPEN old_pen = (HPEN)SelectObject(hdc, maze_pen);

    for (int y = 0; y < g_app.rows; y++)
    {
        for (int x = 0; x < g_app.columns; x++)
        {
            const int left = origin_x + x * cell_size;
            const int top = origin_y + y * cell_size;
            const int right = left + cell_size;
            const int bottom = top + cell_size;
            Cell cell = g_app.table.data[y][x];

            if (cell.wall.top)
            {
                MoveToEx(hdc, left, top, NULL);
                LineTo(hdc, right, top);
            }
            if (cell.wall.right)
            {
                MoveToEx(hdc, right, top, NULL);
                LineTo(hdc, right, bottom);
            }
            if (cell.wall.bottom)
            {
                MoveToEx(hdc, left, bottom, NULL);
                LineTo(hdc, right, bottom);
            }
            if (cell.wall.left)
            {
                MoveToEx(hdc, left, top, NULL);
                LineTo(hdc, left, bottom);
            }
        }
    }

    SelectObject(hdc, old_pen);
    DeleteObject(maze_pen);
}

static void maze_apply_algorithm_key(HWND hwnd, WPARAM key)
{
    switch (key)
    {
    case '1':
        g_app.algorithm = MAZE_ALGO_PRIM;
        break;
    case '2':
        g_app.algorithm = MAZE_ALGO_GROWING_TREE;
        break;
    case '3':
        g_app.algorithm = MAZE_ALGO_WATSON;
        break;
    case '4':
        g_app.algorithm = MAZE_ALGO_BINARY;
        break;
    default:
        return;
    }

    g_app.seed = (unsigned int)time(NULL);
    maze_sync_controls_from_state();
    maze_apply_and_redraw(hwnd);
}

static LRESULT CALLBACK maze_window_proc(HWND hwnd, UINT message, WPARAM w_param, LPARAM l_param)
{
    (void)l_param;

    switch (message)
    {
    case WM_KEYDOWN:
        if (w_param == 'R')
        {
            g_app.seed = (unsigned int)time(NULL);
            maze_sync_controls_from_state();
            maze_apply_and_redraw(hwnd);
            return 0;
        }
        if (w_param == VK_OEM_PLUS || w_param == VK_ADD)
        {
            g_app.columns++;
            g_app.rows++;
            maze_sync_controls_from_state();
            maze_apply_and_redraw(hwnd);
            return 0;
        }
        if ((w_param == VK_OEM_MINUS || w_param == VK_SUBTRACT) && g_app.columns > 4 && g_app.rows > 4)
        {
            g_app.columns--;
            g_app.rows--;
            maze_sync_controls_from_state();
            maze_apply_and_redraw(hwnd);
            return 0;
        }

        maze_apply_algorithm_key(hwnd, w_param);
        return 0;

    case WM_COMMAND:
        switch (LOWORD(w_param))
        {
        case IDC_APPLY:
            maze_apply_and_redraw(hwnd);
            return 0;

        case IDC_REGENERATE:
            g_app.seed = (unsigned int)time(NULL);
            maze_sync_controls_from_state();
            maze_apply_and_redraw(hwnd);
            return 0;

        case IDC_ALGORITHM:
            if (HIWORD(w_param) == CBN_SELCHANGE)
            {
                maze_apply_and_redraw(hwnd);
            }
            return 0;

        default:
            break;
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        maze_draw(hwnd, hdc);
        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        if (g_app.table.data)
        {
            CLEAR_TABLE(&g_app.table);
        }
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProcA(hwnd, message, w_param, l_param);
    }

    return DefWindowProcA(hwnd, message, w_param, l_param);
}

int run_maze_window(int columns, int rows, unsigned int seed, MazeAlgorithm algorithm)
{
    HINSTANCE instance = GetModuleHandleA(NULL);
    const char *class_name = "LabiritWindowClass";

    g_app.columns = columns;
    g_app.rows = rows;
    g_app.seed = seed;
    g_app.algorithm = algorithm;
    g_app.table.data = NULL;

    WNDCLASSA window_class;
    ZeroMemory(&window_class, sizeof(window_class));
    window_class.lpfnWndProc = maze_window_proc;
    window_class.hInstance = instance;
    window_class.lpszClassName = class_name;
    window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    window_class.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&window_class);

    HWND hwnd = CreateWindowExA(0,
                                class_name,
                                "Labirit",
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT,
                                CW_USEDEFAULT,
                                1000,
                                760,
                                NULL,
                                NULL,
                                instance,
                                NULL);
    if (!hwnd)
    {
        fprintf(stderr, "Failed to create window\n");
        return 1;
    }

    maze_create_controls(hwnd);
    maze_generate_current();
    maze_sync_controls_from_state();
    maze_set_window_title(hwnd);

    ShowWindow(hwnd, SW_SHOWDEFAULT);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}

#else

int run_maze_window(int columns, int rows, unsigned int seed, MazeAlgorithm algorithm)
{
    TABLE table = INIT_TABLE((unsigned int)columns, (unsigned int)rows, seed);
    if (!table.data)
    {
        fprintf(stderr, "Failed to allocate maze table\n");
        return 1;
    }

    maze_run_algorithm(table, algorithm);
    print_maze_info(table, algorithm);
    print_maze(table);
    CLEAR_TABLE(&table);
    return 0;
}

#endif
