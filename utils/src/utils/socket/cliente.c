#include <utils/socket/cliente.h>

int crear_conexion_con_servidor(char *ip, char *puerto)
{
  struct addrinfo hints, *servinfo;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  getaddrinfo(ip, puerto, &hints, &servinfo);

  int socketCliente = socket(servinfo->ai_family,
                             servinfo->ai_socktype,
                             servinfo->ai_protocol);

  bool conexion = !(connect(socketCliente, servinfo->ai_addr, servinfo->ai_addrlen));
  freeaddrinfo(servinfo);

  return conexion ? socketCliente : DESCONEXION;
}

void liberar_conexion_con_servidor(int socketCliente)
{
  close(socketCliente);
}

void terminar_programa(int socket, Config *config, Logger *logger)
{
  liberar_conexion_con_servidor(socket);
  config_destroy(config);
  log_destroy(logger);
}