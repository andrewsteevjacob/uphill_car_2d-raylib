#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#define WINDOW_WIDTH 1500
#define WINDOW_HEIGHT 1000

#define GRAVITY 4
#define FRICTION 0.2
#define ROTATION_SPEED 10
#define ROTATE_BACK_SPEED 3
#define CAR_SPEED 9
#define HILL_SPEED 0.2
#define TRANSPARENT_BLACK (Color){0,0,0,100}

bool IsPointBelowLine(Vector2 a, Vector2 b, Vector2 point, Vector2 *collisionPoint) {
    // Handle non-vertical segments.
    if (fabs(b.x - a.x) > 1e-6) {
        // Compute the parameter t from a to b using the x coordinate.
        float t = (point.x - a.x) / (b.x - a.x);
        // If t is not between 0 and 1, the vertical line does not intersect the segment.
        if (t < 0.0f || t > 1.0f)
            return false;
        // Compute the y-coordinate of the intersection on the segment.
        collisionPoint->x = point.x;
        collisionPoint->y = a.y + t * (b.y - a.y);
        // In raylib, a larger y means further down the screen.
        return (point.y > collisionPoint->y);
    } else {
        // For a vertical segment, the point must align with the segment's x value (within a tolerance).
        if (fabs(point.x - a.x) > 1e-6)
            return false;
        collisionPoint->x = a.x;
        // Use the lower end (largest y value) of the segment as the collision point.
        collisionPoint->y = fmax(a.y, b.y);
        return (point.y > collisionPoint->y);
    }
}

typedef struct Wheel {
    Vector2 position;
    Vector2 velocity;
    float radius;
    float padding;
    float stiffness;
    float damping;
    float offset;
    bool on_ground;
} Wheel;

typedef struct Car {
    Vector2 position;
    Vector2 velocity;
    float width;
    float height;
    float angle;
    Wheel back_wheel;
    Wheel front_wheel;
} Car;

void car_control( Car* car, float dt ) {
    if( ! car->back_wheel.on_ground && ! car->front_wheel.on_ground ) {
        if( IsKeyDown( KEY_LEFT ) ) {
            car->angle += ROTATION_SPEED * dt;
        } else if( IsKeyDown( KEY_RIGHT ) ) {
            car->angle -= ROTATION_SPEED * dt;
        }
    }

    if( IsKeyDown( KEY_RIGHT ) ) {
        if( car->back_wheel.on_ground ) {
            car->velocity.x += CAR_SPEED * dt;
        }

        if( car->front_wheel.on_ground ) {
            car->velocity.x += CAR_SPEED * dt;
        }
    } else if( IsKeyDown( KEY_LEFT ) ) {
        if( car->back_wheel.on_ground ) {
            car->velocity.x -= CAR_SPEED * dt;
        }

        if( car->front_wheel.on_ground ) {
            car->velocity.x -= CAR_SPEED * dt;
        }
    }
}

void car_rotate( Car* car, float dt ) {
    //if( car->back_wheel.on_ground && car->front_wheel.on_ground ) {
        float angle = Vector2LineAngle( car->back_wheel.position, car->front_wheel.position ) * RAD2DEG;
        float diff = angle - car->angle;
        car->angle += diff * ROTATE_BACK_SPEED * dt;
        //printf( "Angle difference: %f (%f / %f)\n", diff, angle, car->angle );
    //}
}

void car_move( Car* car, Vector2 terrain[], int terrain_length, float dt ) {
    //gravity add part 2 
    car->position.x += car->velocity.x;
    car->position.y += car->velocity.y;

    if( car->back_wheel.on_ground ) {
        int terrain_index = floor( car->back_wheel.position.x / terrain_length );
        Vector2 point1 = terrain[terrain_index];
        Vector2 point2 = terrain[terrain_index + 1]; // TODO: Fix overflow

        DrawCircleV( point1, 10, RED );
        DrawCircleV( point2, 10, ORANGE );

        float angle = Vector2LineAngle( point1, point2 ) * RAD2DEG;
        printf("Terrain angle (back wheel): %f (%d)\n", angle, terrain_index);

        car->velocity.x += angle * HILL_SPEED * dt;

        float friction = car->velocity.x * FRICTION;
        car->velocity.x -= friction * dt;
    }

    if( car->front_wheel.on_ground ) {
        int terrain_index = floor( car->front_wheel.position.x / terrain_length );
        Vector2 point1 = terrain[terrain_index];
        Vector2 point2 = terrain[terrain_index + 1]; // TODO: Fix overflow

        DrawCircleV( point1, 10, RED );
        DrawCircleV( point2, 10, ORANGE );

        float angle = Vector2LineAngle( point1, point2 ) * RAD2DEG;
        printf("Terrain angle (front wheel): %f (%d)\n", angle, terrain_index);

        car->velocity.x += angle * HILL_SPEED * dt;

        float friction = car->velocity.x * FRICTION;
        car->velocity.x -= friction * dt;
    }

    float max_y = GetScreenHeight() - car->height / 2;

    //gravity add part 2
    if( car->position.y >= max_y ) {
        car->velocity.y = 0;
        car->position.y = max_y;
    } else {
        car->velocity.y += GRAVITY * dt;
    }
}

void car_draw( Car* car ) {
    DrawRectanglePro( (Rectangle){
        .width = car->width,
        .height = car->height,
        .x = car->position.x,
        .y = car->position.y,
    }, (Vector2){car->width / 2, car->height / 2}, car->angle, TRANSPARENT_BLACK );

    DrawCircle( car->back_wheel.position.x, car->back_wheel.position.y, car->back_wheel.radius, TRANSPARENT_BLACK );
    DrawCircle( car->front_wheel.position.x, car->front_wheel.position.y, car->front_wheel.radius, TRANSPARENT_BLACK );
}

