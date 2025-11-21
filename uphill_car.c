#include "raylib.h"

#define gravity 20

typedef struct car{
    Vector2 position;
    Vector2 velocity;
    float width; 
    float height; 
}car;

typedef struct wheel{
    Vector2 position;
    Vector2 velocity;
    float radius; 
    float padding;
}wheel; 

int main(void) {
    InitWindow(1000,800, "UPHILL CAR");
    SetTargetFPS(60);

    car car = {
        .position = (Vector2) {300,300} ,
        .width = 250 , 
        .height = 100
    };

    wheel back_wheel = {
        .radius = 30,
        .padding = 10 
    };

    wheel front_wheel= {
        .radius = 30,
        .padding = 10 
    };
    back_wheel.position = (Vector2){
        .x = car.position.x + back_wheel.radius ,
        .y = car.position.y +car.height + back_wheel.radius + back_wheel.padding 
    };


    front_wheel.position = (Vector2){
        .x = car.position.x - back_wheel.radius + car.width ,
        .y = car.position.y + car.height + back_wheel.radius + back_wheel.padding 
    };


    while (!WindowShouldClose()) {
        BeginDrawing();

        ClearBackground(WHITE);

        DrawRectangle(car.position.x, car.position.y, car.width, car.height, BLACK);
        DrawCircle(back_wheel.position.x  , back_wheel.position.y , back_wheel.radius , BLACK);
        DrawCircle(front_wheel.position.x , front_wheel.position.y , front_wheel.radius , BLACK);

        DrawText("test car", 200, 200, 20, BLACK);

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
