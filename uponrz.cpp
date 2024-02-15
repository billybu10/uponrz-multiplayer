#include <iostream>
#include <raylib.h>
#include <deque>
#include <raymath.h>

using namespace std;

static bool allowMove = false;

int cellSize = 20;
int cellCount = 30;
int offset = 75;

double lastUpdateTime = 0;
bool pause = false;
bool startScr = true;
bool multi = false;

bool ElementInDeque(Vector2 element, deque<Vector2> deque)
{
    for (unsigned int i = 0; i < deque.size(); i++)
    {
        if (Vector2Equals(deque[i], element))
        {
            return true;
        }
    }
    return false;
}

bool EventTriggered(double interval)
{
    double currentTime = GetTime();
    if (currentTime - lastUpdateTime >= interval)
    {
          lastUpdateTime = currentTime;
          return true;
      }
      return false;
  }

  class Snake
  {
  public:
      deque<Vector2> body;
      Vector2 direction = {1, 0};
      bool addSegment = false;
      int score = 0;
      int scoreOffset = 0;
      Color color;
      int headposx, headposy;

      Snake(int headposx, int headposy, bool second){
        this->color = second ? RED : BLUE;
        this->headposy = headposy;
        this->headposx = headposx;
        if(second) {this->scoreOffset = 100;}
        else {this->scoreOffset = 0;}
        body =  {Vector2{headposx, headposy}, Vector2{headposx-1, headposy}, Vector2{headposx-2, headposy}};
    }

    void Draw()
    {
        for (unsigned int i = 0; i < body.size(); i++)
        {
            float x = body[i].x;
            float y = body[i].y;
            Rectangle segment = Rectangle{offset + x * cellSize, offset + y * cellSize, (float)cellSize, (float)cellSize};
            DrawRectangleRounded(segment, 0.5, 6, color);
            DrawText(TextFormat("%i", score), offset - 4 + scoreOffset, offset + cellSize * cellCount + 10, 40, color);
        }
    }

    void Update()
    {
        body.push_front(Vector2Add(body[0], direction));
        if (addSegment == true)
        {
            addSegment = false;
        }
        else
        {
            body.pop_back();
        }
    }

    void Reset()
    {
        body =  {Vector2{headposx, headposy}, Vector2{headposx-1, headposy}, Vector2{headposx-2, headposy}};
        direction = {1, 0};
    }
};

class Food
{

public:
    Vector2 position;
    Texture2D texture;

    Food(deque<Vector2> snakeBody)
    {
        Image image = LoadImage("food.png");
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
        position = GenerateRandomPos(snakeBody);
    }

    ~Food()
    {
        UnloadTexture(texture);
    }

    void Draw()
    {
        DrawTexture(texture, offset + position.x * cellSize, offset + position.y * cellSize, WHITE);
    }

    Vector2 GenerateRandomCell()
    {
        float x = GetRandomValue(0, cellCount - 1);
        float y = GetRandomValue(0, cellCount - 1);
        return Vector2{x, y};
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody))
        {
            position = GenerateRandomCell();
        }
        return position;
    }

    Vector2 GenerateRandomPos(deque<Vector2> snakeBody, deque<Vector2> snakeBody2)
    {
        Vector2 position = GenerateRandomCell();
        while (ElementInDeque(position, snakeBody) || ElementInDeque(position, snakeBody2))
        {
            position = GenerateRandomCell();
        }
        return position;
    }
};

class Game
{
public:
    Snake snake = Snake(6, 9, false);
    Snake *snake2 = NULL;
    Food food = Food(snake.body);
    bool running = true;
    int score = 0;

    Game(){
      if(multi) 
      {
        snake2 = new Snake(6, 16, true);
      }
    }

    ~Game(){
      delete snake2;
    }

    void Draw()
    {
        food.Draw();
        snake.Draw();
        if(multi) snake2->Draw();
    }

    void Update()
    {
        if (running)
        {
            snake.Update();
            if(multi) 
            {
                snake2->Update();
                CheckColllisionBetween();
            }
            CheckCollisionWithFood();
            CheckCollisionWithEdges();
            CheckCollisionWithTail();
        }
    }

    void CheckCollisionWithFood()
    {
        
        if (multi) {
            if (Vector2Equals(snake2->body[0], food.position))
            {
                food.position = food.GenerateRandomPos(snake.body, snake2->body);
                snake2->addSegment = true;
                snake2->score++;
            }
            if (Vector2Equals(snake.body[0], food.position))
            {
                food.position = food.GenerateRandomPos(snake.body, snake2->body);
                snake.addSegment = true;
                snake.score++;
            }
        }else{
            if (Vector2Equals(snake.body[0], food.position))
            {
                food.position = food.GenerateRandomPos(snake.body);
                snake.addSegment = true;
                snake.score++;
            }
        }
    }

