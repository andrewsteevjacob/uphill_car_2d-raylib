// fireworks.c
// Full Raylib fireworks show with stars, moon, grass, flower pots,
// rockets, multicolour spinners + “Happy Diwali” popping text.

#include "raylib.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265358979323846f

#define FLOWERPOTS 5
#define ROCKETS 4
#define SPINNERS 3

#define MAX_PARTICLES 200
#define MAX_FOUNTAIN 60
#define MAX_TRAIL 60
#define MAX_ROCKET_PARTICLES 250
#define SPINNER_SPARKS 120
#define STARS 120

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float life;
    Color color;
    bool active;
} Particle;

typedef struct {
    Vector2 pos;
    bool exploded;
    Particle fountain[MAX_FOUNTAIN];
    Particle explosion[MAX_PARTICLES];
} FlowerPot;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    bool launched;
    bool exploded;
    Particle trail[MAX_TRAIL];
    Particle boom[MAX_ROCKET_PARTICLES];
} Rocket;

typedef struct {
    Vector2 pos;
    float angle;
    bool active;
    Particle sparks[SPINNER_SPARKS];
} Spinner;

// Random multicolour
Color RandomColor() {
    return (Color){ rand()%256, rand()%256, rand()%256, 255 };
}

// Smooth rainbow color for spinner wheel
Color SpinnerWheelColor(float t) {
    return (Color){
        (int)(127 + 127 * sinf(t * 2)),
        (int)(127 + 127 * sinf(t * 2 + 2)),
        (int)(127 + 127 * sinf(t * 2 + 4)),
        255
    };
}

void SpawnFountainParticle(Particle *p, Vector2 o) {
    p->active = true;
    p->pos = o;
    p->vel.x = rand()%40 - 20;
    p->vel.y = -(150 + rand()%150);
    p->life = 0.3f + (rand()%30)/100.0f;
    p->color = ORANGE;
}

void SpawnExplosionParticle(Particle *p, Vector2 o, float minS, float maxS) {
    p->active = true;
    p->pos = o;
    float a = ((float)rand()/RAND_MAX) * 2 * PI;
    float s = minS + ((float)rand()/RAND_MAX)*(maxS - minS);
    p->vel.x = cosf(a)*s;
    p->vel.y = sinf(a)*s;
    p->life = 1.0f;
    p->color = RandomColor();
}

void SpawnTrailParticle(Particle *p, Vector2 o) {
    p->active = true;
    p->pos = o;
    p->vel.x = rand()%60 - 30;
    p->vel.y = rand()%60 - 30;
    p->life = 0.3f;
    p->color = YELLOW;
}

void SpawnSpinnerSpark(Particle *p, Vector2 o) {
    p->active = true;
    p->pos = o;
    float a = ((float)rand()/RAND_MAX)*2*PI;
    float s = 150 + rand()%150;
    p->vel.x = cosf(a)*s;
    p->vel.y = sinf(a)*s;
    p->life = 0.8f + (float)rand()/RAND_MAX*0.5f;
    p->color = RandomColor();
}

