#include "header.h"
using namespace std;
#define PORT 5432

/* Dado un socket, un nickname y el estado de login, registra un nuevo cliente con el nickname dado si el 
   mismo no se encuentra en uso. 
   En caso contrario, envia un mensaje indicando la falla. Además, actualiza 
   la variable log con el nuevo estado de login del cliente. */
// Client addClient(int s, const string& nickname){

//     /* COMPLETAR */
// }


// /* Dado un cliente, lo elimina del sistema y cierra su socket adecuadamanete(ver shutdown()) */
// void deleteClient(Client& c){

//    /* COMPLETAR */
// }

// /* Dado un nick, devuelve un puntero al cliente encontrado con dicho nickname. En caso de no existir,
//    el puntero es NULL */
// Client* getClient(const string& nick) {
//     /* COMPLETAR */
// }

// /* Dado un cliente y un mensaje, envía dicho mensaje a traves del socket asociado al cliente */
// void send(Client* c, const string& msg) {
//     /* COMPLETAR */
// }


/* Funcion que ejecutan los threads */
void connection_handler(int socket_desc){

    // int s = *((int*) socket_desc);
    int n;
    char str[MENSAJE_MAXIMO];

    /* Pedir login */
    
    string nickname;
    char s[] = { 'c', 'o', 'd', 'i', 'n', 'g' } ;
    /* Main loop */
    send(socket_desc, s, strlen(s), 0);
    while(1) {

        /* leer socket, salir si hubo error*/
        /* COMPLETAR */

        n = recv(socket_desc, str, MENSAJE_MAXIMO, 0);

        /* Parsear el buffer recibido*/
        /* COMPLETAR */
        cout << "aa: " << str << endl;

        /* Detectar el tipo de mensaje (crudo(solo texto) o comando interno(/..),
           y ejecutar la funcion correspondiente segun el caso */
        /* COMPLETAR */
    }

    while(1) {
        /* leer socket, salir si hubo error*/
        /* COMPLETAR */
        n = recv(socket_desc, str, MENSAJE_MAXIMO, 0);

        /* Parsear el buffer recibido*/
        /* COMPLETAR */
        cout << "El mensaje es: " << str << endl;

        /* Detectar el tipo de mensaje (crudo(solo texto) o comando interno(/..),
           y ejecutar la funcion correspondiente segun el caso */
        /* COMPLETAR */
    }
   
    // return NULL;  
}


int connection_setup(){

    int listening_socket, len;
    struct sockaddr_in local;
    thread threads[MAX_CLIENTS];

    /* Crear un socket de tipo UNIX con TCP (SOCK_STREAM). */
    if ((listening_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("creando socket");
        exit(1);
    }

    /* Establecer la dirección a la cual conectarse para escuchar. */
    local.sin_family = AF_INET;
    local.sin_port = htons(PORT);
    local.sin_addr.s_addr = INADDR_ANY;

    if (bind(listening_socket, (struct sockaddr *)&local, sizeof(local)) == -1) {
        perror("haciendo bind");
        exit(1);
    }

    /* Escuchar en el socket y permitir 5 conexiones en espera. */
    if (listen(listening_socket, 5) == -1) {
        perror("escuchando");
        exit(1);
    }

    printf("Server is on! \n");
    return listening_socket;
}


int main(void)
{
    struct sockaddr_un remote;
    int t = sizeof(remote);
    int i = 0;
    thread threads[MAX_CLIENTS];
    // Abrimos un socket para escuchar conexiones entrantes
    int s = connection_setup();
    int s1;
    while(1) 
    {    
        if ((s1 = accept(s, (struct sockaddr*) &remote, (socklen_t*) &t)) == -1) {
            perror("aceptando la conexión entrante");
            exit(1);
        }
        threads[i] = thread(connection_handler, s1);
        i++;
    }

    /* Cerramos las conexiones pendientes. */
    /* COMPLETAR */
  
    /* Cerramos la conexión que escucha. */
    close(s);

    return 0;
}

