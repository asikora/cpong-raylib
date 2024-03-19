#include "raylib.h"

#define RAYGUI_IMPLEMENTATION
#include "raygui.h"

typedef struct Player {
    Texture2D texture;
    Vector2 pos;
    Vector2 velocity;
    Rectangle area;

}Player;

typedef struct Ball {
    Texture2D texture;
    Vector2 pos;
    Vector2 area;
    float radius;
    Vector2 velocity;
    int direction;
} Ball;

typedef enum GameState {
    GS_PVP,
    GS_PVE,
    GS_INMENU,
    GS_GAMEOVER,
    GS_QUIT,
    GS_NEWGAMEPVP,
    GS_NEWGAMEPVE
}GameState;

int p1_points = 0;
int p2_points = 0;

const int screenWidth = 800;
const int screenHeight = 450;
const int paddleHeight = 104;
const int ballRadius = 11;
const int gameOverPoints = 2;

const int playerVelocity = 300;
const int aiVelocity = 150;

Vector2 p1pos = {1, (screenHeight / 2) - (paddleHeight/2)};
Vector2 p2pos = {screenWidth-25, screenHeight / 2 - (paddleHeight/2)} ;
Vector2 ballStarPosition = {screenWidth / 2 - ballRadius, screenHeight / 2 - ballRadius};

Player p1,p2;
Ball ball;

bool p1MovingUp,p2MovingUp;
bool p1MovingDown,p2MovingDown;
bool isPVP;

GameState currentState = GS_INMENU;


Ball CreateBall(char *filename, Vector2 pos) {
    Ball b = {
        .pos = pos,
        .texture = LoadTexture(filename),
        .radius = ballRadius,
        .velocity = {500,0},
        .direction = 1
    };

    return b;
}

Player CreatePlayer(char *filename, Vector2 pos) {
    
    Player p = {
        .pos = pos,
        .velocity = {0,0},
        .texture = LoadTexture(filename),
    };

    p.area = (Rectangle) {
        p.pos.x,
        p.pos.y,
        p.texture.width,
        p.texture.height
    };

    return p;
}

void UpdateBallPosition(Ball *b, float delta) {

    b->pos.x += b->velocity.x * delta * b->direction;
    b->pos.y += b->velocity.y * delta;
    b->area.x = b->pos.x + b->radius;
    b->area.y = b->pos.y + b->radius;

}

void UpdatePlayerPosition(Player *p, float delta) {

    p->pos.y += p->velocity.y * delta;
    p->area.x = p->pos.x;
    p->area.y = p->pos.y;

}

void ResetBallPosition(Ball *ball, Vector2 startPos) {

    ball->pos = startPos;
    ball->velocity.y = 0;
}

void DrawLines(int screenWidth, int screenHeight, int length, int gap, float thick) {

    Vector2 startPos = {screenWidth/2, 0};
    Vector2 endPos = {screenWidth/2, length};

    int count = screenHeight / (length+gap);
    
    for (int i = 0;i<=count; i++) {
        DrawLineEx(startPos,endPos , 2, WHITE);
        startPos.y += length+gap;
        endPos.y = startPos.y + length;
    }    

}

void DrawPlayer(Player *p) {
    //DrawRectangleRec(p->area, RED);
    DrawTextureEx(p->texture, p->pos, 0, 1, WHITE);
}

void DrawBall(Ball *b) {
   //DrawCircle(b->area.x, b->area.y, b->radius, RED);
    DrawTextureEx(b->texture, b->pos, 0, 1, WHITE);
}

void HandleInput() {
    float delta = GetFrameTime();
    p1MovingDown = false;
    p1MovingUp = false;
    p2MovingDown = false;
    p2MovingUp = false;

    if (IsKeyDown(KEY_ESCAPE)) {
        currentState = GS_INMENU;
    }

    if (IsKeyDown(KEY_W)  && p1.pos.y >= 0) {
        p1.velocity.y = -playerVelocity;
        UpdatePlayerPosition(&p1, delta);
        p1MovingUp = true;

    }

    if (IsKeyDown(KEY_S)  && p1.pos.y < screenHeight - paddleHeight) {
        p1.velocity.y = playerVelocity;
        UpdatePlayerPosition(&p1, delta);
        p1MovingDown = true;

    }

    //Handle player 2 input if not in PVP then "AI" takes over
    if(currentState == GS_PVP) {

        if (IsKeyDown(KEY_UP) && p2.pos.y >= 0) {
            p2.velocity.y = -playerVelocity;
            UpdatePlayerPosition(&p2, delta);
            p2MovingUp = true;
        }

        if (IsKeyDown(KEY_DOWN) && p2.pos.y < screenHeight - paddleHeight) {
            p2.velocity.y = playerVelocity;
            UpdatePlayerPosition(&p2, delta);
            p2MovingDown = true;
        }
    }

    else {
        if(ball.pos.x  > 220) {
            
            if(ball.pos.y > p2.pos.y && p2.pos.y < screenHeight - paddleHeight) {
                p2.velocity.y = aiVelocity;
                UpdatePlayerPosition(&p2, delta);
            }

            if(ball.pos.y < p2.pos.y && p2.pos.y >= 0) {
                p2.velocity.y = -aiVelocity;
                UpdatePlayerPosition(&p2, delta);
            }
        }

    }

}

