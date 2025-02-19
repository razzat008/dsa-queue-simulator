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
