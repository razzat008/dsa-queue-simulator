#include <SDL2/SDL.h>
#include <SDL2/SDL_rect.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_VEHICLES 100

typedef struct {
  SDL_Rect rect;
  int vehicle_id;
  char road_id;
  int lane;
  int speed;
  char targetRoad;
  int targetLane;
} Vehicle;

typedef struct {
  Vehicle *vehicles[MAX_VEHICLES];
  int front;
  int rear;
  int size;
} VehicleQueue;

void initQueue(VehicleQueue *q) {
  q->front = 0;
  q->rear = -1;
  q->size = 0;
}

int isQueueFull(VehicleQueue *q) { return q->size >= MAX_VEHICLES; }

int isQueueEmpty(VehicleQueue *q) { return q->size == 0; }

void enqueue(VehicleQueue *q, Vehicle *v) {
  if (isQueueFull(q)) {
    printf("Queue is full! Cannot enqueue vehicle %d\n", v->vehicle_id);
    free(v);
    return;
  }
  q->rear = (q->rear + 1) % MAX_VEHICLES;
  q->vehicles[q->rear] = v;
  q->size++;
  printf("Enqueued vehicle %d on Road %c Lane %d\n", v->vehicle_id, v->road_id,
         v->lane);
}

Vehicle *dequeue(VehicleQueue *q) {
  if (isQueueEmpty(q)) {
    printf("Queue is empty!\n");
    return NULL;
  }
  Vehicle *v = q->vehicles[q->front];
  q->front = (q->front + 1) % MAX_VEHICLES;
  q->size--;
  return v;
}

int InitializeSDL(void) {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
    return -1;
  }
  return 0;
}

SDL_Window *CreateWindow(const char *title, int width, int height) {
  SDL_Window *window = SDL_CreateWindow(
      title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
      SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
  if (!window) {
    SDL_Log("Window could not be created! SDL_Error: %s\n", SDL_GetError());
  }
  return window;
}

SDL_Renderer *CreateRenderer(SDL_Window *window) {
  SDL_Renderer *renderer =
      SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    SDL_Log("Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
  }
  return renderer;
}

void drawVehicle(SDL_Renderer *renderer, Vehicle *vehicle) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    printf("Drawing vehicle %d at (%d, %d) with size (%d, %d)\n", 
           vehicle->vehicle_id, vehicle->rect.x, vehicle->rect.y, vehicle->rect.w, vehicle->rect.h);
    SDL_RenderFillRect(renderer, &vehicle->rect);
}

void DrawDashedLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2, int dashLength) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
    
    float xIncrement = (float)dx / steps;
    float yIncrement = (float)dy / steps;

    float x = x1;
    float y = y1;

    if (dashLength == 0) {
        // If dashLength is 0, draw a continuous line
        for (int i = 0; i <= steps; i++) {
            SDL_RenderDrawPoint(renderer, (int)x, (int)y);
            x += xIncrement;
            y += yIncrement;
        }
    } else {
        // Draw a dashed line
        for (int i = 0; i <= steps; i++) {
            if ((i / dashLength) % 2 < 1) {
                SDL_RenderDrawPoint(renderer, (int)x, (int)y);
            }
            x += xIncrement;
            y += yIncrement;
        }
    }
}

void DrawLaneMarking(SDL_Renderer *renderer) {
  SDL_Color laneMarking = {247, 233, 23, 255};
  SDL_Color laneMarkingRed = {247, 0, 0, 255};

  SDL_SetRenderDrawColor(renderer, laneMarking.r, laneMarking.g, laneMarking.b,
                         laneMarking.a);
  // Horizontal road markings
  DrawDashedLine(renderer, 0, 250, 150, 250, 10);
  DrawDashedLine(renderer, 0, 350, 150, 350, 10);
  DrawDashedLine(renderer, 450, 250, 600, 250, 10);
  DrawDashedLine(renderer, 450, 350, 600, 350, 10);

  SDL_SetRenderDrawColor(renderer, laneMarkingRed.r, laneMarkingRed.g,
                         laneMarkingRed.b, laneMarkingRed.a);
  // Split middle lane markings - horizontal
  DrawDashedLine(renderer, 600, 300, 450, 300, 0); // Middle divider
  DrawDashedLine(renderer, 0, 300, 150, 300, 0);   // Middle divider

  SDL_SetRenderDrawColor(renderer, laneMarking.r, laneMarking.g, laneMarking.b,
                         laneMarking.a);
  // Vertical road markings
  DrawDashedLine(renderer, 250, 0, 250, 150, 10);
  DrawDashedLine(renderer, 350, 0, 350, 150, 10);
  DrawDashedLine(renderer, 250, 450, 250, 600, 10);
  DrawDashedLine(renderer, 350, 450, 350, 600, 10);

  SDL_SetRenderDrawColor(renderer, laneMarkingRed.r, laneMarkingRed.g,
                         laneMarkingRed.b, laneMarkingRed.a);
  // Split middle lane markings - vertical
  DrawDashedLine(renderer, 300, 0, 300, 150, 0);   // Middle divider
  DrawDashedLine(renderer, 300, 600, 300, 450, 0); // Middle divider
}

void DrawTrafficLight(SDL_Renderer *renderer, int XPos, int YPos, int isGreen,
                      char *orientation) {

  const int width = 30;
  const int height = 90;

  if (isGreen) {
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  } else {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  }

  SDL_Rect trafficLightRect;

  if (strcmp(orientation, "vertical") == 0) {
    trafficLightRect =
        (SDL_Rect){XPos, YPos, width, height}; // Horizontal orientation
  } else if (strcmp(orientation, "horizontal") == 0) {
    trafficLightRect =
        (SDL_Rect){XPos, YPos, height, width}; // Vertical orientation
  } else {
    printf("Invalid orientation: %s\n", orientation);
    return;
  }

  SDL_RenderFillRect(renderer, &trafficLightRect);
}
