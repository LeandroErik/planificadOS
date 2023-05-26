#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <utils/socket/socket.h>

/**
 * @brief Inicia un servidor.
 *
 * @param ip IP de servidor.
 * @param puerto Puerto de servidor.
 *
 * @return Socket del servidor.
 */
int iniciar_servidor(char *ip, char *puerto);

/**
 * @brief Espera a que un cliente se conecte al servidor.
 *
 * @param socketServidor Socket del servidor.
 *
 * @return Socket del cliente.
 */
int esperar_cliente(int socketServidor);

/**
 * @brief Apaga el servidor.
 *
 * @param socketServidor Socket del servidor.
 */
void apagar_servidor(int socketServidor);

#endif