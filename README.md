# TCP Server from Scratch (C)

Este proyecto es una implementacion manual de un servidor TCP utilizando Sockets de Berkeley en C.
El codigo sigue la teoria y las practicas recomendadas de la "Beej's Guide to Network Programming".

## Instrucciones de uso

1. Compilar:
   gcc server.c -o server

2. Ejecutar:
   ./server

3. Conectar cliente (en otra terminal):
   nc localhost 3490

## Comandos disponibles
- "hola": El servidor te saluda.
- "admin": Acceso al modo secreto (ejemplo).
- "exit": Desconecta al cliente.

## Tecnologias
- Lenguaje C (Sockets, netdb.h)
- Servidor concurrente con fork()
- Gestion de procesos zombies con waitpid

## Referencias
Basado en las guias de Beej's Guide to Network Programming.
