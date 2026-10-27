#include <iostream>
#include <raylib.h>

using namespace std;

//--------------- CLASS: PLAYER BULLET
class Bullet {
public:
    Vector2 position;   // bullet position
    float speed;        // bullet speed
    bool active;

    // Default constructor for easy array creation
    Bullet() {
        position = { 0, 0 };
        speed = 15.0f;
        active = false;
    }

    void Update() {
        if (active) {
            position.y -= speed;
            if (position.y < 0) {
                active = false; // Disable when off-screen
            }
        }
    }

    void Draw() {
        if (active) {
            DrawRectangle(position.x, position.y, 4, 10, YELLOW);
        }
    }
};

//--------------- CLASS: ENEMY BULLET
class EnemyBullet {
public:
    Vector2 position;
    float speed;
    bool active;

    EnemyBullet() {
        position = { 0, 0 };
        speed = 10.0f;
        active = false;
    }

    void Update() {
        if (active) {
            position.y += speed;
            if (position.y > GetScreenHeight()) {
                active = false;
            }
        }
    }

    void Draw() {
        if (active) {
            DrawRectangle(position.x, position.y, 4, 12, RED);
        }
    }
};

//--------------- CLASS: ENEMY
class Enemy {
public:
    Vector2 position;
    float speed;
    bool active;

    Enemy() {
        position = { 0, 0 };
        speed = 3.0f;
        active = false;
    }

    void Update() {
        if (active) {
            position.y += speed;
            if (position.y > GetScreenHeight()) {
                active = false; // Enemy disappears if it reaches the bottom
            }
        }
    }

    void Draw(Texture2D texture) {
        if (active) {
            DrawTexture(texture, position.x, position.y, WHITE);
        }
    }
};