    void CheckCollisionWithEdges()
    {
        if (snake.body[0].x == cellCount || snake.body[0].x == -1)
        {
            GameOver();
        }
          if (snake.body[0].y == cellCount || snake.body[0].y == -1)
          {
              GameOver();
          }

        if(multi){
            if (snake2->body[0].x == cellCount || snake2->body[0].x == -1)
            {
                GameOver2();
            }
            if (snake2->body[0].y == cellCount || snake2->body[0].y == -1)
            {
                GameOver2();
            }
        }
    }

    void GameOver()
    {
        snake.Reset();
        if(multi) {
            snake2->Reset();
            food.position = food.GenerateRandomPos(snake.body, snake2->body);
        }else food.position = food.GenerateRandomPos(snake.body);
        running = false;
        snake.score = 0;
    }

    void GameOver2()
    {
        snake2->Reset();
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body, snake2->body);
        running = false;
        snake2->score = 0;
    }

    void GameOverBoth()
    {
        snake2->Reset();
        snake.Reset();
        food.position = food.GenerateRandomPos(snake.body, snake2->body);
        running = false;
        snake2->score = 0;
        snake.score = 0;
    }

    void CheckCollisionWithTail()
    {
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        if (ElementInDeque(snake.body[0], headlessBody))
        {
            GameOver();
        }
        if(multi){
            headlessBody = snake2->body;
            headlessBody.pop_front();
            if (ElementInDeque(snake2->body[0], headlessBody))
            {
                GameOver2();
            }
        }
    }
     
    void CheckColllisionBetween(){
        deque<Vector2> headlessBody = snake.body;
        headlessBody.pop_front();
        deque<Vector2> headlessBody2 = snake2->body;
        headlessBody2.pop_front();
        if (snake.body[0].x == snake2->body[0].x && snake.body[0].y == snake2->body[0].y) {
            GameOverBoth();
        }else if (ElementInDeque(snake2->body[0], headlessBody)) {
            GameOver2();
        }else if (ElementInDeque(snake.body[0], headlessBody2)) {
            GameOver();
        }
    }
};

int main()
{
    cout << "Starting the game..." << endl;
    InitWindow(2 * offset + cellSize * cellCount, 2 * offset + cellSize * cellCount, "Uponrz");
    SetTargetFPS(60);
    SetExitKey(KEY_ESCAPE); 

    while (startScr && WindowShouldClose() == false){
            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("Uponrz", offset - 4, 20, 100, WHITE);
            DrawText("Enter by uruchomic tryb jednoosobbowy", offset - 4, 160, 30, WHITE);
            DrawText("Spacja by uruchomic tryb dwuosobbowy", offset - 4, 190, 30, WHITE);
            DrawText("Sterowanie", offset - 4, 240, 30, WHITE);
            DrawText("Gracz pierwszy: strzalki", offset - 4, 270, 30, WHITE);
            DrawText("Gracz pierwszy: WASD", offset - 4, 300, 30, WHITE);
            if (IsKeyPressed(KEY_SPACE))
            {
                startScr = false;
                multi = true;
            }
            if (IsKeyPressed(KEY_ENTER))
            {
                startScr = false;
                multi = false;
            }
            EndDrawing();
    }

    Game game = Game();

    while (WindowShouldClose() == false)
    {
        BeginDrawing(); 

        if(!pause){
            if (EventTriggered(0.15))
            {
                allowMove = true;
                game.Update();
            }

            if (IsKeyPressed(KEY_UP) && game.snake.direction.y != 1 && allowMove)
            {
                game.snake.direction = {0, -1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_DOWN) && game.snake.direction.y != -1 && allowMove)
            {
                game.snake.direction = {0, 1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_LEFT) && game.snake.direction.x != 1 && allowMove)
            {
                game.snake.direction = {-1, 0};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_RIGHT) && game.snake.direction.x != -1 && allowMove)
            {
                game.snake.direction = {1, 0};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_W) && game.snake2->direction.y != 1 && allowMove && multi)
            {
                game.snake2->direction = {0, -1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_S) && game.snake2->direction.y != -1 && allowMove && multi)
            {
                game.snake2->direction = {0, 1};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_A) && game.snake2->direction.x != 1 && allowMove && multi)
            {
                game.snake2->direction = {-1, 0};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_D) && game.snake2->direction.x != -1 && allowMove && multi)
            {
                game.snake2->direction = {1, 0};
                game.running = true;
                allowMove = false;
            }
            if (IsKeyPressed(KEY_SPACE))
            {
                pause = true;
            }

            // Drawing
            ClearBackground(BLACK);
            DrawRectangleLinesEx(Rectangle{(float)offset - 4, (float)offset - 5, (float)cellSize * cellCount + 10, (float)cellSize * cellCount + 10}, 5, WHITE);
            DrawText("Uponrz", offset - 4, 20, 40, WHITE);
            game.Draw();
        }else{
            ClearBackground(BLACK);
            DrawText("GAME PAUSED", GetScreenWidth()/2 - MeasureText("GAME PAUSED", 40)/2, GetScreenWidth()/2 - 40, 40, GRAY);
            if (IsKeyPressed(KEY_SPACE))
            {
                pause = false;
            }
        }

        EndDrawing();
    }
    CloseWindow();
    return 0;
}
