#ifndef KERNEL_THREAD_H
#define KERNEL_THREAD_H

#include <kernel_utils.h>

/**
 * @brief Espera a que una consola se conecte al servidor y recibe sus instrucciones.
 *
 * @param socketKernel Socket de Kernel.
 */
void esperar_consola(int socketKernel);

/**
 * @brief Maneja un paquete enviado por consola.
 *
 * @param socketConsola Socket de Consola.
 */
void manejar_paquete_consola(int socketConsola);

/**
 * @brief Maneja la generación un PCB en base a lo recibido por consola y el envío al CPU via Dispatch.
 *
 * @param logger Logger.
 * @param socketConsola Socket de Consola.
 *
 * @return 0 si el PCB fue enviado correctamente, 1 en caso contrario.
 */
int manejar_envio_pcb(Logger *logger, int socketConsola);

/**
 * @brief Maneja una conexión con CPU usando el puerto de Interrupción.
 *
 */
void enviar_interrupcion(int socketInterrupt);

#endif