#ifndef PROCESO_H_
#define PROCESO_H_

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>

#include <utils/protocolo.h>
#include <kernel_utils.h>

/*Listas y colas de procesos*/
extern t_queue *colaNuevos;
extern t_queue *colaBloqueados;
extern t_list *colaListos;
extern t_queue *colaEjecutando;
extern t_queue *colaFinalizado;
extern t_queue *colaSuspendidoListo;

/*semaforos*/
extern pthread_mutex_t mutexNumeroProceso;
extern pthread_mutex_t mutexProcesoListo;

extern pthread_mutex_t mutexColaNuevos;
extern pthread_mutex_t mutexColaListos;
extern pthread_mutex_t mutexColaBloqueados;
extern pthread_mutex_t mutexColaEjecutando;
extern pthread_mutex_t mutexColaFinalizado;
extern pthread_mutex_t mutexColaSuspendidoListo;

extern pthread_mutex_t mutex_cola;
extern pthread_mutex_t mutexcantidadProcesosMemoria;

extern Semaforo semaforoProcesoNuevo;
extern Semaforo semaforoProcesoListo;
extern Semaforo semaforoProcesoEjecutando;

extern Semaforo contadorBloqueados;

extern Semaforo analizarSuspension;
extern Semaforo suspensionFinalizada;
extern Semaforo despertarPlanificadorLargoPlazo;

extern Semaforo semaforoCantidadProcesosEjecutando;
extern Semaforo comunicacionMemoria;

extern int socketMemoria;
extern int socketDispatch;
extern int socketInterrupt;
extern int socketKernel;

/*Hilos*/
extern Hilo hilo_planificador_largo_plazo;
extern Hilo hilo_planificador_corto_plazo;
extern Hilo hilo_dispositivo_io;

/*Contador de procesos en memoria*/
extern int cantidadProcesosEnMemoria;

/*Funciones del proceso*/
void ejecutar(Pcb *);

/*Funciones de inicializacion*/
void inicializar_colas_procesos();
void iniciar_planificadores();
void inicializar_semaforos();

/*Planificadores*/
void *planificador_largo_plazo();
bool sePuedeAgregarMasProcesos();
void *planificador_mediano_plazo();
void *planificador_corto_plazo_fifo();

void *planificador_corto_plazo_srt();

void *dispositivo_io();

void *monitorizarSuspension(Pcb *);

/*Transiciones*/
void agregar_proceso_nuevo(Pcb *);
void agregar_proceso_listo(Pcb *);
void agregar_proceso_bloqueado(Pcb *);
void agregar_proceso_ejecutando(Pcb *);
void agregar_proceso_suspendido_bloqueado(Pcb *);
void agregar_proceso_suspendido_listo(Pcb *);
void agregar_proceso_finalizado(Pcb *);
Pcb *extraer_proceso_nuevo();
Pcb *extraer_proceso_suspendido_listo();

/*Planificacion SRT*/
Pcb *sacar_proceso_mas_corto();
float obtener_tiempo_de_trabajo_actual(Pcb *);
bool ordenar_segun_tiempo_de_trabajo(void *, void *);

/*Varios*/
void enviar_pcb(Pcb *, int);
void *queue_peek_at(t_queue *elf, int);
char *leer_cola(t_queue *);
char *leer_lista(t_list *);

/*Monitores de variables globales*/
void incrementar_cantidad_procesos_memoria();
void decrementar_cantidad_procesos_memoria();
int cantidad_procesos_memoria();

int lectura_cola_mutex(t_queue *, pthread_mutex_t *);

/**
 * @brief Imprime las colas de planificacion
 *
 */
void imprimir_colas();

/**
 * @brief saca al proceso de la cola de ejecucion
 *
 */
Pcb *sacar_proceso_ejecutando();

/**
 * @brief saca al proceso de la cola de bloqueados
 *
 */
Pcb *sacar_proceso_bloqueado();

/**
 * @brief Dependiendo del estado del pcb se lo agrega a una de las colas.
 *
 */
void manejar_proceso_recibido(Pcb *, int);

Pcb *sacar_proceso_listo();

/**
 * @brief Instante actual en segundos
 *
 *@returns segundos desde 01 / 01 / 1970
 */
int obtener_tiempo_actual();

int tiempo_total_bloqueado();

void manejar_proceso_interrumpido(Pcb *);

int tabla_pagina_primer_nivel(int pid, int tamanio);

void liberar_estructuras();
void liberar_semaforos();
void liberar_conexiones();
bool es_SRT();
bool esProcesoNuevo(Pcb *);

int calcular_tiempo_rafaga_real_anterior(Pcb *);

bool procesoSigueBloqueado(int);

void imprimir_pcb(Pcb *);

bool buscar_pcb_cola(int);

#endif