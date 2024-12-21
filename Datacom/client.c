// Revize Edilmis Client Kodu
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024

void temizle_buffer(char *buffer) {
    memset(buffer, 0, BUFFER_SIZE);
}

int main() {
    WSADATA wsa;
    SOCKET client_socket;
    struct sockaddr_in server;
    char buffer[BUFFER_SIZE];

    // Winsock baslat
    printf("Winsock baslatiliyor...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock baslatilamadi. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    // Soket olustur
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Soket olusturulamadi. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);

    // Sunucuya baglan
    if (connect(client_socket, (struct sockaddr *)&server, sizeof(server)) < 0) {
        printf("Baglanti basarisiz. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    printf("Sunucuya baglanildi. Komutlar girin (UyeEkle, Uyeler, UyeSil):\n");

    while (1) {
        printf("> ");
        temizle_buffer(buffer);
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = 0; // Yeni satir karakterini kaldir

        // Komutu sunucuya gonder
        send(client_socket, buffer, strlen(buffer), 0);

        // Sunucudan cevap al
        temizle_buffer(buffer);
        recv(client_socket, buffer, BUFFER_SIZE, 0);
        printf("%s", buffer);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}

