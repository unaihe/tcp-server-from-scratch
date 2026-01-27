# TCP Server from Scratch (C)

Este proyecto es una implementacion manual de un servidor TCP utilizando Sockets de Berkeley en C.

## Instrucciones de uso

1. Compilar:
   gcc server.c -o server

2. Ejecutar:
   ./server

3. Conectar cliente (en otra terminal):
   nc localhost 3490

## Tecnologias
- Lenguaje C (Sockets, netdb.h)
- Servidor concurrente con fork()