void HandlePhysics() {

    Rectangle top = {0,0, screenWidth, 1};
    Rectangle bottom = {0, screenHeight, screenWidth, 1};

    // Check for collision with player 1 paddle
    if(CheckCollisionCircleRec(ball.area, ball.radius, p1.area)) {
        
        ball.direction = -ball.direction;
        if (p1MovingUp) {
            ball.velocity.y = GetRandomValue(80,160);
        }
        else if (p1MovingDown) {
            ball.velocity.y = -GetRandomValue(80,160);
        }
        else {
            ball.velocity.y = GetRandomValue(-50,50);
        }
    }


    // Check for collision with player 2 paddle
    if (CheckCollisionCircleRec(ball.area, ball.radius, p2.area)) {

        ball.direction = -ball.direction;
        if (p2MovingUp) {
            ball.velocity.y = GetRandomValue(80,160);
        }
        else if (p2MovingDown) {
            ball.velocity.y = -GetRandomValue(80,160);
        }
        else {
            ball.velocity.y = GetRandomValue(-50,50);
        }


    }
    // Check for collision with top and bottom of the screen 
    if(CheckCollisionCircleRec(ball.area, ball.radius, top) || CheckCollisionCircleRec(ball.area, ball.radius, bottom)) {
        ball.velocity.y = -ball.velocity.y;
    }

    if (ball.pos.x > screenWidth)
    {
        p1_points++;
        ResetBallPosition(&ball, ballStarPosition);
    }

    if(ball.pos.x < 0 ) {
        p2_points++;
        ResetBallPosition(&ball, ballStarPosition);
    }

}

void HandleDraw() {

    float deltaTime = GetFrameTime();

    BeginDrawing();
        ClearBackground(BLACK);
        DrawLines(screenWidth, screenHeight, 25, 10, 2);
        DrawPlayer(&p1);
        DrawPlayer(&p2);
        UpdateBallPosition(&ball, deltaTime);
        
        DrawBall(&ball);

        const char *p1_points_text = TextFormat("Points %d", p1_points);
        const char *p2_points_text = TextFormat("Points %d", p2_points);
        const int topMargin = 5;
        const int leftMargin = 5;
        
        DrawText(p1_points_text, leftMargin, topMargin, 24, WHITE);
        DrawText(p2_points_text, screenWidth-MeasureText(p2_points_text, 24)-leftMargin, topMargin, 24, WHITE);

    EndDrawing(); 
}

void PlayGame(Player *p1, Player *p2, Ball *ball) {

    float deltaTime = GetFrameTime();

    HandleInput(deltaTime);
    HandlePhysics();
    HandleDraw(deltaTime);

    if (p1_points > gameOverPoints || p2_points > gameOverPoints) {
        currentState = GS_GAMEOVER;
    }
    
}

void GameOver() {
    char *game_over_text;
    currentState = GS_GAMEOVER;

    BeginDrawing();
        ClearBackground(BLACK);

        if (isPVP) {
            if (p1_points > p2_points) {
                game_over_text = "Player 1 wins!";
            }
            else
                game_over_text = "Player 2 wins!";
        }
        else {
            if(p2_points > p1_points) {
                game_over_text = "You lose!";
            }
            else {
                game_over_text = "You win!";
            }
        }

        DrawText(game_over_text, screenWidth/2 - MeasureText(game_over_text, 48) / 2, screenHeight / 2 - 48, 48, RED);

    EndDrawing();

    if(IsKeyDown(KEY_ESCAPE)) {
        currentState = GS_INMENU;
    }
}

void DrawMenu() {

    BeginDrawing();
        ClearBackground(BLACK);

        Rectangle playButtonRect = {screenWidth/2 - 70,screenHeight/2 - 20,140,40};
        Rectangle playPVEButtonRect = {screenWidth/2 - 70,screenHeight/2 + 40,140,40};
        Rectangle quitButtonRect = {screenWidth/2 - 70,screenHeight/2 + 100,140,40};

        const char *title = "CPong - raylib pong clone";
        DrawText(title, screenWidth/2 - MeasureText(title,  36) / 2, 36, 36, RED);

        GuiSetStyle(DEFAULT, TEXT_SIZE, 24);
        if(GuiButton(playButtonRect, "PLAY PVP")) {
           currentState = GS_NEWGAMEPVP;
        }
        if(GuiButton(playPVEButtonRect, "PLAY PVE")) {
            currentState = GS_NEWGAMEPVE;
        }
        if(GuiButton(quitButtonRect, "QUIT")){
            currentState = GS_QUIT;
        }

    EndDrawing();
}

int main()
{


    InitWindow(screenWidth, screenHeight, "CPong - Raylib");
    SetExitKey(KEY_NULL);

    SetTargetFPS(60);



    bool exitWindow = false;

    while (!WindowShouldClose())
    {

        switch (currentState) {
            case GS_NEWGAMEPVP:
                p1 = CreatePlayer("assets/paddleBlu.png", p1pos );
                p2 = CreatePlayer("assets/paddleRed.png", p2pos);
                ball = CreateBall("assets/ballGrey.png",ballStarPosition);
                p1_points = 0;
                p2_points = 0;
                currentState = GS_PVP;
                break;
            case GS_NEWGAMEPVE:
                p1 = CreatePlayer("assets/paddleBlu.png", p1pos );
                p2 = CreatePlayer("assets/paddleRed.png", p2pos);
                ball = CreateBall("assets/ballGrey.png",ballStarPosition);
                p1_points = 0;
                p2_points = 0;
                currentState = GS_PVE;
                break;
            case GS_PVP:
                isPVP = true;
                PlayGame(&p1, &p2, &ball);
                break;
            case GS_PVE:
                PlayGame(&p1, &p2, &ball);
                break;
            case GS_INMENU:
                DrawMenu();
                break;
            case GS_GAMEOVER:
                GameOver();
                break;
            case GS_QUIT:
                exitWindow = true;
                break;
            default:
                break;
        }

        if(exitWindow) {
            break;
        }

    }

    CloseWindow();

    return 0;
}