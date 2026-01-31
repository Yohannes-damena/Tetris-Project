#include <raylib.h>
#include <iostream>
#include <array>
#include <string>
#include <cstring>
#include <algorithm>
using namespace std;

// Tetrominoes shapes
enum Block
{
    BarBlock,
    BoxBlock,
    TBlock,
    LBlock,
    JBlock,
    ZBlock,
    SBlock
};
enum Orientation
{
    UP,
    RIGHT,
    DOWN,
    LEFT
};

// Tetrominos and colors (7 blocks)
const array<string, 7> tetromino = {
    "..X...X...X...X.", // BarBlock (I)
    ".XX..XX.........", // BoxBlock (O)
    ".X..XXX.........", // TBlock
    "..X...X...XX....", // LBlock
    ".X...X...XX.....", // JBlock
    ".X...XX...X.....", // ZBlock
    "..X..XX..X......"  // SBlock
};

struct ActiveBlock
{
    Block block;
    Orientation orientation;
    int x;
    int y;
    Color color;
    int colorIndex;
};

// Colors for the 7 tetrominoes
const Color purple = {166, 0, 247, 255};
const Color yellow = {237, 234, 4, 255};
const Color green = {47, 230, 23, 255};
const Color darkGrey = {26, 31, 40, 255};
const Color red = {232, 18, 18, 255};
const Color orange = {226, 116, 17, 255};
const Color cyan = {21, 204, 209, 255};
const Color blue = {13, 64, 216, 255};
const Color lightBlue = {59, 85, 162, 255};
const Color darkBlue = {44, 44, 127, 255};
// const Color blue = blue;
const Color PIECE_COLORS[7] = {purple, yellow, green, red, orange, cyan, blue};

// Board size
const int BOARD_WIDTH = 300;
const int BOARD_HEIGHT = 600;
const int INF_AREA = 250;
const int ROW_NB = 20;
const int COL_NB = 10;
// Each individual cell width and height.
const int CELL_WIDTH = BOARD_WIDTH / COL_NB;
const int CELL_HEIGHT = BOARD_HEIGHT / ROW_NB;
// Board (playfield position)
const int BOARD_X = 0;
const int BOARD_Y = 0;

// Scoreboard (UI panel position & size)
const int SCORE_X = BOARD_WIDTH;
const int SCORE_Y = 0;
const int SCORE_WIDTH = INF_AREA;
const int SCORE_HEIGHT = BOARD_HEIGHT;

int field[COL_NB * ROW_NB]; //[20 * 30]
bool hasActiveBlock = false;
bool gameEnd = false;
int score = 0;

// Default Prototypes

void drawGrid();
void drawBackground();
void playGame(ActiveBlock &activeBlock);
void drawActiveBlock(const ActiveBlock &activeBlock);
void spawnBlock(ActiveBlock &activeBlock);
bool blockCanGoDown(const ActiveBlock &activeBlock);
void lockActiveBlock(const ActiveBlock &activeBlock);
int Rotate(int x, int y, int r);
void movePiece(ActiveBlock &activeBlock);
bool DoesPieceFit(const ActiveBlock &activeBlock);
void drawLockedCells();
void gameOver();
int handleLineClearsAndScore();
void scoreBoard();

// Game Window Setup
void init()
{
    InitWindow(BOARD_WIDTH + INF_AREA, BOARD_HEIGHT, "TETRIS GAME");
    SetTargetFPS(60);
    // Clear the field to ensure no garbage blocks (-1 means empty)
    fill_n(field, COL_NB * ROW_NB, -1);
}

