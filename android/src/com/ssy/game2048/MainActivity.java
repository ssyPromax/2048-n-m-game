// 2048 (N*M) - Android 版
// 行数 / 列数各用一个滑动条在 2~10 之间选择
// 操作: 在棋盘上滑动移动方块, 点「开始新游戏」按当前设置重开
package com.ssy.game2048;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.graphics.Canvas;
import android.graphics.Color;
import android.graphics.Paint;
import android.os.Bundle;
import android.view.MotionEvent;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.SeekBar;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.List;
import java.util.Random;

public class MainActivity extends Activity {
    static final int MIN_N = 2, MAX_N = 10, WIN_TILE = 2048;

    int rows = 4, cols = 4;
    int[][] board = new int[MAX_N][MAX_N];
    long score = 0;
    boolean winShown = false, gameOver = false;
    final Random rand = new Random();

    BoardView boardView;
    TextView scoreText;
    SeekBar tbRows, tbCols;
    TextView lbRows, lbCols;

    // ---------------- 游戏逻辑 ----------------

    void init() {
        for (int i = 0; i < MAX_N; i++)
            for (int j = 0; j < MAX_N; j++)
                board[i][j] = 0;
        score = 0;
        winShown = false;
        gameOver = false;
    }

    void addTile() {
        List<Integer> ei = new ArrayList<Integer>(), ej = new ArrayList<Integer>();
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                if (board[i][j] == 0) { ei.add(i); ej.add(j); }
        if (ei.isEmpty()) return;
        int k = rand.nextInt(ei.size());
        board[ei.get(k)][ej.get(k)] = rand.nextInt(10) == 0 ? 4 : 2;
    }