int main()
{
    InitWindow(720, 820, "Space Invaders");
    SetTargetFPS(30);

    Texture2D background = LoadTexture("background.png");
    Texture2D shipTexture = LoadTexture("plane.png");
    Texture2D enemyTexture = LoadTexture("enemy.png");

    float shipX = GetScreenWidth() / 2.0f - shipTexture.width / 2.0f;
    float shipY = GetScreenHeight() - shipTexture.height - 20;
    bool playerAlive = true;

    // Player bullets
    const int maxBullets = 20;
    Bullet bullets[maxBullets];

    // Enemies
    const int maxEnemies = 10;
    Enemy enemies[maxEnemies];

    // Enemy bullet
    EnemyBullet enemyBullet;

    // Timers
    double lastShootTime = 0;
    double shootInterval = 0.5;

    double lastEnemySpawn = 0;
    double enemySpawnInterval = 1.2; 

    double lastEnemyShootTime = 0;

    while (WindowShouldClose() == false)
    {
        // ------------------ GAME LOGIC ------------------
        if (playerAlive) {
            // Player movement
            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) shipX -= 7;
            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) shipX += 7;

            // Keep player on screen
            if (shipX < 0) shipX = 0;
            if (shipX > GetScreenWidth() - shipTexture.width)
                shipX = GetScreenWidth() - shipTexture.width;

            // Player shooting
            if (IsKeyPressed(KEY_SPACE) && GetTime() - lastShootTime >= shootInterval) {
                for (int i = 0; i < maxBullets; i++) {
                    if (!bullets[i].active) {
                        bullets[i].position = { shipX + shipTexture.width / 2.0f - 2, shipY };
                        bullets[i].active = true;
                        lastShootTime = GetTime();
                        break;
                    }
                }
            }

            // Enemy spawning
            if (GetTime() - lastEnemySpawn >= enemySpawnInterval) {
                for (int i = 0; i < maxEnemies; i++) {
                    if (!enemies[i].active) {
                        enemies[i].position = { (float)GetRandomValue(0, GetScreenWidth() - enemyTexture.width), -50 };
                        enemies[i].active = true;
                        lastEnemySpawn = GetTime();
                        break;
                    }
                }
            }

            // Random enemy shooting
            if (!enemyBullet.active && GetTime() - lastEnemyShootTime > 1.5) {
                // Find all active enemies
                int activeEnemiesList[maxEnemies];
                int activeEnemyCount = 0;
                for (int i = 0; i < maxEnemies; i++) {
                    if (enemies[i].active) {
                        activeEnemiesList[activeEnemyCount] = i;
                        activeEnemyCount++;
                    }
                }

                // If there are active enemies, pick a random one to shoot
                if (activeEnemyCount > 0) {
                    int shooterIndex = activeEnemiesList[GetRandomValue(0, activeEnemyCount - 1)];
                    enemyBullet.position = { enemies[shooterIndex].position.x + enemyTexture.width / 2.0f - 2, enemies[shooterIndex].position.y + enemyTexture.height };
                    enemyBullet.active = true;
                    lastEnemyShootTime = GetTime();
                }
            }

            // Update objects
            for (int i = 0; i < maxBullets; i++) bullets[i].Update();
            for (int i = 0; i < maxEnemies; i++) enemies[i].Update();
            enemyBullet.Update();

            // ------------------ COLLISIONS ------------------
            Rectangle playerRec = { shipX, shipY, (float)shipTexture.width, (float)shipTexture.height };

            for (int i = 0; i < maxEnemies; i++) {
                if (enemies[i].active) {
                    Rectangle enemyRec = { enemies[i].position.x, enemies[i].position.y, (float)enemyTexture.width, (float)enemyTexture.height };

                    // 1. Player bullet hits enemy
                    for (int j = 0; j < maxBullets; j++) {
                        if (bullets[j].active) {
                            Rectangle bulletRec = { bullets[j].position.x, bullets[j].position.y, 4, 10 };
                            
                            if (CheckCollisionRecs(bulletRec, enemyRec)) {
                                enemies[i].active = false; // Enemy dies
                                bullets[j].active = false;  // Bullet disappears
                            }
                        }
                    }

                    // 2. Enemy crashes into player
                    if (CheckCollisionRecs(playerRec, enemyRec)) {
                        playerAlive = false;
                    }
                }
            }

            // 3. Enemy bullet hits player
            if (enemyBullet.active) {
                Rectangle enemyBulletRec = { enemyBullet.position.x, enemyBullet.position.y, 4, 12 };
                if (CheckCollisionRecs(playerRec, enemyBulletRec)) {
                    playerAlive = false; // Player dies
                }
            }
        } 
        else {
            // GAME OVER - Press 'R' to restart
            if (IsKeyPressed(KEY_R)) {
                playerAlive = true;
                shipX = GetScreenWidth() / 2.0f - shipTexture.width / 2.0f;
                enemyBullet.active = false;
                for (int i = 0; i < maxEnemies; i++) enemies[i].active = false;
                for (int i = 0; i < maxBullets; i++) bullets[i].active = false;
            }
        }

        // ------------------ DRAWING ------------------
        BeginDrawing();
        ClearBackground(BLACK);

        // Draw background
        DrawTexture(background, 0, 0, WHITE);

        if (playerAlive) {
            DrawTexture(shipTexture, shipX, shipY, WHITE);

            for (int i = 0; i < maxBullets; i++) bullets[i].Draw();
            for (int i = 0; i < maxEnemies; i++) enemies[i].Draw(enemyTexture);
            enemyBullet.Draw();
        } 
        else {
            // Death screen
            DrawText("GAME OVER", GetScreenWidth() / 2 - 120, GetScreenHeight() / 2 - 50, 40, RED);
            DrawText("Press 'R' to Restart", GetScreenWidth() / 2 - 140, GetScreenHeight() / 2 + 10, 28, LIGHTGRAY);
        }

        EndDrawing();
    }

    // Free 
    UnloadTexture(enemyTexture);
    UnloadTexture(shipTexture);
    UnloadTexture(background);
    CloseWindow();

    return 0;
}