void car_apply_suspension( Car* car, Wheel* wheel, float dt ) {
    Vector2 bottom_direction = Vector2Rotate( (Vector2) {0, 1}, car->angle * DEG2RAD );

    Vector2 attachment_point = Vector2Rotate( (Vector2) {-car->width / 2 + wheel->padding + wheel->radius + wheel->offset, 0}, car->angle * DEG2RAD );
    attachment_point = Vector2Add( attachment_point, car->position );
    DrawCircleV( attachment_point, 10, RED );

    float length = Vector2Distance( wheel->position, attachment_point );
    float resting_length = car->height / 2 + wheel->padding + wheel->radius;
    float stretch = length - resting_length;

    wheel->position = Vector2Add( attachment_point, Vector2Scale( bottom_direction, length ) );

    float spring_force = stretch * wheel->stiffness * dt;
    Vector2 relative_velocity = Vector2Subtract( car->velocity, wheel->velocity );
    Vector2 damping_force = Vector2Scale( relative_velocity, wheel->damping * dt );

    Vector2 force = Vector2Subtract( Vector2Scale( bottom_direction, spring_force ), damping_force );

    car->velocity = Vector2Add( car->velocity, force );
    wheel->velocity = Vector2Subtract( wheel->velocity, Vector2Scale( force, 0.7 ) );
}

void wheel_move( Wheel* wheel, Vector2 terrain[], int terrain_count, float dt ) {
    wheel->position.x += wheel->velocity.x;
    wheel->position.y += wheel->velocity.y;

    wheel->on_ground = false;

    for( int i = 1; i < terrain_count; i++ ) {
        Vector2 point1 = terrain[i-1];
        Vector2 point2 = terrain[i];

        Vector2 collision_point = {0};
        Vector2 bottom_of_wheel = {wheel->position.x, wheel->position.y + wheel->radius};
        if( IsPointBelowLine( point1, point2, bottom_of_wheel, &collision_point ) ) {
            wheel->velocity.y = 0;
            wheel->position.y = collision_point.y - wheel->radius + 1;

            wheel->on_ground = true;
        }
    }

    if( ! wheel->on_ground ) {
        wheel->velocity.y += GRAVITY * dt;
    }
}

int main() {
    InitWindow( WINDOW_WIDTH, WINDOW_HEIGHT, "Uphill Racer" );
    SetTargetFPS( 60 );

    Camera2D camera = {
        .offset = (Vector2) {WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2},
        .target = (Vector2) {0, 0},
        .rotation = 0,
        .zoom = 1,
    };

    //defining the car
    Car car = {
        .position = (Vector2) {1200, 300},
        .width = 250,
        .height = 100,
        .angle = 0,
    };

    car.back_wheel = (Wheel) {
        .radius = 25,
        .padding = 10,
        .stiffness = 0.8,
        .damping = 0.6,
    };

    car.back_wheel.position = (Vector2){
        .x = car.position.x - car.width / 2 + car.back_wheel.radius + car.back_wheel.padding,
        .y = car.position.y + car.height / 2 + car.back_wheel.radius + car.back_wheel.padding,
    };

    car.front_wheel = (Wheel) {
        .radius = 25,
        .padding = 10,
        .stiffness = 0.8,
        .damping = 0.6,
    };

    car.front_wheel.offset = car.width - car.back_wheel.radius - car.back_wheel.padding - car.front_wheel.padding - car.front_wheel.radius;

    car.front_wheel.position = (Vector2){
        .x = car.position.x + car.width / 2 - car.front_wheel.radius - car.front_wheel.padding,
        .y = car.position.y + car.height / 2 + car.front_wheel.radius + car.front_wheel.padding,
    };

    int terrain_length = 100;
    int terrain_count = 255;
    Vector2 terrain[terrain_count];

    int pos = GetRandomValue( WINDOW_HEIGHT * 0.7, WINDOW_HEIGHT * 0.95 );
    for( int i = 0; i < terrain_count; i++ ) {
        int movement = GetRandomValue( -50, 50 );

        Vector2 point = {i * terrain_length, pos};
        terrain[i] = point;

        pos = pos + movement;
    }

    while( ! WindowShouldClose() ) {
        BeginDrawing();
        BeginMode2D( camera );

        camera.target = car.position;
        camera.zoom = 1.3 - car.velocity.x / 10;
        if( camera.zoom > 1.3 ) {
            camera.zoom = 1.3;
        }
        if( camera.zoom < 1 ) {
            camera.zoom = 1;
        }

        float dt = GetFrameTime();

        ClearBackground( WHITE );

        for( int i = 1; i < terrain_count; i++ ) {
            Vector2 point1 = terrain[i-1];
            Vector2 point2 = terrain[i];

            DrawLineEx( point1, point2, 5, BLACK );
        }

        car_control( &car, dt );
        car_move( &car, terrain, terrain_length, dt );
        car_rotate( &car, dt );
        wheel_move( &car.back_wheel, terrain, terrain_count, dt );
        wheel_move( &car.front_wheel, terrain, terrain_count, dt );
        car_apply_suspension( &car, &car.back_wheel, dt );
        car_apply_suspension( &car, &car.front_wheel, dt );

        car_draw( &car );

        EndMode2D();
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
