#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_UYELER 100

typedef struct {
    char isim[50];
    char soyisim[50];
    char uyelik_turu[20];
    int kalan_gun;
} Uye;

Uye uyeler[MAX_UYELER];
int uye_sayisi = 0;

void temizle_buffer(char *buffer) {
    memset(buffer, 0, BUFFER_SIZE);
}

void uye_ekle(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    char isim[50], soyisim[50], uyelik_turu[20];
    int uyelik_tipi, ay;

    // Ýsim al
    temizle_buffer(buffer);
    send(client_socket, "Uye ismi: ", strlen("Uye ismi: "), 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    strcpy(isim, buffer);

    // Soyisim al
    temizle_buffer(buffer);
    send(client_socket, "Uye soyismi: ", strlen("Uye soyismi: "), 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    strcpy(soyisim, buffer);

    // Üyelik tipi seçimi
    temizle_buffer(buffer);
    send(client_socket, 
        "1) Gold Uyelik (Aylik = 600 TL)\n"
        "2) Silver Uyelik (Aylik = 500 TL)\n"
        "3) Bronz Uyelik (Aylik = 400 TL)\n"
        "Seciminizi yapin (1-3): ", 
        strlen(
            "1) Gold Uyelik (Aylik = 600 TL)\n"
            "2) Silver Uyelik (Aylik = 500 TL)\n"
            "3) Bronz Uyelik (Aylik = 400 TL)\n"
            "Seciminizi yapin (1-3): "), 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    uyelik_tipi = atoi(buffer);

    // Üyelik tipi belirleme
    if (uyelik_tipi == 1) strcpy(uyelik_turu, "Gold");
    else if (uyelik_tipi == 2) strcpy(uyelik_turu, "Silver");
    else if (uyelik_tipi == 3) strcpy(uyelik_turu, "Bronz");
    else strcpy(uyelik_turu, "Bilinmiyor");

    // Kaç ay alýnacak
    temizle_buffer(buffer);
    send(client_socket, "Kac aylik uyelik istiyorsunuz? ", strlen("Kac aylik uyelik istiyorsunuz? "), 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    ay = atoi(buffer);

    // Ücret hesaplama
    int aylik_ucret = uyelik_tipi == 1 ? 600 : uyelik_tipi == 2 ? 500 : 400;
    int toplam_ucret = aylik_ucret * ay;
    int toplam_gun = ay * 30;

    // Yeni üye ekleme
    strcpy(uyeler[uye_sayisi].isim, isim);
    strcpy(uyeler[uye_sayisi].soyisim, soyisim);
    strcpy(uyeler[uye_sayisi].uyelik_turu, uyelik_turu);
    uyeler[uye_sayisi].kalan_gun = toplam_gun;
    uye_sayisi++;

    // Baþarý mesajý gönder
    temizle_buffer(buffer);
    sprintf(buffer, "Tebrikler, %s %s. %s uyeliginiz icin toplam ucret: %d TL (%d gun).\n", 
            isim, soyisim, uyelik_turu, toplam_ucret, toplam_gun);
    send(client_socket, buffer, strlen(buffer), 0);
}

void uyeleri_listele(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    temizle_buffer(buffer);

    if (uye_sayisi == 0) {
        send(client_socket, "Uyeler: Kayitli uye bulunmamaktadir.\n", strlen("Uyeler: Kayitli uye bulunmamaktadir.\n"), 0);
        return;
    }
    int i;
    strcat(buffer, "Uyeler:\n");
    for (i = 0; i < uye_sayisi; i++) {
        char satir[BUFFER_SIZE];
        sprintf(satir, "Isim: %s, Soyisim: %s, Uyelik: %s, Kalan Gun: %d\n", 
                uyeler[i].isim, uyeler[i].soyisim, uyeler[i].uyelik_turu, uyeler[i].kalan_gun);
        strcat(buffer, satir);
    }
    send(client_socket, buffer, strlen(buffer), 0);
}

void uye_sil(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    char isim[50];
    int bulundu = 0;

    // Silinecek üyenin ismini al
    temizle_buffer(buffer);
    send(client_socket, "Silmek istediginiz uye adini girin: ", strlen("Silmek istediginiz uye adini girin: "), 0);
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    strcpy(isim, buffer);
	int i,j;
    // Üye arama ve silme
    for (i = 0; i < uye_sayisi; i++) {
        if (strcmp(uyeler[i].isim, isim) == 0) {
            bulundu = 1;
            for (j = i; j < uye_sayisi - 1; j++) {
                uyeler[j] = uyeler[j + 1];
            }
            uye_sayisi--;
            break;
        }
    }

    // Sonuç mesajý
    temizle_buffer(buffer);
    if (bulundu) {
        sprintf(buffer, "%s uyesi silinmistir.\n", isim);
    } else {
        sprintf(buffer, "%s adinda bir uye bulunamadi.\n", isim);
    }
    send(client_socket, buffer, strlen(buffer), 0);
}

int main() {
    WSADATA wsa;
    SOCKET server_socket, client_socket;
    struct sockaddr_in server, client;
    int c;
    char buffer[BUFFER_SIZE];

    // Winsock baþlat
    printf("Winsock baslatiliyor...\n");
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("Winsock baslatilamadi. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    // Sunucu soketi oluþtur
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET) {
        printf("Soket olusturulamadi. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR) {
        printf("Baglama basarisiz. Hata kodu: %d\n", WSAGetLastError());
        return 1;
    }

    listen(server_socket, 3);
    printf("Baglanti bekleniyor...\n");

    c = sizeof(struct sockaddr_in);
    while ((client_socket = accept(server_socket, (struct sockaddr *)&client, &c)) != INVALID_SOCKET) {
        printf("Baglanti kabul edildi.\n");

        while (1) {
            temizle_buffer(buffer);
            recv(client_socket, buffer, BUFFER_SIZE, 0);

            if (strcmp(buffer, "UyeEkle") == 0) {
                uye_ekle(client_socket);
            } else if (strcmp(buffer, "Uyeler") == 0) {
                uyeleri_listele(client_socket);
            } else if (strcmp(buffer, "UyeSil") == 0) {
                uye_sil(client_socket);
            } else {
                send(client_socket, "Gecersiz komut.\n", strlen("Gecersiz komut.\n"), 0);
            }
        }
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}

