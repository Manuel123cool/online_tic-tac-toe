#include "chap03.h"
#include <stdio.h>
#define FALSE 0
#define TRUE 1
#define FIELD_SIZE 9
#define BOOL char
#include <time.h>
#include <stdlib.h>

void printGameField(int field[])
{
    int i;
    for (i = 0; i < FIELD_SIZE; ++i)
    {
        if (field[i] == 1)
            printf("X");
        else if (field[i] == 2)
            printf("O");
        else 
            printf(" ");

        if (i == 2 || i == 5 || i == 8)
            printf("\n");
    }
}

BOOL checkIfWon(BOOL xIs, int field[])
{
    int posLines[8][3] = 
    {
        {0,3,6},
        {1,4,7},
        {2,5,8},
        {0,1,2},
        {3,4,5},
        {6,7,8},
        {0,4,8},
        {2,4,6}
    };
    
    int numberForChecking = 0; 
    if (xIs)
        numberForChecking = 1;
    else
        numberForChecking = 2;

    int i;
    for (i = 0; i < 8; ++i)
    {
        int count = 0;

        int j;
        for (j = 0; j < 3; ++j)
        {
            if (field[posLines[i][j]] == numberForChecking)
            {
                count++;
            }
        }
        if (count == 3)
        {
            return TRUE;            
        }
    }
    return FALSE;
}

void setupClient(SOCKET *socket_pointer)
{
    printf("Enter host ip address: \n");    
    char ipAddress[100];
    scanf("%s", ipAddress);

    /*Configuring remote address*/
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(ipAddress, "8085", &hints, &peer_address)) {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return;
    }

    /*creating socket*/
    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
            peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return;
    }

    /* Connecting... */
    if (connect(socket_peer,
                peer_address->ai_addr, peer_address->ai_addrlen)) {
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return;
    }
    freeaddrinfo(peer_address);
    
    *socket_pointer = socket_peer; 
}

BOOL getWhoStarts(SOCKET socket)
{
    while (TRUE)
    {
        fd_set reads; 
        FD_ZERO(&reads);
        FD_SET(socket, &reads);

        if (select(socket + 1, &reads, 0, 0, 0)) {
            fprintf(stderr, "select() failed. (%d) \n", GETSOCKETERRNO());
            exit(0);
        }
        if (FD_ISSET(socket, &reads))
        {
            char read[1];
            int bytes_received = recv(socket, read, 1, 0);

            printf("Rec data %d\n", read[0]);
            if  (bytes_received < 1)
            {
                printf("Connection closed peer.\n");
                break;
            }
            if (read[0] == 0 || read[0] == 1)
            {
                printf("Rec data %d\n", read[0]);
                return read[0];
            }
            else
            {
                fprintf(stderr, "received wrong data. %d", GETSOCKETERRNO());
                exit(0);
            }
        }
    }
}

void setupHost(SOCKET *socketClient, SOCKET *socketHost)
{
    /*Configuring local address...\n" */
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo *bind_address;
    getaddrinfo(0, "8085", &hints, &bind_address);


    /* Creating socket... */
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
            bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen)) {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        exit(0);
    }


    /* "Binding socket to local address...*/
    if (bind(socket_listen,
                bind_address->ai_addr, bind_address->ai_addrlen)) {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        exit(0);
    }
    freeaddrinfo(bind_address);


    /* "Listening... */
    if (listen(socket_listen, 10) < 0) {
        fprintf(stderr, "listen() failed. (%d)\n", GETSOCKETERRNO());
        exit(0);
    }

    /* "Waiting for connection...*/
    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    SOCKET socket_client = accept(socket_listen,
            (struct sockaddr*) &client_address, &client_len);
    if (!ISVALIDSOCKET(socket_client)) {
        fprintf(stderr, "accept() failed. (%d)\n", GETSOCKETERRNO());
        exit(0);
    }

    *socketClient = socket_client;
    *socketHost = socket_listen;
}

void sendWhoStarts(SOCKET socket, SOCKET clientSocket,  BOOL hostIs)
{
    char response[1];
    response[0] = hostIs;
    int bytes_sent = send(clientSocket, response, strlen(response), 0);
    if (bytes_sent != 1)
        printf("Error sending data %d", bytes_sent);
}

void doGame(BOOL wantToHost)
{
    SOCKET socket;
    SOCKET clientSocket;
    if (!wantToHost)
        setupClient(&socket);
    else 
        setupHost(&socket, &clientSocket);

    int field[FIELD_SIZE];
    int i;
    for (i = 0; i < FIELD_SIZE; ++i)
        field[i] = 0;  

    srand(time(NULL));   
    int r = rand() % 2;
    BOOL xIs = r;

    BOOL hostStarts;
    if (!wantToHost)
    {
        hostStarts = getWhoStarts(socket);
    }
    else 
    {
        hostStarts = xIs;
        sendWhoStarts(socket, clientSocket, xIs);
    }

    printf("%d \n", hostStarts);

    while (TRUE)
    {
        printf("Enter number 1 to 9 for playing\n"); 
        int enterNum = -1;
        scanf("%d", &enterNum);

        if (field[enterNum - 1] != 0)
        {
            printf("Number is occupied\n");
            continue;
        }

        if (xIs)
        {
            field[enterNum - 1] = 1;
            xIs = FALSE;
        }
        else
        {
            field[enterNum - 1] = 2;
            xIs = TRUE;
        }

        BOOL xIsWon = checkIfWon(TRUE, field);
        BOOL oIsWon = checkIfWon(FALSE, field);

        if (xIsWon)
        {
            printf("X won\n");
            return;
        } 
        if (oIsWon)
        {
            printf("O won\n");
            return;
        }

        printGameField(field);
    }        
 
    if (wantToHost)
        CLOSESOCKET(clientSocket);

    CLOSESOCKET(socket);

}

int main()
{
    printf("Do you want to host? (y/n)\n");
    char wantToHost;
    scanf("%c", &wantToHost);
    if (wantToHost == 'y')
        wantToHost = TRUE;
    else
        wantToHost = FALSE;

    doGame(wantToHost); 


    return 0;
}    

