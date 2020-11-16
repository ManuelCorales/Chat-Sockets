#include "header.h"
using namespace std;
#define PORT 5432

// Funcion para establecer la conexion del cliente con el servidor.
// Devuelve el socket descriptor de la conexion
mutex mtxThreadEscuchaInicializado;
int connection_setup(){
    int socket_fd;
    int len;
    struct sockaddr_in  remote;
    char buf[MENSAJE_MAXIMO];
    /* Crear un socket de dominio INET con TCP (SOCK_STREAM).  */
    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Error creando socket");
        exit(1);
    }

    /* Establecer la dirección a la cual conectarse. */
    remote.sin_family = AF_INET;
    remote.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &(remote.sin_addr));

    /* Conectarse. */
    if (connect(socket_fd, (struct sockaddr *)&remote, sizeof(remote)) == -1) {
        perror("conectandose");
        exit(1);
    }

    return socket_fd;
}


// Funcion principal de un cliente.
// El siguiente esquema puede servir como guia. Aprovechen las funciones de string!

int main(void){
    
    /* Conectarse al server (usando INET) */
    int socket_fd = connection_setup();
    

    /* Loop principal que envía mensajes al servidor */
    string msg;
    mtxThreadEscuchaInicializado.lock();
    thread threadEscuchador(escucharSocket, socket_fd);
    mtxThreadEscuchaInicializado.lock();
    while(1) {
        getline(cin, msg);
        if(send(socket_fd, msg.data(), msg.length(), 0) < 0) {
            perror("Se cerró el socket");
            exit(1);
        }
    }
    /* Cerrar el socket. */
    close(socket_fd);
}

void escucharSocket(int s){
    while(1){
        if(leer_de_socket(s) < 0) {
            close(s);
            cout << "Se cerró el servidor" << endl;
            return;
        }
        mtxThreadEscuchaInicializado.unlock();
    }
}