int main()
{
    init();
    ActiveBlock activeBlock; // Init out struct Activeblock
    while (!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(darkBlue);
        drawBackground();
        drawGrid();
        if (!gameEnd)
        {
            movePiece(activeBlock);
            playGame(activeBlock);
            DoesPieceFit(activeBlock);
            handleLineClearsAndScore();
            scoreBoard();
        }
        else
        {
            // Still draw locked cells and show Game Over message
            drawLockedCells();
            gameOver();
        }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
// Function for the grids
void drawBackground()
{
    // Board background
    DrawRectangle(
        BOARD_X,
        BOARD_Y,
        BOARD_WIDTH,
        BOARD_HEIGHT,
        darkGrey);
    // Scoreboard background
    DrawRectangle(
        SCORE_X,
        SCORE_Y,
        SCORE_WIDTH,
        SCORE_HEIGHT,
        darkBlue);
}

void drawGrid()
{
    int startPosX = 0;
    int startPosY = 0;
    for (int row = 0; row <= ROW_NB; row++)
    { // y coordinate
        DrawLine(startPosX, row * CELL_HEIGHT, BOARD_WIDTH, row * CELL_HEIGHT, blue);
    }
    for (int col = 0; col <= COL_NB; col++)
    { // x coordinate
        DrawLine(col * CELL_WIDTH, startPosY, col * CELL_WIDTH, BOARD_HEIGHT, blue);
    }
}

// function for the spawn blocks
void spawnBlock(ActiveBlock &activeBlock)
{
    activeBlock.block = Block(GetRandomValue(BarBlock, SBlock));
    activeBlock.orientation = Orientation(GetRandomValue(UP, LEFT));
    activeBlock.x = COL_NB / 2 - 2;
    activeBlock.y = 0;
    // If the spawned piece immediately overlaps locked cells, set game over
    for (int px = 0; px < 4; px++)
    {
        for (int py = 0; py < 4; py++)
        {
            int idx = Rotate(px, py, activeBlock.orientation);
            if (tetromino[activeBlock.block][idx] == 'X')
            {
                int fx = activeBlock.x + px;
                int fy = activeBlock.y + py;
                if (fx >= 0 && fx < COL_NB && fy >= 0 && fy < ROW_NB)
                {
                    if (field[fy * COL_NB + fx] != -1)
                    {
                        gameEnd = true;
                        return;
                    }
                }
            }
        }
    }
    // Use the block type as the color index for deterministic coloring
    activeBlock.colorIndex = static_cast<int>(activeBlock.block);
    activeBlock.color = PIECE_COLORS[activeBlock.colorIndex];
}
void drawActiveBlock(const ActiveBlock &activeBlock)
{
    for (int x = 0; x < 4; x++)
    {
        for (int y = 0; y < 4; y++)
        {
            int index = Rotate(x, y, activeBlock.orientation);
            if (tetromino[activeBlock.block][index] == 'X')
            {
                DrawRectangle(
                    (activeBlock.x + x) * CELL_WIDTH,
                    (activeBlock.y + y) * CELL_HEIGHT,
                    CELL_WIDTH,
                    CELL_HEIGHT,
                    activeBlock.color);
                DrawRectangleLines(
                    (activeBlock.x + x) * CELL_WIDTH,
                    (activeBlock.y + y) * CELL_HEIGHT,
                    CELL_WIDTH,
                    CELL_HEIGHT,
                    darkBlue);
            }
        }
    }
}
void playGame(ActiveBlock &activeBlock)
{
    // Simple drop timer (controls falling speed)
    static int dropCounter = 0;
    const int DROP_FRAMES = 30; // ~2 drops per second at 60 FPS

    if (!hasActiveBlock)
    {
        spawnBlock(activeBlock);
        hasActiveBlock = true;
        dropCounter = 0;
    }

    if (++dropCounter >= DROP_FRAMES)
    {
        dropCounter = 0;
        if (blockCanGoDown(activeBlock))
        {
            activeBlock.y++;
        }
        else
        {
            lockActiveBlock(activeBlock);
            hasActiveBlock = false;
        }
    }

    // Draw
    drawLockedCells();
    drawActiveBlock(activeBlock);
}
void gameOver()
{
    const char *title = "GAME OVER";
    const char *scoreLabel = "FINAL SCORE";

    int titleFontSize = 32;
    int scoreFontSize = 16;

    int titleWidth = MeasureText(title, titleFontSize);
    int scoreWidth = MeasureText(scoreLabel, scoreFontSize);

    int centerX = BOARD_WIDTH + (INF_AREA / 2);
    int spaceY = BOARD_WIDTH / 2;
    DrawText(
        title,
        centerX - titleWidth / 2,
        spaceY,
        titleFontSize,
        RED);

    DrawText(
        scoreLabel,
        centerX - scoreWidth / 2,
        spaceY + 50,
        scoreFontSize,
        DARKGRAY);

    DrawText(
        TextFormat("%d", score),
        centerX - MeasureText(TextFormat("%d", score), scoreFontSize) / 2,
        spaceY + 100,
        scoreFontSize,
        BLACK);
    DrawText(
        "Press ENTER to restart",
        centerX - MeasureText("Press ENTER to restart", scoreFontSize) / 2,
        spaceY + 150,
        scoreFontSize,
        RED);
    if (IsKeyPressed(KEY_ENTER))
    {
        fill_n(field, COL_NB * ROW_NB, -1);
        score = 0;
        hasActiveBlock = false;
        gameEnd = false;
    }
}
// Lock active block into the field
void lockActiveBlock(const ActiveBlock &activeBlock)
{
    for (int x = 0; x < 4; x++)
    {
        for (int y = 0; y < 4; y++)
        {
            int index = Rotate(x, y, activeBlock.orientation);
            if (tetromino[activeBlock.block][index] == 'X')
            {
                int fx = activeBlock.x + x;
                int fy = activeBlock.y + y;
                if (fx >= 0 && fx < COL_NB && fy >= 0 && fy < ROW_NB)
                {
                    field[fy * COL_NB + fx] = activeBlock.colorIndex;

                    // If any locked cell reaches the top row (y == 0), game over
                    if (fy <= 0)
                    {
                        gameEnd = true;
                    }
                }
            }
        }
    }
}
bool DoesPieceFit(const ActiveBlock &activeBlock)
{
    // Use the global field array
    for (int px = 0; px < 4; px++)
        for (int py = 0; py < 4; py++)
        {
            int index = Rotate(px, py, activeBlock.orientation);
            int fx = activeBlock.x + px;
            int fy = activeBlock.y + py;
            if (fx >= 0 && fx < COL_NB && fy >= 0 && fy < ROW_NB)
            {
                if (tetromino[activeBlock.block][index] == 'X' && field[fy * COL_NB + fx] != -1)
                    return false;
            }
            else
            {
                if (tetromino[activeBlock.block][index] == 'X')
                    return false;
            }
        }
    return true;
}
void movePiece(ActiveBlock &activeBlock)
{
    if (gameEnd)
        return;

    ActiveBlock test = activeBlock; // copy for testing

    if (IsKeyPressed(KEY_LEFT))
    {
        test.x--;
        if (DoesPieceFit(test))
            activeBlock.x--;
    }

    if (IsKeyPressed(KEY_RIGHT))
    {
        test.x++;
        if (DoesPieceFit(test))
            activeBlock.x++;
    }

    if (IsKeyPressed(KEY_DOWN))
    {
        test.y++;
        if (DoesPieceFit(test))
            activeBlock.y++;
    }

    if (IsKeyPressed(KEY_UP))
    {
        test.orientation = Orientation((test.orientation + 1) % 4);
        if (DoesPieceFit(test))
            activeBlock.orientation = test.orientation;
    }

    if (IsKeyPressed(KEY_SPACE)) // hard drop
    {
        while (true)
        {
            test = activeBlock;
            test.y++;
            if (!DoesPieceFit(test))
                break;
            activeBlock.y++;
        }
    }
}

// Draw locked cells
void drawLockedCells()
{
    for (int r = 0; r < ROW_NB; r++)
    {
        for (int c = 0; c < COL_NB; c++)
        {
            int idx = field[r * COL_NB + c];
            if (idx != -1)
            {
                DrawRectangle(c * CELL_WIDTH, r * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT, PIECE_COLORS[idx]);
                DrawRectangleLines(c * CELL_WIDTH, r * CELL_HEIGHT, CELL_WIDTH, CELL_HEIGHT, BLACK);
            }
        }
    }
}

bool blockCanGoDown(const ActiveBlock &activeBlock)
{
    for (int x = 0; x < 4; x++)
    {
        for (int y = 0; y < 4; y++)
        {
            int index = Rotate(x, y, activeBlock.orientation);
            if (tetromino[activeBlock.block][index] == 'X')
            {
                int newX = activeBlock.x + x;
                int newY = activeBlock.y + y + 1;
                if (newY >= ROW_NB)
                {
                    return false;
                }
                if (newX < 0 || newX >= COL_NB)
                {
                    return false;
                }
                if (field[newY * COL_NB + newX] != -1)
                { // Collision with a locked block.
                    return false;
                }
            }
        }
    }
    return true;
}

int Rotate(int x, int y, int r)
{
    switch (r % 4)
    {
    case 0:
        return y * 4 + x;
    case 1:
        return 12 + y - (x * 4);
    case 2:
        return 15 - (y * 4) - x;
    case 3:
        return 3 - y + (x * 4);
    }
    return 0;
}

// Scoreboard and block clearing logic

int handleLineClearsAndScore()
{
    int linesCleared = 0;

    // Scan from bottom to top
    for (int row = ROW_NB - 1; row >= 0; row--)
    {
        bool full = true;

        for (int col = 0; col < COL_NB; col++)
        {
            if (field[row * COL_NB + col] == -1)
            {
                full = false;
                break;
            }
        }

        if (full)
        {
            // Move everything above down
            for (int y = row; y > 0; y--)
            {
                for (int col = 0; col < COL_NB; col++)
                {
                    field[y * COL_NB + col] =
                        field[(y - 1) * COL_NB + col];
                }
            }

            // Clear top row
            for (int col = 0; col < COL_NB; col++)
                field[col] = -1;

            linesCleared++;
            row++; // recheck same row after collapse
        }
    }

    // Scoring
    switch (linesCleared)
    {
    case 1:
        score += 100;
        break;
    case 2:
        score += 300;
        break;
    case 3:
        score += 500;
        break;
    case 4:
        score += 800;
        break;
    default:
        break;
    }

    return linesCleared;
}
void scoreBoard()
{
    DrawText("SCORE", SCORE_X + 40, SCORE_Y + 80, 28, BLACK);
    DrawText(
        TextFormat("%d", score),
        SCORE_X + 40,
        SCORE_Y + 40,
        28,
        BLACK);
}
