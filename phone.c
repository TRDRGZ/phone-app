#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define BUFFER_SIZE 2048  // 変更: バッファサイズを適切に設定

void gate(signed short* buffer, double* amplitude) {
    double threshold = 0.2; // 0.0 ~ 1.0 入力波形の変位がこれを超えるとゲートが開く
    double release = 0.95;  // 0.0 ~ 1.0 ゲートの閉じるスピードを調整 amplitude *= release^t
    double attack = 0.1;   // 0.0 ~ 1.0 ゲートの開くスピードを調整 amplitude += attack/t
    int i;

    // バッファ内のすべてのサンプルとthresholdを比較
    for (i = 0; i < BUFFER_SIZE; i++) {
        double level = (double)abs(buffer[i]);

        // 一つでもサンプルがthresholdを超えていた場合
        if (level > threshold * 32768) {
            *amplitude += attack; // amplitude増加
            if (*amplitude > 1.0) *amplitude = 1.0;  // 上限を1.0にする
            break;
        }
    }
    // 一つもthresholdを超えないままループを終えた場合
    if (i >= BUFFER_SIZE) {
        *amplitude *= release; //amplitude減少
        if (*amplitude < 0.0) *amplitude = 0.0;  // 下限を0.0にする
    }
}

void amp(signed short* buffer, double* amplitude) {
    int i;
    for (i = 0; i < BUFFER_SIZE; i++) {
        int sample = buffer[i];
        sample = (int)(sample * *amplitude);
        if (sample > 32767) sample = 32767;  // 16ビットPCMの上限
        if (sample < -32768) sample = -32768;  // 16ビットPCMの下限
        buffer[i] = (signed short)sample;
    }
}

void *send_audio(void *arg) {
    int sock = *((int *)arg);
    signed short buffer[BUFFER_SIZE];
    double amplitude = 1.0;
    while (1) {
        ssize_t num_read = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (num_read <= 0) break;
        gate(buffer, &amplitude); //bufferを送ってゲートの動きを決定
        amp(buffer, &amplitude); //ゲートの動きに合わせてバッファの中身の音量変更
        send(sock, buffer, num_read, 0);
    }
    return NULL;
}

void *receive_audio(void *arg) {
    int sock = *((int *)arg);
    signed short buffer[BUFFER_SIZE];
    while (1) {
        ssize_t num_received = recv(sock, buffer, sizeof(buffer), 0);
        if (num_received <= 0) break;

        write(STDOUT_FILENO, buffer, num_received);
    }
    return NULL;
}

int client_setup(char *server_ip, int port) {
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) { perror("socket"); exit(1); }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(s, (struct sockaddr *)&addr, sizeof(addr)) == -1) { perror("connect"); exit(1); }
    return s;
}

int server_setup(int port) {
    int ss = socket(AF_INET, SOCK_STREAM, 0);
    if (ss == -1) { perror("socket"); exit(1); }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(ss, (struct sockaddr *)&addr, sizeof(addr)) == -1) { perror("bind"); exit(1); }
    if (listen(ss, 10) == -1) { perror("listen"); exit(1); }

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(struct sockaddr_in);
    int s = accept(ss, (struct sockaddr *)&client_addr, &len);
    close(ss);

    return s;
}

int main(int argc, char *argv[]) {
    if ((argc < 2) || (argc > 3)) {
        fprintf(stderr, "Usage Error\n");
        exit(EXIT_FAILURE);
    }

    int sock;
    if (argc == 3) {
        char* server_ip = argv[1];
        int port = atoi(argv[2]);
        sock = client_setup(server_ip, port);
    }

    if (argc == 2) {
        int port = atoi(argv[1]);
        sock = server_setup(port);
    }

    pthread_t send_thread, receive_thread;

    if (pthread_create(&send_thread, NULL, send_audio, &sock) != 0) {
        perror("pthread_create send_audio");
        close(sock);
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&receive_thread, NULL, receive_audio, &sock) != 0) {
        perror("pthread_create receive_audio");
        close(sock);
        exit(EXIT_FAILURE);
    }

    pthread_join(send_thread, NULL);
    pthread_join(receive_thread, NULL);

    close(sock);

    return 0;
}
