#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

#define ROADS 4
#define LANES 3

typedef struct
{
    int vehicle_id;
    char road_id;
    int lane;
} Vehicle;

void send_data(int socket_fd, Vehicle *data)
{
    if (send(socket_fd, data, sizeof(*data), 0) < 0)
    {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
    printf("Data sent to client: Vehicle ID: %d on Road %c Lane %cL%d\n",
           data->vehicle_id, data->road_id, data->road_id, data->lane);
}


char getRandomRoad()
{
    char roads[] = {'A', 'B', 'C', 'D'};
    return roads[rand() % ROADS];
}

Vehicle generate_vehicle()
{
    Vehicle v;

    v.vehicle_id = rand() % 100;
    v.road_id = getRandomRoad();
    v.lane = rand() % LANES + 1;

    return v;
}

int main()
{

}

