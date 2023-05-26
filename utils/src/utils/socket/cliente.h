#ifndef CLIENTE_H
#define CLIENTE_H

#include <utils/socket/socket.h>

/**
 * @brief Crea una conexión con un servidor específico.
 *
 * @param ip IP del servidor.
 * @param puerto Puerto del servidor.
 *
 * @return Socket del cliente.
 */
int crear_conexion_con_servidor(char *ip, char *puerto);

/**
 * @brief Libera la conexión con el servidor.
 *
 * @param socketCliente Socket del cliente.
 */
void liberar_conexion_con_servidor(int socketCliente);

/**
 * @brief Termina un programa "destruyendo" los parámetros (liberando espacio).
 *
 * @param socket Socket conectado al servidor.
 * @param config Configuración.
 * @param logger Logger.
 */
void terminar_programa(int socket, Config *config, Logger *logger);

#endif