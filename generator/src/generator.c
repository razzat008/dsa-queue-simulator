#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define PORT 8080
#define ROADS 4
#define LANES 3

typedef struct {
    int vehicle_id;
    char road_id;
    int lane;
} Vehicle;

void send_data(int socket_fd, Vehicle *data) {
    if (send(socket_fd, data, sizeof(Vehicle), 0) < 0) {
        perror("Send failed");
        exit(EXIT_FAILURE);
    }
    printf("Data sent to client: Vehicle ID: %d on Road %c Lane %d\n",
           data->vehicle_id, data->road_id, data->lane);
}

int create_socket() {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    return sock_fd;
}

void set_socket_options(int sock_fd) {
    int opt = 1;
    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
                   sizeof(opt)) < 0) {
        perror("setsockopt failed");
        exit(EXIT_FAILURE);
    }
}

void bind_socket(int sock_fd, struct sockaddr_in *address) {
    if (bind(sock_fd, (struct sockaddr *)address, sizeof(*address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }
}

void listen_for_connections(int sock_fd) {
    if (listen(sock_fd, 3) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }
}

int accept_connection(int sock_fd, struct sockaddr_in *address) {
    int addrlen = sizeof(*address);
    int new_socket =
        accept(sock_fd, (struct sockaddr *)address, (socklen_t *)&addrlen);
    if (new_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }
    return new_socket;
}

char getRandomRoad() {
    char roads[] = {'A', 'B', 'C', 'D'};
    return roads[rand() % ROADS];
}

int getRandomLane() { return rand() % LANES + 1; }

int getRandomVehicleID() { return rand() % 10000; }

Vehicle generate_vehicle() {
    Vehicle v;
    v.vehicle_id = getRandomVehicleID();
    v.road_id = getRandomRoad();
    v.lane = getRandomLane();
    return v;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;

    srand(time(NULL));
    server_fd = create_socket();
    set_socket_options(server_fd);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind_socket(server_fd, &address);
    printf("Server listening on port %d\n", PORT);
    listen_for_connections(server_fd);

    new_socket = accept_connection(server_fd, &address);
    printf("Simulator connected!\n");

    while (1) {
        Vehicle vehicle = generate_vehicle();
        send_data(new_socket, &vehicle);
        // sleep(rand() % 3 + 1); // Random delay between 1-3 seconds
    }

    close(new_socket);
    close(server_fd);
    return 0;
}
