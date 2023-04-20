#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <thread>

using namespace std;

class MessageThread{
public:
    MessageThread(int cd) : _cd(cd){};
    ~MessageThread(){};
    void handleMessage(){
        
        char buffer[80];
        ssize_t bytes;
        do{
            bytes = recv(_cd, buffer, 80, 0);
            if(bytes > 0) buffer[bytes] = '\0';
            else break;
            bytes = send(_cd, buffer, strlen(buffer), 0); 
        }while(bytes != -1);

        cout << "Conexion terminada\n";
        //close(_cd);
    };

private:
    int _cd;

};
int main(int argc ,char** argv)
{
    if(argc < 3){
        cerr << "No hay argumentos suficientes (" << argc << "), especifique dominio o ip y puerto\n";
        return -1;
    }

    struct addrinfo hints;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_flags = AI_PASSIVE;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;
    struct addrinfo* res;

    int sc = getaddrinfo(argv[1], argv[2], &hints, &res);

    if(sc != 0){
        cerr << "[getaddrinfo]: " << gai_strerror(sc) << "\n";
        return -1;
    }

    int sd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

    if(sd < 0){
        cout << "Error en la creacion del socket, cerrando el programa... Código de error: " << errno << "\n";
        freeaddrinfo(res);
        return -1;
    }

    int err = bind(sd,(struct sockaddr *) res->ai_addr, res->ai_addrlen);

    if(err == -1){
        cout << "Error en bind, cerrando el programa... Código de error: " << errno << "\n";
        freeaddrinfo(res);
        close(sd);
        return -1;
    }

    err = listen(sd, 5);

    if(err == -1){
        cout << "Error en listen, cerrando el programa... Código de error: " << errno << "\n";
        freeaddrinfo(res);
        close(sd);
        return -1;
    }
    char buffer[80];
    
    sockaddr cliente;
    socklen_t cliente_len = sizeof(struct sockaddr);
    char host[NI_MAXHOST];
    char serv[NI_MAXSERV];

    while(true){
        int cd = accept(sd, &cliente, &cliente_len);

        if(cd == -1){
            cout << "ERROR\n";
            continue;
        }

        getnameinfo(&cliente, cliente_len, host, NI_MAXHOST, 
            serv, NI_MAXSERV, NI_NUMERICHOST | NI_NUMERICSERV);

        cout << "Conexión desde " <<  host << ":" << serv << "\n";

        MessageThread* th = new MessageThread(cd);

        thread([th](){
            th->handleMessage();
            delete th;
        }).detach();

    }
    close(sd);
    freeaddrinfo(res);
    return 0;
}