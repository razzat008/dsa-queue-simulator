#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_VEHICLES 100

typedef struct{
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

int isQueueFull(VehicleQueue *q) {
    return q->size >= MAX_VEHICLES;
}

int isQueueEmpty(VehicleQueue *q) {
    return q->size == 0;
}

void enqueue(VehicleQueue *q, Vehicle *v) {
    if (isQueueFull(q)) {
        printf("Queue is full! Cannot enqueue vehicle %d\n", v->vehicle_id);
        free(v);
        return;
    }
    q->rear = (q->rear + 1) % MAX_VEHICLES;
    q->vehicles[q->rear] = v;
    q->size++;
    printf("Enqueued vehicle %d on Road %c Lane %d\n", v->vehicle_id, v->road_id, v->lane);
}

Vehicle* dequeue(VehicleQueue *q) {
    if (isQueueEmpty(q)) {
        printf("Queue is empty!\n");
        return NULL;
    }
    Vehicle *v = q->vehicles[q->front];
    q->front = (q->front + 1) % MAX_VEHICLES;
    q->size--;
    return v;
}

