#include <SDL2/SDL.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define PORT 8080
#define MAX_VEHICLES 10000
#define SCREEN_WIDTH 600
#define SCREEN_HEIGHT 600

static int northSouthGreen = 0;
static int eastWestGreen = 1;

// Base Vehicle struct for receiving from generator
typedef struct {
    int vehicle_id;
    char road_id;
    int lane;
} BaseVehicle;

// Extended Vehicle struct for simulation
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

void drawVehicle(SDL_Renderer *renderer, Vehicle *vehicle) {
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red vehicle
    SDL_RenderFillRect(renderer, &vehicle->rect);
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
        title, 0, 0, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI);
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

void DrawDashedLine(SDL_Renderer *renderer, int x1, int y1, int x2, int y2,
                    int dashLength) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = (abs(dx) > abs(dy)) ? abs(dx) : abs(dy);
    float xIncrement = (float)dx / steps;
    float yIncrement = (float)dy / steps;
    float x = x1;
    float y = y1;

    if (dashLength == 0) {
        for (int i = 0; i <= steps; i++) {
            SDL_RenderDrawPoint(renderer, (int)x, (int)y);
            x += xIncrement;
            y += yIncrement;
        }
    } else {
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
    DrawDashedLine(renderer, 0, 250, 150, 250, 10);
    DrawDashedLine(renderer, 0, 350, 150, 350, 10);
    DrawDashedLine(renderer, 450, 250, 600, 250, 10);
    DrawDashedLine(renderer, 450, 350, 600, 350, 10);

    SDL_SetRenderDrawColor(renderer, laneMarkingRed.r, laneMarkingRed.g,
                           laneMarkingRed.b, laneMarkingRed.a);
    DrawDashedLine(renderer, 600, 300, 450, 300, 0);
    DrawDashedLine(renderer, 0, 300, 150, 300, 0);

    SDL_SetRenderDrawColor(renderer, laneMarking.r, laneMarking.g, laneMarking.b,
                           laneMarking.a);
    DrawDashedLine(renderer, 250, 0, 250, 150, 10);
    DrawDashedLine(renderer, 350, 0, 350, 150, 10);
    DrawDashedLine(renderer, 250, 450, 250, 600, 10);
    DrawDashedLine(renderer, 350, 450, 350, 600, 10);

    SDL_SetRenderDrawColor(renderer, laneMarkingRed.r, laneMarkingRed.g,
                           laneMarkingRed.b, laneMarkingRed.a);
    DrawDashedLine(renderer, 300, 0, 300, 150, 0);
    DrawDashedLine(renderer, 300, 600, 300, 450, 0);
}

void DrawTrafficLight(SDL_Renderer *renderer, int XPos, int YPos, int isGreen,
                      char *orientation) {
    const int width = 30;
    const int height = 90;
    SDL_Rect trafficLightRect;

    if (isGreen) {
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    } else {
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    }

    if (strcmp(orientation, "vertical") == 0) {
        trafficLightRect = (SDL_Rect){XPos, YPos, width, height};
    } else if (strcmp(orientation, "horizontal") == 0) {
        trafficLightRect = (SDL_Rect){XPos, YPos, height, width};
    } else {
        printf("Invalid orientation: %s\n", orientation);
        return;
    }
    SDL_RenderFillRect(renderer, &trafficLightRect);
}

void TrafficLightState(SDL_Renderer *renderer, int northSouthGreen,
                       int eastWestGreen) {
    DrawTrafficLight(renderer, 175, 255, northSouthGreen, "vertical");
    DrawTrafficLight(renderer, 395, 255, northSouthGreen, "vertical");
    DrawTrafficLight(renderer, 255, 175, eastWestGreen, "horizontal");
    DrawTrafficLight(renderer, 255, 395, eastWestGreen, "horizontal");
}

void DrawBackground(SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 34, 139, 34, 255);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_Rect horizontalRoad = {0, 150, 600, 300};
    SDL_Rect verticalRoad = {150, 0, 300, 600};
    SDL_RenderFillRect(renderer, &horizontalRoad);
    SDL_RenderFillRect(renderer, &verticalRoad);
    DrawLaneMarking(renderer);
}

typedef struct {
    int x_start, x_end;
    int y_start, y_end;
} LanePosition;

LanePosition lanePositions[4][3] = {
    {{150, 250, -30, -30}, {270, 300, -30, -30}, {350, 450, -30, -30}},
    {{350, 450, 630, 630}, {300, 330, 630, 630}, {150, 250, 630, 630}},
    {{630, 630, 150, 250}, {630, 630, 270, 300}, {630, 630, 350, 450}},
    {{-30, -30, 350, 450}, {-30, -30, 300, 330}, {-30, -30, 150, 250}}};

