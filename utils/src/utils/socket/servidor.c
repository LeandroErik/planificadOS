#include <utils/socket/servidor.h>

int iniciar_servidor(char *ip, char *puerto)
{
  int socketServidor;

  struct addrinfo hints, *servinfo;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(ip, puerto, &hints, &servinfo);

  socketServidor = socket(servinfo->ai_family,
                          servinfo->ai_socktype,
                          servinfo->ai_protocol);

  bind(socketServidor, servinfo->ai_addr, servinfo->ai_addrlen);

  bool escucha = !(listen(socketServidor, SOMAXCONN));
  free(servinfo);

  return escucha ? socketServidor : DESCONEXION;
}

int esperar_cliente(int socketServidor)
{
  return accept(socketServidor, NULL, NULL);
}

void apagar_servidor(int socketServidor)
{
  close(socketServidor);
}