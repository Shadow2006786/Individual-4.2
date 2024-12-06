#include "echo.h"
#include <windows.h>

SOCKET server_socket = -1;

void free_socket()
{
    if (server_socket > 0)
    {
        closesocket(server_socket);
    }
}

void usage(const char* exe_name)
{
    printf("Usage:\n");
    printf("\t%s -p <port>\n", exe_name);
}

int start(int argc, char* argv[])
{
    int port = DEFAULT_PORT;

    if (argc >= 2)
    {
        char arg_line[128] = { 0 };
        combine_arg_line(arg_line, argv, 1, argc);

        int ret = sscanf(arg_line, "-p %d", &port);
        if (ret < 1) {
            usage(argv[0]);
            return -1;
        }
    }

    return init_server(port);
}

int init_server(short port)
{
    server_socket = socket(AF_INET, SOCK_DGRAM, 0);

    if (server_socket <= 0)
    {
        printf("Cannot create socket\n");
        return -1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        printf("Cannot bind socket to port %d\n", port);
        return -2;
    }

    printf("Server is running on port %d\n", port);

    return process_requests();
}

int process_requests()
{
    struct sockaddr_in client_addr;
    int len = sizeof(client_addr);
    char buffer[1024];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        int ret = recvfrom(server_socket, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &len);

        if (ret <= 0)
        {
            printf("Receiving data error\n");
            continue;
        }

        printf("Received program name: %s from %s\n", buffer, inet_ntoa(client_addr.sin_addr));

        char response[1024] = { 0 };

        STARTUPINFO si = { 0 };
        PROCESS_INFORMATION pi = { 0 };
        si.cb = sizeof(si);

        if (CreateProcess(NULL, buffer, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            printf("Program '%s' started successfully\n", buffer);
            strcpy(response, "Program started successfully.");
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            printf("Failed to start program '%s'\n", buffer);
            strcpy(response, "Error: Program not found or failed to start.");
        }

        ret = sendto(server_socket, response, strlen(response), 0, (struct sockaddr*)&client_addr, len);

        if (ret <= 0)
        {
            printf("Sending response error\n");
        }
    }

    return 0;
}