void getLaneCenter(char road, int lane, int *x, int *y) {
    int roadIndex = road - 'A';
    int laneIndex = lane - 1;
    int middleLaneOffset = 0;

    if (lane == 2) {
        if (road == 'A') middleLaneOffset = -15;
        else if (road == 'B') middleLaneOffset = 15;
        else if (road == 'D') middleLaneOffset = 15;
        else if (road == 'C') middleLaneOffset = -15;
    }

    if (road == 'A' || road == 'B') {
        *x = ((lanePositions[roadIndex][laneIndex].x_start +
               lanePositions[roadIndex][laneIndex].x_end) / 2) + middleLaneOffset;
        *y = (road == 'A') ? -30 : SCREEN_HEIGHT + 10;
    } else {
        *x = (road == 'C') ? SCREEN_WIDTH + 10 : -30;
        *y = ((lanePositions[roadIndex][laneIndex].y_start +
               lanePositions[roadIndex][laneIndex].y_end) / 2) + middleLaneOffset;
    }
}

void moveVehicle(Vehicle *vehicle) {
    int targetX, targetY;
    getLaneCenter(vehicle->targetRoad, vehicle->targetLane, &targetX, &targetY);

    if (vehicle->targetLane == 1) {
        if (!((vehicle->road_id == 'D' && vehicle->lane == 3 && vehicle->targetRoad == 'A') ||
              (vehicle->road_id == 'A' && vehicle->lane == 3 && vehicle->targetRoad == 'C') ||
              (vehicle->road_id == 'C' && vehicle->lane == 3 && vehicle->targetRoad == 'B') ||
              (vehicle->road_id == 'B' && vehicle->lane == 3 && vehicle->targetRoad == 'D'))) {
            return;
        }
    }

    if (vehicle->targetLane == 2) {
        if (!((vehicle->road_id == 'A' && vehicle->lane == 2 && vehicle->targetRoad == 'B') ||
              (vehicle->road_id == 'A' && vehicle->lane == 2 && vehicle->targetRoad == 'C') ||
              (vehicle->road_id == 'C' && vehicle->lane == 2 && vehicle->targetRoad == 'A') ||
              (vehicle->road_id == 'C' && vehicle->lane == 2 && vehicle->targetRoad == 'D') ||
              (vehicle->road_id == 'B' && vehicle->lane == 2 && vehicle->targetRoad == 'A') ||
              (vehicle->road_id == 'B' && vehicle->lane == 2 && vehicle->targetRoad == 'D') ||
              (vehicle->road_id == 'D' && vehicle->lane == 2 && vehicle->targetRoad == 'C') ||
              (vehicle->road_id == 'D' && vehicle->lane == 2 && vehicle->targetRoad == 'B'))) {
            return;
        }
    }

    int shouldStop = 0;
    int stopX = vehicle->rect.x;
    int stopY = vehicle->rect.y;

    if (vehicle->lane == 2) {
        if (vehicle->road_id == 'A' && northSouthGreen) {
            stopY = 150 - 20;
            shouldStop = (vehicle->rect.y <= stopY);
        } else if (vehicle->road_id == 'B' && northSouthGreen) {
            stopY = 450;
            shouldStop = (vehicle->rect.y >= stopY);
        } else if (vehicle->road_id == 'D' && eastWestGreen) {
            stopX = 150 - 20;
            shouldStop = (vehicle->rect.x <= stopX);
        } else if (vehicle->road_id == 'C' && eastWestGreen) {
            stopX = 450;
            shouldStop = (vehicle->rect.x >= stopX);
        }
    }

    if (shouldStop) {
        vehicle->rect.x = stopX;
        vehicle->rect.y = stopY;
        return;
    }

    int reachedX = (abs(vehicle->rect.x - targetX) <= vehicle->speed);
    int reachedY = (abs(vehicle->rect.y - targetY) <= vehicle->speed);

    if ((vehicle->road_id == 'A' && vehicle->targetRoad == 'C') ||
        (vehicle->road_id == 'B' && vehicle->targetRoad == 'D')) {
        if (!reachedY) {
            vehicle->rect.y += (vehicle->rect.y < targetY) ? vehicle->speed : -vehicle->speed;
        } else if (!reachedX) {
            vehicle->rect.x += (vehicle->rect.x < targetX) ? vehicle->speed : -vehicle->speed;
        }
    } else {
        if (!reachedX) {
            vehicle->rect.x += (vehicle->rect.x < targetX) ? vehicle->speed : -vehicle->speed;
        } else if (!reachedY) {
            vehicle->rect.y += (vehicle->rect.y < targetY) ? vehicle->speed : -vehicle->speed;
        }
    }

    if (reachedX) vehicle->rect.x = targetX;
    if (reachedY) vehicle->rect.y = targetY;

    if (reachedX && reachedY) {
        vehicle->road_id = vehicle->targetRoad;
        vehicle->lane = vehicle->targetLane;
    }
}

