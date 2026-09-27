// 2048 (N*M) - Windows 图形界面版
// 行数 / 列数可在 2~10 之间用滑动条选择
// 操作: 方向键 或 W/A/S/D 移动, R 用当前设置重开, Esc 退出
#include <windows.h>
#include <commctrl.h>
#include <cstdio>
#include <ctime>

const int MIN_N = 2;           // 最小行/列数
const int MAX_N = 10;          // 最大行/列数
const int WIN_TILE = 2048;     // 获胜方块
const int GAP = 8;             // 格子间距
const int MARGIN = 16;         // 窗口边距
const int TOPBAR = 100;        // 顶部设置区高度
const int BOARD_H = 470;       // 棋盘区域高度
const int WIN_W = 640;
const int WIN_H = TOPBAR + BOARD_H + MARGIN + 24;

// 控件 ID
#define ID_TB_ROWS   1001
#define ID_TB_COLS   1002
#define ID_LB_ROWS   1003
#define ID_LB_COLS   1004
#define ID_BTN_START 1005

int R = 4, C = 4;              // 当前行数 / 列数
int board[MAX_N][MAX_N];
long long score = 0;
bool winShown = false;
bool gameOver = false;
HWND g_hwnd, g_tbRows, g_tbCols, g_lbRows, g_lbCols;

void init() {
    for (int i = 0; i < MAX_N; i++)
        for (int j = 0; j < MAX_N; j++)
            board[i][j] = 0;
    score = 0;
    winShown = false;
    gameOver = false;
}

void addTile() {
    int ei[MAX_N * MAX_N], ej[MAX_N * MAX_N], cnt = 0;
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            if (board[i][j] == 0) { ei[cnt] = i; ej[cnt] = j; cnt++; }
    if (!cnt) return;
    int k = rand() % cnt;
    board[ei[k]][ej[k]] = (rand() % 10 == 0) ? 4 : 2;
}

bool canMove() {
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++) {
            if (board[i][j] == 0) return true;
            if (j + 1 < C && board[i][j] == board[i][j + 1]) return true;
            if (i + 1 < R && board[i][j] == board[i + 1][j]) return true;
        }
    return false;
}

int mergeLine(int* line, int len) {
    int tmp[MAX_N] = {0}, t = 0;
    for (int i = 0; i < len; i++)
        if (line[i]) tmp[t++] = line[i];
    int gain = 0;
    for (int i = 0; i + 1 < t; i++)
        if (tmp[i] && tmp[i] == tmp[i + 1]) {
            tmp[i] *= 2;
            gain += tmp[i];
            tmp[i + 1] = 0;
        }
    for (int i = 0; i < len; i++) line[i] = 0;
    t = 0;
    for (int i = 0; i < len; i++)
        if (tmp[i]) line[t++] = tmp[i];
    return gain;
}

bool doMove(int dir) { // 0=左 1=右 2=上 3=下
    int before[MAX_N][MAX_N];
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            before[i][j] = board[i][j];
    int lines = (dir <= 1) ? R : C;   // 水平移动按行扫, 垂直按列扫
    int len = (dir <= 1) ? C : R;     // 每条线的长度
    for (int k = 0; k < lines; k++) {
        int line[MAX_N];
        for (int i = 0; i < len; i++) {
            switch (dir) {
                case 0: line[i] = board[k][i]; break;
                case 1: line[i] = board[k][len - 1 - i]; break;
                case 2: line[i] = board[i][k]; break;
                case 3: line[i] = board[len - 1 - i][k]; break;
            }
        }
        score += mergeLine(line, len);
        for (int i = 0; i < len; i++) {
            switch (dir) {
                case 0: board[k][i] = line[i]; break;
                case 1: board[k][len - 1 - i] = line[i]; break;
                case 2: board[i][k] = line[i]; break;
                case 3: board[len - 1 - i][k] = line[i]; break;
            }
        }
    }
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            if (before[i][j] != board[i][j]) return true;
    return false;
}

bool hasWon() {
    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++)
            if (board[i][j] >= WIN_TILE) return true;
    return false;
}

COLORREF tileColor(int v) {
    switch (v) {
        case 2:    return RGB(0xEE, 0xE4, 0xDA);
        case 4:    return RGB(0xED, 0xE0, 0xC8);
        case 8:    return RGB(0xF2, 0xB1, 0x79);
        case 16:   return RGB(0xF5, 0x95, 0x63);
        case 32:   return RGB(0xF6, 0x7C, 0x5F);
        case 64:   return RGB(0xF6, 0x5E, 0x3B);
        case 128:  return RGB(0xED, 0xCF, 0x72);
        case 256:  return RGB(0xED, 0xCC, 0x61);
        case 512:  return RGB(0xED, 0xC8, 0x50);
        case 1024: return RGB(0xED, 0xC5, 0x3F);
        case 2048: return RGB(0xED, 0xC2, 0x2E);
        default:   return RGB(0x3C, 0x3A, 0x32);
    }
}

