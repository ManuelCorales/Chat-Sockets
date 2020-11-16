#include "header.h"
using namespace std;
#define PORT 5432

vector<Cliente> clientes;
int cantidadMensajes;
mutex mtxModificacionClientes;

Cliente traerClientePorNombre(string nombre);

void ejecutarComando(string comando, string nombre){
    string resultadoComando;
    // Devuelve al usuario el listado de todos los usuarios conectados
    if(comando == "/list"){
        mtxModificacionClientes.lock();
        resultadoComando = "Las personas activas en el chat son: \r\n";
        for(int i = 0; i < clientes.size(); i ++){
            resultadoComando += clientes[i].nombre + "\r\n";
        }
        mtxModificacionClientes.unlock();

    // Devuelve la cantidad de mensajes enviados por todos (no se cuentan los comandos)
    } else if(comando == "/cantmensajes") {
        resultadoComando = "La cantidad de mensajes totales es: " + to_string(cantidadMensajes) + "\r\n";

    // Manda un mensaje privado a algún conectado. Sintaxis: /msgpriv -destinatario- -mensaje-
    } else if(comando.substr(0, 8) == "/msgpriv") {
        if(comando.size() < 10) {
            resultadoComando = "Especifique destinatario \r\n";
        } else {
            size_t lugarNombre = comando.find(" ");
            string destinatario = comando.substr(lugarNombre + 1, comando.find(" ", lugarNombre + 1) - lugarNombre - 1);

            bool destinatarioEncontrado = false;
            mtxModificacionClientes.lock();
            for(int i = 0; i < clientes.size(); i++){
                if (clientes[i].nombre == destinatario) {
                    destinatarioEncontrado = true;
                }
            }
            if (string("/msgpriv " + destinatario + " ").size() >= comando.size()) {
                resultadoComando = "Especifique un mensaje\r\n";
            } else if(destinatarioEncontrado){
                resultadoComando = "Mensaje enviado correctamente\r\n";
                traerClientePorNombre(destinatario).enviar(string("Mensaje privado de " + nombre + ": " + comando.substr(comando.find(" ", lugarNombre + 1)) + "\r\n").c_str());
            } else if (!destinatarioEncontrado) {
                resultadoComando = "Usuario no conectado\r\n";
            }
            mtxModificacionClientes.unlock();
        }
    } else if(comando == "/help") {
        resultadoComando = "\r\nComandos disponibles: \r\n/list: *Devuelve al usuario el listado de todos los usuarios conectados*\r\n/cantmensajes: *Devuelve la cantidad de mensajes enviados por todos (no se cuentan los comandos)*\r\n/msgpriv -destinatario- -mensaje-: *Manda un mensaje privado a algún conectado sintaxis*\r\n/help: *Devuelve este mensaje*\r\n";
    } else {
        resultadoComando = "Comando no reconocido, pruebe con el comando /help\r\n";
    }

    // Devuelvo resultado del comando
    traerClientePorNombre(nombre).enviar(resultadoComando.c_str());
}


/* Busca al cliente por su nombre */
Cliente traerClientePorNombre(string nombre){
    for(int i = 0; i < clientes.size(); i++){
        if (clientes[i].nombre == nombre) {

            return clientes[i];
        }
    }
    return Cliente(-1, "error");
}

/* Funcion que ejecutan los threads */
void connection_handler(int socket_desc){

    int n;
    char str[MENSAJE_MAXIMO];
    mutex mtxSumaCantidadMensajes;
    string nickname;
    string mensajeNombre = "Decime tu nombre\r\n";
    string nombre = "";
    bool nombreRepetido;
    /* While que pide login hasta que se ahaga correctamente*/
    while(1) {

        nombreRepetido = false;
        
        send(socket_desc, mensajeNombre.c_str(), strlen(mensajeNombre.c_str()), 0);
        n = recv(socket_desc, str, MENSAJE_MAXIMO, 0);
        // Si hay un problema del otro lado cierro el socket
        if (n <= 0){
            close(socket_desc);
            return;
        }

        // Verifico la unicidad del nombre 
        mtxModificacionClientes.lock();
        for(int i = 0; i < clientes.size(); i++){
            if (clientes[i].nombre == string(str)) {
                nombreRepetido = true;
                break;
            }
        }
        mtxModificacionClientes.unlock();

        // Si el nombre está repetio entonces le digo eso al cliente
        if(nombreRepetido){
            string mensajeADevolver = "Ese nombre ya está en uso en el chat\r\n";
            send(socket_desc, mensajeADevolver.c_str(), strlen(mensajeADevolver.c_str()), 0);
            continue; 
        } else {
            nombre = string(str);
            clientes.push_back(Cliente(socket_desc, nombre));
            traerClientePorNombre(nombre).enviar(string("Bienvenido al chat\r\n").c_str());
            break;
        }
    }

    // Lumpio str
    memset(str, 0, MENSAJE_MAXIMO);

    while(1) {
        // Recibo mensaje
        n = recv(socket_desc, str, MENSAJE_MAXIMO, 0);
        
        // Si hay un problema del otro lado cierro el socket
        if (n <= 0){
            mtxModificacionClientes.lock();
            for(int i = 0; i < clientes.size() ; i++){
                if(clientes[i].nombre == nombre) {
                    close(socket_desc);
                    clientes.erase(clientes.begin() + i);
                    mtxModificacionClientes.unlock();
                    return;
                }
            }
        }
    
        string mensaje(str);
        
        // Si el mensaje comienza con un / llamo a la función ejecutarComando
        if(mensaje.substr(0, 1) == "/"){
            ejecutarComando(mensaje, nombre);
        } else {
            // Envío el mensaje a todos
            mtxSumaCantidadMensajes.lock();
            cantidadMensajes += 1;
            mtxSumaCantidadMensajes.unlock();
            mensaje = traerClientePorNombre(nombre).nombre + ": " + mensaje + "\r\n";
            for(int i = 0; i < clientes.size(); i++){
                clientes[i].enviar(mensaje.c_str());
            }
            cout << traerClientePorNombre(nombre).nombre << ": " << str << endl;
        }
        
        /* Limpio la variable str*/
        memset(str, 0, MENSAJE_MAXIMO);
    }
}

/*Setea toda la conexion*/
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
    mutex mtxRegistroUsuario;
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
        // Crea un nuevo thread para que atienda a ese socket
        threads[i] = thread(connection_handler, s1);
        i++;
    }
    return 0;
}