Uint32 lastSwitchTime = 0;

void updateTrafficLights() {
    Uint32 currentTime = SDL_GetTicks();
    if (currentTime - lastSwitchTime > 8555) {
        northSouthGreen = !northSouthGreen;
        eastWestGreen = !eastWestGreen;
        lastSwitchTime = currentTime;
    }
}

int create_socket() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sock_fd;
}

void connect_to_server(int sock_fd, struct sockaddr_in *address) {
    if (connect(sock_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("Connection to server failed");
        exit(EXIT_FAILURE);
    }
}

void receive_vehicle(int socket_fd, VehicleQueue *queue) {
    BaseVehicle base_v;
    ssize_t bytes_received = recv(socket_fd, &base_v, sizeof(BaseVehicle), MSG_DONTWAIT);
    if (bytes_received < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            perror("Receive failed");
        }
        return;
    }
    if (bytes_received == 0) {
        printf("Server disconnected\n");
        exit(0);
    }
    if (bytes_received != sizeof(BaseVehicle)) {
        return; // Incomplete data, wait for more
    }

    Vehicle *new_vehicle = (Vehicle *)malloc(sizeof(Vehicle));
    if (!new_vehicle) {
        perror("Failed to allocate memory for vehicle");
        return;
    }

    new_vehicle->vehicle_id = base_v.vehicle_id;
    new_vehicle->road_id = base_v.road_id;
    new_vehicle->lane = base_v.lane;
    new_vehicle->rect.w = 20;
    new_vehicle->rect.h = 20;
    new_vehicle->speed = 2;
    new_vehicle->targetRoad = 'B'; // Default, can be customized
    new_vehicle->targetLane = 2;   // Default, can be customized
    getLaneCenter(new_vehicle->road_id, new_vehicle->lane, &new_vehicle->rect.x,
                  &new_vehicle->rect.y);

    enqueue(queue, new_vehicle);
    printf("Received Vehicle ID: %d on Road %c Lane %d\n",
           new_vehicle->vehicle_id, new_vehicle->road_id, new_vehicle->lane);
}

int main() {
    if (InitializeSDL() < 0) return 1;

    SDL_Window *window = CreateWindow("Traffic Simulator", SCREEN_WIDTH, SCREEN_HEIGHT);
    if (!window) return 1;

    SDL_Renderer *renderer = CreateRenderer(window);
    if (!renderer) return 1;

    VehicleQueue queue;
    initQueue(&queue);

    int sock_fd = create_socket();
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    connect_to_server(sock_fd, &server_addr);
    printf("Connected to generator on port %d\n", PORT);

    // Set socket to non-blocking
    int flags = fcntl(sock_fd, F_GETFL, 0);
    fcntl(sock_fd, F_SETFL, flags | O_NONBLOCK);

    Vehicle *active_vehicles[MAX_VEHICLES] = {0};
    int num_active_vehicles = 0;
    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_KEYDOWN) {
                running = 0;
            }
        }

        receive_vehicle(sock_fd, &queue);

        while (!isQueueEmpty(&queue) && num_active_vehicles < MAX_VEHICLES) {
            Vehicle *v = dequeue(&queue);
            active_vehicles[num_active_vehicles++] = v;
        }

        updateTrafficLights();

        for (int i = 0; i < num_active_vehicles; i++) {
            if (active_vehicles[i]) {
                moveVehicle(active_vehicles[i]);
                int targetX, targetY;
                getLaneCenter(active_vehicles[i]->targetRoad,
                              active_vehicles[i]->targetLane, &targetX, &targetY);
                if (active_vehicles[i]->rect.x == targetX &&
                    active_vehicles[i]->rect.y == targetY) {
                    printf("Vehicle %d reached target and is removed.\n",
                           active_vehicles[i]->vehicle_id);
                    free(active_vehicles[i]);
                    active_vehicles[i] = NULL;
                }
            }
        }

        int write_idx = 0;
        for (int i = 0; i < num_active_vehicles; i++) {
            if (active_vehicles[i]) {
                active_vehicles[write_idx++] = active_vehicles[i];
            }
        }
        num_active_vehicles = write_idx;

        DrawBackground(renderer);
        TrafficLightState(renderer, northSouthGreen, eastWestGreen);
        for (int i = 0; i < num_active_vehicles; i++) {
            if (active_vehicles[i]) {
                drawVehicle(renderer, active_vehicles[i]);
            }
        }
        SDL_RenderPresent(renderer);
        SDL_Delay(30);
    }

    close(sock_fd);
    for (int i = 0; i < num_active_vehicles; i++) {
        if (active_vehicles[i]) free(active_vehicles[i]);
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
