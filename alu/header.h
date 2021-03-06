#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <algorithm>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <vector>
#include <thread>
#include <map>
#include <mutex>
#define MENSAJE_MAXIMO 50
#define MAX_CLIENTS 10
#define BUF_SIZE 4096
#define SOCK_PATH       "mi_socket"

using namespace std;
int leer_de_socket(int s);
void escucharSocket(int s);
vector<string> split(const string& str, const string& delim);

struct Cliente {
    int nSocket;
    string nombre;

    Cliente(int nSocketAGuardar, string nombreCliente){
        nombre = nombreCliente;
        nSocket = nSocketAGuardar;
    }

    void enviar(const char* str){
        send(nSocket, str, strlen(str), 0);
    }
};