int main() {

    int W = 1280, H = 720;
    InitWindow(W, H, "Fireworks with Scenery + Happy Diwali");
    SetTargetFPS(60);
    srand(time(NULL));

    // ------------- STARS ----------------
    Vector2 stars[STARS];
    for (int i = 0; i < STARS; i++) {
        stars[i].x = rand() % W;
        stars[i].y = rand() % (int)(H * 0.6f);
    }

    // ------------- FLOWER POTS ----------------
    FlowerPot pots[FLOWERPOTS] = {0};
    for (int i = 0; i < FLOWERPOTS; i++)
        pots[i].pos = (Vector2){ (i+1) * (W/(FLOWERPOTS+1)), H - 120 };

    // ------------- ROCKETS ----------------
    Rocket rockets[ROCKETS] = {0};
    for (int r = 0; r < ROCKETS; r++)
        rockets[r].pos = (Vector2){ 80 + r*90, H - 120 };

    // ------------- SPINNERS ----------------
    Spinner spinners[SPINNERS] = {0};
    for (int s = 0; s < SPINNERS; s++)
        spinners[s].pos = (Vector2){ 200 + s * ((W - 400)/(SPINNERS - 1)), H - 60 };

    // ---------------------------------------------------------
    //                         MAIN LOOP
    // ---------------------------------------------------------
    while (!WindowShouldClose()) {

        float dt = GetFrameTime();
        float t = GetTime();

        // SPACE triggers fireworks
        if (IsKeyPressed(KEY_SPACE)) {

            for (int r = 0; r < ROCKETS; r++) {
                rockets[r].launched = true;
                rockets[r].vel = (Vector2){140 + rand()%80, -280 - rand()%120};
            }

            for (int f = 0; f < FLOWERPOTS; f++) {
                pots[f].exploded = true;
                for (int i = 0; i < MAX_PARTICLES; i++)
                    SpawnExplosionParticle(&pots[f].explosion[i], pots[f].pos, 120, 430);
            }

            for (int s = 0; s < SPINNERS; s++) {
                spinners[s].active = true;
                for (int i = 0; i < SPINNER_SPARKS; i++)
                    SpawnSpinnerSpark(&spinners[s].sparks[i], spinners[s].pos);
            }
        }

        // ---------------- UPDATE FLOWERPOTS ----------------
        for (int f = 0; f < FLOWERPOTS; f++) {
            FlowerPot *fp = &pots[f];

            if (!fp->exploded) {
                for (int i = 0; i < MAX_FOUNTAIN; i++) {
                    Particle *p = &fp->fountain[i];
                    if (!p->active && rand()%100 < 45)
                        SpawnFountainParticle(p, fp->pos);
                    if (p->active) {
                        p->life -= dt;
                        if (p->life <= 0) p->active = false;
                        p->pos.x += p->vel.x*dt;
                        p->pos.y += p->vel.y*dt;
                        p->vel.y += 260*dt;
                    }
                }
            }
            else {
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    Particle *p = &fp->explosion[i];
                    if (!p->active) continue;
                    p->life -= dt;
                    if (p->life <= 0) p->active = false;
                    p->vel.y += 300*dt;
                    p->pos.x += p->vel.x*dt;
                    p->pos.y += p->vel.y*dt;
                }
            }
        }

        // ---------------- UPDATE ROCKETS ----------------
        for (int r = 0; r < ROCKETS; r++) {
            Rocket *rk = &rockets[r];

            if (rk->launched && !rk->exploded) {

                rk->pos.x += rk->vel.x*dt;
                rk->pos.y += rk->vel.y*dt;
                rk->vel.y += 40*dt;

                for (int i = 0; i < MAX_TRAIL; i++) {
                    Particle *p = &rk->trail[i];
                    if (!p->active) {
                        if (rand()%100 < 50)
                            SpawnTrailParticle(p, rk->pos);
                        break;
                    }
                    p->life -= dt;
                    if (p->life <= 0) p->active = false;
                    p->pos.x += p->vel.x*dt;
                    p->pos.y += p->vel.y*dt;
                    p->vel.y += 80*dt;
                }

                if (rk->pos.y < H*0.25f) {
                    rk->exploded = true;
                    for (int i = 0; i < MAX_ROCKET_PARTICLES; i++)
                        SpawnExplosionParticle(&rk->boom[i], rk->pos, 150, 480);
                }
            }
            else if (rk->exploded) {
                for (int i = 0; i < MAX_ROCKET_PARTICLES; i++) {
                    Particle *p = &rk->boom[i];
                    if (!p->active) continue;
                    p->life -= dt;
                    if (p->life <= 0) p->active = false;
                    p->vel.y += 270*dt;
                    p->pos.x += p->vel.x*dt;
                    p->pos.y += p->vel.y*dt;
                }
            }
        }

        // ---------------- UPDATE SPINNERS ----------------
        for (int s = 0; s < SPINNERS; s++) {
            Spinner *sp = &spinners[s];
            if (!sp->active) continue;

            sp->angle += 14*dt*60;

            for (int i = 0; i < SPINNER_SPARKS; i++) {
                Particle *p = &sp->sparks[i];
                p->life -= dt;
                if (p->life <= 0)
                    SpawnSpinnerSpark(p, sp->pos);
                p->vel.y += 250*dt;
                p->pos.x += p->vel.x*dt;
                p->pos.y += p->vel.y*dt;
            }
        }

        // ---------------------------------------------------
        //                        DRAW
        // ---------------------------------------------------
        BeginDrawing();
        ClearBackground((Color){10, 10, 40, 255});  // dark blue sky

        float time = GetTime();

        // --------- STARS ---------
        for (int i = 0; i < STARS; i++)
            DrawPixel(stars[i].x, stars[i].y, WHITE);

        // --------- MOON ---------
        DrawCircle(W - 150, 120, 55, YELLOW);
        DrawCircle(W - 130, 120, 55, (Color){10,10,40,255});

        // --------- GRASS LAND ---------
        DrawRectangle(0, H - 100, W, 100, (Color){20,140,20,255});
        for (int x = 0; x < W; x += 15)
            DrawLine(x, H - 100, x+5, H - 110 - rand()%10, (Color){30,180,30,255});

        // --------- HAPPY DIWALI POP TEXT ----------
        float scale = 1.0f + 0.08f * sinf(time * 4);  // popping effect
        const char *msg = "HAPPY DIWALI";
        int baseSize = 64;
        int size = baseSize * scale;

        int textWidth = MeasureText(msg, size);
        DrawText(msg, (W - textWidth)/2, H/2 - 200, size, (Color){245, 245, 220, 255}); // Beige color

        DrawText("Press SPACE to ignite all fireworks!", 20, 20, 24, WHITE);

        // --------- DRAW FLOWERPOTS ---------
        for (int f = 0; f < FLOWERPOTS; f++) {
            FlowerPot *fp = &pots[f];

            DrawTriangle((Vector2){fp->pos.x-40, fp->pos.y},
                         (Vector2){fp->pos.x+40, fp->pos.y},
                         (Vector2){fp->pos.x, fp->pos.y-40},
                         BROWN);

            if (!fp->exploded) {
                for (int i = 0; i < MAX_FOUNTAIN; i++) {
                    Particle *p = &fp->fountain[i];
                    if (!p->active) continue;
                    Color c = p->color; c.a = p->life*255;
                    DrawCircleV(p->pos, 3, c);
                }
            } 
            else {
                for (int i = 0; i < MAX_PARTICLES; i++) {
                    Particle *p = &fp->explosion[i];
                    if (!p->active) continue;
                    Color c = p->color; c.a = p->life*255;
                    DrawCircleV(p->pos, 4, c);
                }
            }
        }

        // --------- DRAW ROCKETS ---------
        for (int r = 0; r < ROCKETS; r++) {
            Rocket *rk = &rockets[r];

            for (int i = 0; i < MAX_TRAIL; i++) {
                Particle *p = &rk->trail[i];
                if (p->active) {
                    Color c = p->color; c.a = p->life*255;
                    DrawCircleV(p->pos, 2, c);
                }
            }

            if (!rk->exploded)
                DrawCircleV(rk->pos, 8, RED);

            for (int i = 0; i < MAX_ROCKET_PARTICLES; i++) {
                Particle *p = &rk->boom[i];
                if (p->active) {
                    Color c = p->color; c.a = p->life*255;
                    DrawCircleV(p->pos, 4, c);
                }
            }
        }

        // --------- DRAW SPINNERS ---------
        for (int s = 0; s < SPINNERS; s++) {
            Spinner *sp = &spinners[s];
            if (!sp->active) continue;

            DrawCircleV(sp->pos, 20, SpinnerWheelColor(time));

            for (int i = 0; i < SPINNER_SPARKS; i++) {
                Particle *p = &sp->sparks[i];
                Color c = p->color; c.a = p->life*255;
                DrawCircleV(p->pos, 4, c);
            }
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}