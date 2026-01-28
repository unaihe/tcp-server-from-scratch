#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"  // El puerto donde escucharemos
#define BACKLOG 10   // Cuánta gente cabe en la cola de espera

// --- EL ENTERRADOR DE ZOMBIES ---
void sigchld_handler(int s)
{
    (void)s; // Ignorar warning
    int saved_errno = errno;
    while(waitpid(-1, NULL, WNOHANG) > 0);
    errno = saved_errno;
}

// --- EL TRADUCTOR DE IP (IPv4 vs IPv6) ---
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
    int sockfd, new_fd;  // Escuchar en sockfd, nueva conexión en new_fd
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr; // Info del cliente
    socklen_t sin_size;
    struct sigaction sa;
    int yes=1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    // 1. RELLENAR EL FORMULARIO
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // Usar mi IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // 2. BUSCAR UN SOCKET QUE FUNCIONE Y ENCHUFARLO (BIND)
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo); // Ya no necesitamos la lista

    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    // 3. ABRIR AL PÚBLICO
    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    // 4. ACTIVAR AL ENTERRADOR
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    printf("server: esperando conexiones...\n");

    // 5. BUCLE PRINCIPAL
    while(1) {  
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        if (!fork()) { 
            // --- HIJO ---
            close(sockfd); 
            
            char *bienvenida = "Bienvenido al servidor de Unai!\n";
            send(new_fd, bienvenida, strlen(bienvenida), 0);
            
            char buffer[1024];
            int numbytes;

            // Este bucle mantiene la charla viva con ESTE cliente.
            while(1) { 
                // Esperar mensaje
                numbytes = recv(new_fd, buffer, sizeof(buffer)-1, 0);
                // Si error o desconexión
                if (numbytes <= 0) {
                    if (numbytes == 0) printf("Server: Cliente desconectado.\n");
                    else perror("recv");
                    break; // Romper Bucle 2
                }
                buffer[numbytes] = '\0';
                buffer[strcspn(buffer, "\n")] = '\0';
                if (strncmp(buffer,"hola",4)==0){
                	char *msg= "hola\n";
                	send(new_fd, msg, 5, 0);
                }
                else if (strcmp(buffer, "admin") == 0) {
                	char *secret = "Servidor: *** MODO ADMIN ACTIVADO ***\nClave del sistema: 12345\n";
                	send(new_fd, secret, strlen(secret), 0);
            	}
            	else if (strncmp(buffer, "notas ", 6) == 0) {
		        char nota_guardada[20];
		        char *texto_nota = buffer + 6; 
			
		        printf("DEBUG: Intentando guardar '%s' en un espacio de 20 bytes...\n", texto_nota);
			//Al no comprobar nada se podrá copiar hasta el infinito
		        strcpy(nota_guardada, texto_nota);

		        char *msg = "Nota guardada (espero...)\n";
		        send(new_fd, msg, strlen(msg), 0);
            	}
            }

            close(new_fd); // Cierro mi canal privado
            exit(0);       // El hijo muere aquí
        }
        
        // --- SOY EL PADRE ---
        close(new_fd); // Cierro mi copia del canal privado y vuelvo arriba
    } 

    return 0;
}