    boolean canMove() {
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++) {
                if (board[i][j] == 0) return true;
                if (j + 1 < cols && board[i][j] == board[i][j + 1]) return true;
                if (i + 1 < rows && board[i][j] == board[i + 1][j]) return true;
            }
        return false;
    }

    int mergeLine(int[] line, int len) {
        int[] tmp = new int[len];
        int t = 0;
        for (int i = 0; i < len; i++)
            if (line[i] != 0) tmp[t++] = line[i];
        int gain = 0;
        for (int i = 0; i + 1 < t; i++)
            if (tmp[i] != 0 && tmp[i] == tmp[i + 1]) {
                tmp[i] *= 2;
                gain += tmp[i];
                tmp[i + 1] = 0;
            }
        for (int i = 0; i < len; i++) line[i] = 0;
        t = 0;
        for (int i = 0; i < len; i++)
            if (tmp[i] != 0) line[t++] = tmp[i];
        return gain;
    }

    boolean doMove(int dir) { // 0=左 1=右 2=上 3=下
        int[][] before = new int[rows][cols];
        for (int i = 0; i < rows; i++)
            System.arraycopy(board[i], 0, before[i], 0, cols);
        int lines = (dir <= 1) ? rows : cols;
        int len = (dir <= 1) ? cols : rows;
        for (int k = 0; k < lines; k++) {
            int[] line = new int[len];
            for (int i = 0; i < len; i++) {
                switch (dir) {
                    case 0: line[i] = board[k][i]; break;
                    case 1: line[i] = board[k][len - 1 - i]; break;
                    case 2: line[i] = board[i][k]; break;
                    default: line[i] = board[len - 1 - i][k]; break;
                }
            }
            score += mergeLine(line, len);
            for (int i = 0; i < len; i++) {
                switch (dir) {
                    case 0: board[k][i] = line[i]; break;
                    case 1: board[k][len - 1 - i] = line[i]; break;
                    case 2: board[i][k] = line[i]; break;
                    default: board[len - 1 - i][k] = line[i]; break;
                }
            }
        }
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                if (before[i][j] != board[i][j]) return true;
        return false;
    }

    boolean hasWon() {
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                if (board[i][j] >= WIN_TILE) return true;
        return false;
    }

    void startGame(int r, int c) {
        if (r < MIN_N) r = MIN_N;
        if (r > MAX_N) r = MAX_N;
        if (c < MIN_N) c = MIN_N;
        if (c > MAX_N) c = MAX_N;
        rows = r;
        cols = c;
        init();
        addTile();
        addTile();
        updateScore();
        boardView.invalidate();
    }

    void afterMove() {
        addTile();
        updateScore();
        boardView.invalidate();
        if (!winShown && hasWon()) {
            winShown = true;
            new AlertDialog.Builder(this)
                    .setTitle("胜利")
                    .setMessage("恭喜! 你合成了 2048! 继续挑战更高分吧~")
                    .setPositiveButton("继续", null)
                    .show();
        }
        if (!canMove()) {
            gameOver = true;
            new AlertDialog.Builder(this)
                    .setTitle("游戏结束")
                    .setMessage("最终得分: " + score)
                    .setPositiveButton("再来一局", (dialog, which) -> startGame(rows, cols))
                    .setNegativeButton("退出", (dialog, which) -> finish())
                    .show();
        }
    }

    void updateScore() {
        scoreText.setText("当前棋盘: " + rows + " x " + cols + "    得分: " + score);
    }

    // ---------------- 界面 ----------------

    static int tileColor(int v) {
        switch (v) {
            case 2:    return 0xFFEEE4DA;
            case 4:    return 0xFFEDE0C8;
            case 8:    return 0xFFF2B179;
            case 16:   return 0xFFF59563;
            case 32:   return 0xFFF67C5F;
            case 64:   return 0xFFF65E3B;
            case 128:  return 0xFFEDCF72;
            case 256:  return 0xFFEDCC61;
            case 512:  return 0xFFEDC850;
            case 1024: return 0xFFEDC53F;
            case 2048: return 0xFFEDC22E;
            default:   return 0xFF3C3A32;
        }
    }

    class BoardView extends View {
        final Paint paint = new Paint(Paint.ANTI_ALIAS_FLAG);
        float downX, downY;

        public BoardView(Context ctx) { super(ctx); }

        @Override
        protected void onDraw(Canvas cv) {
            int w = getWidth(), h = getHeight();
            cv.drawColor(0xFFFAF8EF);

            int gap = 8;
            int cw = (w - gap * (cols - 1)) / cols;
            int ch = (h - gap * (rows - 1)) / rows;
            int cell = Math.min(cw, ch);
            int boardW = cols * cell + (cols - 1) * gap;
            int boardH = rows * cell + (rows - 1) * gap;
            int ox = (w - boardW) / 2;
            int oy = (h - boardH) / 2;

            paint.setColor(0xFFBBADA0);
            cv.drawRect(ox, oy, ox + boardW, oy + boardH, paint);

            paint.setTextAlign(Paint.Align.CENTER);
            for (int i = 0; i < rows; i++)
                for (int j = 0; j < cols; j++) {
                    int x = ox + j * (cell + gap);
                    int y = oy + i * (cell + gap);
                    int v = board[i][j];
                    paint.setColor(v == 0 ? 0xFFCDC1B4 : tileColor(v));
                    cv.drawRect(x, y, x + cell, y + cell, paint);
                    if (v == 0) continue;
                    boolean dark = v <= 4;
                    paint.setColor(dark ? 0xFF776E65 : 0xFFFFFFFF);
                    int len = String.valueOf(v).length();
                    float pt = cell / 3f;
                    if (len == 3) pt = cell * 2 / 7f;
                    else if (len >= 4) pt = cell / 4f;
                    paint.setTextSize(pt);
                    cv.drawText(String.valueOf(v), x + cell / 2f,
                            y + cell / 2f - (paint.descent() + paint.ascent()) / 2f, paint);
                }
        }

        @Override
        public boolean onTouchEvent(MotionEvent ev) {
            switch (ev.getAction()) {
                case MotionEvent.ACTION_DOWN:
                    downX = ev.getX();
                    downY = ev.getY();
                    return true;
                case MotionEvent.ACTION_UP:
                    float dx = ev.getX() - downX;
                    float dy = ev.getY() - downY;
                    if (Math.max(Math.abs(dx), Math.abs(dy)) < 24) return true;
                    int dir;
                    if (Math.abs(dx) > Math.abs(dy))
                        dir = dx > 0 ? 1 : 0;
                    else
                        dir = dy > 0 ? 3 : 2;
                    if (!gameOver && doMove(dir)) afterMove();
                    return true;
            }
            return true;
        }
    }

    int dp(int v) {
        return (int) (v * getResources().getDisplayMetrics().density + 0.5f);
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(dp(16), dp(12), dp(16), dp(8));

        // 行数设置行
        LinearLayout rowA = new LinearLayout(this);
        rowA.setOrientation(LinearLayout.HORIZONTAL);
        TextView t1 = new TextView(this);
        t1.setText("行数");
        t1.setTextSize(16);
        tbRows = new SeekBar(this);
        tbRows.setMax(MAX_N - MIN_N);
        tbRows.setProgress(rows - MIN_N);
        lbRows = new TextView(this);
        lbRows.setTextSize(16);
        rowA.addView(t1, new LinearLayout.LayoutParams(dp(48), LinearLayout.LayoutParams.WRAP_CONTENT));
        rowA.addView(tbRows, new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
        rowA.addView(lbRows, new LinearLayout.LayoutParams(dp(56), LinearLayout.LayoutParams.WRAP_CONTENT));

        // 列数设置行
        LinearLayout rowB = new LinearLayout(this);
        rowB.setOrientation(LinearLayout.HORIZONTAL);
        TextView t2 = new TextView(this);
        t2.setText("列数");
        t2.setTextSize(16);
        tbCols = new SeekBar(this);
        tbCols.setMax(MAX_N - MIN_N);
        tbCols.setProgress(cols - MIN_N);
        lbCols = new TextView(this);
        lbCols.setTextSize(16);
        rowB.addView(t2, new LinearLayout.LayoutParams(dp(48), LinearLayout.LayoutParams.WRAP_CONTENT));
        rowB.addView(tbCols, new LinearLayout.LayoutParams(0, LinearLayout.LayoutParams.WRAP_CONTENT, 1));
        rowB.addView(lbCols, new LinearLayout.LayoutParams(dp(56), LinearLayout.LayoutParams.WRAP_CONTENT));

        SeekBar.OnSeekBarChangeListener listener = new SeekBar.OnSeekBarChangeListener() {
            @Override public void onProgressChanged(SeekBar sb, int p, boolean fromUser) {
                lbRows.setText((tbRows.getProgress() + MIN_N) + " 行");
                lbCols.setText((tbCols.getProgress() + MIN_N) + " 列");
            }
            @Override public void onStartTrackingTouch(SeekBar sb) { }
            @Override public void onStopTrackingTouch(SeekBar sb) { }
        };
        tbRows.setOnSeekBarChangeListener(listener);
        tbCols.setOnSeekBarChangeListener(listener);
        lbRows.setText(rows + " 行");
        lbCols.setText(cols + " 列");

        Button btn = new Button(this);
        btn.setText("开始新游戏");
        btn.setOnClickListener(v -> startGame(tbRows.getProgress() + MIN_N, tbCols.getProgress() + MIN_N));

        scoreText = new TextView(this);
        scoreText.setTextSize(15);

        boardView = new BoardView(this);

        TextView hint = new TextView(this);
        hint.setText("在棋盘上滑动移动方块");
        hint.setTextSize(13);

        root.addView(rowA, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        root.addView(rowB, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        LinearLayout.LayoutParams bp = new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT);
        bp.topMargin = dp(6); bp.bottomMargin = dp(6);
        root.addView(btn, bp);
        root.addView(scoreText, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));
        root.addView(boardView, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, 0, 1));
        root.addView(hint, new LinearLayout.LayoutParams(LinearLayout.LayoutParams.MATCH_PARENT, LinearLayout.LayoutParams.WRAP_CONTENT));

        setContentView(root);
        updateScore();
        startGame(rows, cols);
    }
}