void drawTile(HDC hdc, int x, int y, int cell, int v) {
    RECT rc = {x, y, x + cell, y + cell};
    HBRUSH br = CreateSolidBrush(v ? tileColor(v) : RGB(0xCD, 0xC1, 0xB4));
    FillRect(hdc, &rc, br);
    DeleteObject(br);
    if (!v) return;
    bool dark = (v <= 4);
    SetTextColor(hdc, dark ? RGB(0x77, 0x6E, 0x65) : RGB(0xFF, 0xFF, 0xFF));
    SetBkMode(hdc, TRANSPARENT);
    int len = 0, t = v;
    while (t) { len++; t /= 10; }
    int pt = cell / 3;
    if (len == 3) pt = cell * 2 / 7;
    else if (len >= 4) pt = cell / 4;
    HFONT font = CreateFont(pt, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                            DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    HFONT old = (HFONT)SelectObject(hdc, font);
    wchar_t wbuf[16];
    swprintf(wbuf, 16, L"%d", v);
    DrawTextW(hdc, wbuf, -1, &rc, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    SelectObject(hdc, old);
    DeleteObject(font);
}

void drawBoard(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    HDC mem = CreateCompatibleDC(hdc);
    HBITMAP bmp = CreateCompatibleBitmap(hdc, WIN_W, WIN_H);
    HBITMAP oldBmp = (HBITMAP)SelectObject(mem, bmp);

    RECT bg = {0, 0, WIN_W, WIN_H};
    HBRUSH bgBr = CreateSolidBrush(RGB(0xFA, 0xF8, 0xEF));
    FillRect(mem, &bg, bgBr);
    DeleteObject(bgBr);

    // 根据行列数动态计算格子大小, 棋盘居中
    int areaW = WIN_W - MARGIN * 2;
    int cw = (areaW - (C - 1) * GAP) / C;
    int ch = (BOARD_H - (R - 1) * GAP) / R;
    int cell = cw < ch ? cw : ch;
    int boardW = C * cell + (C - 1) * GAP;
    int boardH = R * cell + (R - 1) * GAP;
    int ox = MARGIN + (areaW - boardW) / 2;
    int oy = TOPBAR + (BOARD_H - boardH) / 2;

    RECT panel = {ox, oy, ox + boardW, oy + boardH};
    HBRUSH panelBr = CreateSolidBrush(RGB(0xBB, 0xAD, 0xA0));
    FillRect(mem, &panel, panelBr);
    DeleteObject(panelBr);

    for (int i = 0; i < R; i++)
        for (int j = 0; j < C; j++) {
            int x = ox + j * (cell + GAP);
            int y = oy + i * (cell + GAP);
            drawTile(mem, x, y, cell, board[i][j]);
        }

    SetBkMode(mem, TRANSPARENT);
    SetTextColor(mem, RGB(0x77, 0x6E, 0x65));
    HFONT sf = CreateFont(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Arial"));
    HFONT oldF = (HFONT)SelectObject(mem, sf);
    wchar_t sbuf[64];
    swprintf(sbuf, 64, L"当前棋盘: %d x %d    得分: %lld", R, C, score);
    RECT src = {MARGIN, WIN_H - 24, WIN_W - MARGIN, WIN_H - 4};
    DrawTextW(mem, sbuf, -1, &src, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

    HFONT hf = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Microsoft YaHei"));
    SelectObject(mem, hf);
    RECT hrc = {MARGIN, 76, WIN_W - MARGIN, 96};
    DrawTextW(mem, L"方向键/WASD 移动    R 用当前设置重开    Esc 退出", -1, &hrc,
              DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    SelectObject(mem, oldF);
    DeleteObject(sf);
    DeleteObject(hf);

    BitBlt(hdc, 0, 0, WIN_W, WIN_H, mem, 0, 0, SRCCOPY);
    SelectObject(mem, oldBmp);
    DeleteObject(bmp);
    DeleteDC(mem);
    EndPaint(hwnd, &ps);
}

void startGame(int rows, int cols) {
    if (rows < MIN_N) rows = MIN_N;
    if (rows > MAX_N) rows = MAX_N;
    if (cols < MIN_N) cols = MIN_N;
    if (cols > MAX_N) cols = MAX_N;
    R = rows;
    C = cols;
    init();
    addTile();
    addTile();
    InvalidateRect(g_hwnd, NULL, FALSE);
}

void afterMove() {
    addTile();
    InvalidateRect(g_hwnd, NULL, FALSE);
    if (!winShown && hasWon()) {
        winShown = true;
        MessageBoxW(g_hwnd, L"恭喜! 你合成了 2048! 继续挑战更高分吧~",
                    L"胜利", MB_ICONINFORMATION | MB_OK);
    }
    if (!canMove()) {
        gameOver = true;
        wchar_t buf[128];
        swprintf(buf, 128, L"游戏结束! 最终得分: %lld\n\n按 R 用当前设置重开, 或按 Esc 退出。", score);
        MessageBoxW(g_hwnd, buf, L"游戏结束", MB_ICONINFORMATION | MB_OK);
    }
}

void updateLabels() {
    wchar_t buf[16];
    swprintf(buf, 16, L"%d 行", (int)SendMessageW(g_tbRows, TBM_GETPOS, 0, 0));
    SetWindowTextW(g_lbRows, buf);
    swprintf(buf, 16, L"%d 列", (int)SendMessageW(g_tbCols, TBM_GETPOS, 0, 0));
    SetWindowTextW(g_lbCols, buf);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_PAINT:
            drawBoard(hwnd);
            return 0;
        case WM_HSCROLL:
            updateLabels();
            return 0;
        case WM_COMMAND:
            if (LOWORD(wp) == ID_BTN_START) {
                int r = (int)SendMessageW(g_tbRows, TBM_GETPOS, 0, 0);
                int c = (int)SendMessageW(g_tbCols, TBM_GETPOS, 0, 0);
                startGame(r, c);
                SetFocus(hwnd); // 把键盘焦点还给主窗口, 方便直接操作
            }
            return 0;
        case WM_KEYDOWN:
            if (gameOver) {
                if (wp == 'R') startGame(R, C);
                else if (wp == VK_ESCAPE) DestroyWindow(hwnd);
                return 0;
            }
            switch (wp) {
                case VK_LEFT: case 'A': if (doMove(0)) afterMove(); return 0;
                case VK_RIGHT: case 'D': if (doMove(1)) afterMove(); return 0;
                case VK_UP: case 'W': if (doMove(2)) afterMove(); return 0;
                case VK_DOWN: case 'S': if (doMove(3)) afterMove(); return 0;
                case 'R': startGame(R, C); return 0;
                case VK_ESCAPE: DestroyWindow(hwnd); return 0;
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

HWND makeLabel(HINSTANCE hInst, HWND hwnd, const wchar_t* text, int x, int y, int w, int h) {
    return CreateWindowW(L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT,
                         x, y, w, h, hwnd, NULL, hInst, NULL);
}

HWND makeTrackbar(HINSTANCE hInst, HWND hwnd, int id, int x, int y, int w, int def) {
    HWND tb = CreateWindowW(TRACKBAR_CLASSW, L"",
                            WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_HORZ,
                            x, y, w, 30, hwnd, (HMENU)(INT_PTR)id, hInst, NULL);
    SendMessageW(tb, TBM_SETRANGE, TRUE, MAKELONG(MIN_N, MAX_N));
    SendMessageW(tb, TBM_SETTICFREQ, 1, 0);
    SendMessageW(tb, TBM_SETPOS, TRUE, def);
    return tb;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, int nCmdShow) {
    srand((unsigned)time(NULL));
    InitCommonControls();

    WNDCLASSW wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"Game2048NM";
    RegisterClassW(&wc);

    RECT rc = {0, 0, WIN_W, WIN_H};
    AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, FALSE);
    g_hwnd = CreateWindowW(L"Game2048NM", L"2048 (N*M)",
                           WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                           CW_USEDEFAULT, CW_USEDEFAULT,
                           rc.right - rc.left, rc.bottom - rc.top,
                           NULL, NULL, hInst, NULL);
    if (!g_hwnd) return 1;

    // 设置区: 行数滑条 / 列数滑条 / 开始按钮
    HFONT lf = CreateFont(17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                          DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                          DEFAULT_QUALITY, DEFAULT_PITCH, TEXT("Microsoft YaHei"));

    HWND lb = makeLabel(hInst, g_hwnd, L"行数", MARGIN, 20, 40, 24);
    g_tbRows = makeTrackbar(hInst, g_hwnd, ID_TB_ROWS, MARGIN + 48, 16, 300, R);
    g_lbRows = makeLabel(hInst, g_hwnd, L"", MARGIN + 356, 20, 70, 24);

    HWND lb2 = makeLabel(hInst, g_hwnd, L"列数", MARGIN, 54, 40, 24);
    g_tbCols = makeTrackbar(hInst, g_hwnd, ID_TB_COLS, MARGIN + 48, 50, 300, C);
    g_lbCols = makeLabel(hInst, g_hwnd, L"", MARGIN + 356, 54, 70, 24);

    HWND btn = CreateWindowW(L"BUTTON", L"开始新游戏",
                             WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                             WIN_W - MARGIN - 150, 22, 150, 52,
                             g_hwnd, (HMENU)(INT_PTR)ID_BTN_START, hInst, NULL);

    // 统一设置字体
    HWND ctrls[7] = {lb, g_tbRows, g_lbRows, lb2, g_tbCols, g_lbCols, btn};
    for (int i = 0; i < 7; i++)
        SendMessageW(ctrls[i], WM_SETFONT, (WPARAM)lf, TRUE);

    updateLabels();
    startGame(R, C);

    ShowWindow(g_hwnd, nCmdShow);
    UpdateWindow(g_hwnd);

    MSG msg;
    while (GetMessageW(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return 0;
}
