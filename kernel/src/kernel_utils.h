#ifndef KERNEL_UTILS_H
#define KERNEL_UTILS_H

#include <utils/socket/servidor.h>
#include <utils/socket/cliente.h>
#include <kernel_config.h>
#include <kernel_thread.h>
#include <commons/string.h>
#include <time.h>

#include <proceso.h>

extern Logger *logger;
extern Logger *loggerPlanificacion;

extern int idProcesoGlobal;
extern Hilo hiloConsolas;

extern t_list *socketsConsola;

/**
 * @brief Inicia un logger en el Módulo Kernel.
 *
 * @return Logger.
 */
Logger *iniciar_logger_kernel();

/**
 * @brief Inicia el Servidor Kernel.
 *
 * @return Socket del cliente.
 */
int iniciar_servidor_kernel();

/**
 * @brief Rellena las líneas de instrucciones de un paquete y devuelve el tamaño del proceso.
 *
 * @param listaInstrucciones Lista de instrucciones.
 * @param socketConsola Socket de Consola.
 *
 * @return Tamaño del proceso.
 */
int rellenar_lista_instrucciones(Lista *listaInstrucciones, int socketCliente);

/**
 * @brief Crea un PCB.
 *
 * @param listaInstrucciones Lista de instrucciones.
 * @param tamanioProceso Tamaño del proceso.
 *
 * @return PCB.
 */
Pcb *crear_pcb(Lista *listaInstrucciones, int tamanioProceso);

/**
 * @brief Genera un PCB usando la lista de instrucciones pasada por el cliente.
 *
 * @param socketConsola Socket de Consola.
 *
 * @return PCB.
 */
Pcb *generar_pcb(int socketCliente);

/**
 * @brief Conecta con CPU usando el puerto de Dispatch.
 *
 * @return Socket de CPU en el puerto de Dispatch.
 */
int conectar_con_cpu_dispatch();

/**
 * @brief Conecta con CPU usando el puerto de Interrupt.
 *
 * @return Socket de CPU en el puerto de Interrupt.
 */
int conectar_con_cpu_interrupt();

/**
 * @brief Conecta con Memoria.
 *
 * @return Socket de Memoria.
 */
int conectar_con_memoria();

#endif