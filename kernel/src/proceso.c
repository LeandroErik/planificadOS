#include <proceso.h>
#include <kernel_utils.h>

Logger *logger;
Logger *loggerPlanificacion;

int idProcesoGlobal;
Hilo hiloConsolas;

t_list *socketsConsola;

/*Listas y colas de procesos*/
t_queue *colaNuevos;
t_queue *colaBloqueados;
t_list *colaListos;
t_queue *colaEjecutando;
t_queue *colaFinalizado;
t_queue *colaSuspendidoListo;

/*semaforos*/
pthread_mutex_t mutexNumeroProceso;
pthread_mutex_t mutexProcesoListo;

pthread_mutex_t mutexColaNuevos;
pthread_mutex_t mutexColaListos;
pthread_mutex_t mutexColaBloqueados;
pthread_mutex_t mutexColaEjecutando;
pthread_mutex_t mutexColaFinalizado;
pthread_mutex_t mutexColaSuspendidoListo;

pthread_mutex_t mutex_cola;
pthread_mutex_t mutexcantidadProcesosMemoria;

Semaforo semaforoProcesoNuevo;
Semaforo semaforoProcesoListo;
Semaforo semaforoProcesoEjecutando;

Semaforo contadorBloqueados;

Semaforo analizarSuspension;
Semaforo suspensionFinalizada;
Semaforo despertarPlanificadorLargoPlazo;

Semaforo semaforoCantidadProcesosEjecutando;
Semaforo comunicacionMemoria;

int socketDispatch;
int socketInterrupt;
int socketKernel;

/*Hilos*/
Hilo hilo_planificador_largo_plazo;
Hilo hilo_planificador_corto_plazo;
Hilo hilo_dispositivo_io;
/*Contador de procesos en memoria*/
int cantidadProcesosEnMemoria;

/*Inicializadores*/
void inicializar_semaforos()
{
    pthread_mutex_init(&mutexNumeroProceso, NULL);

    pthread_mutex_init(&mutexColaNuevos, NULL);
    pthread_mutex_init(&mutexColaListos, NULL);
    pthread_mutex_init(&mutexColaEjecutando, NULL);
    pthread_mutex_init(&mutexColaBloqueados, NULL);
    pthread_mutex_init(&mutexColaSuspendidoListo, NULL);
    pthread_mutex_init(&mutexColaFinalizado, NULL);

    sem_init(&semaforoProcesoNuevo, 0, 0);
    sem_init(&semaforoProcesoListo, 0, 0);

    sem_init(&contadorBloqueados, 0, 0);

    sem_init(&analizarSuspension, 0, 0);
    sem_init(&suspensionFinalizada, 0, 0);

    sem_init(&semaforoCantidadProcesosEjecutando, 0, 1);

    sem_init(&despertarPlanificadorLargoPlazo, 0, 0);

    sem_init(&comunicacionMemoria, 0, 1);
}

void inicializar_colas_procesos()
{
    colaNuevos = queue_create();
    colaListos = list_create();
    colaEjecutando = queue_create();
    colaBloqueados = queue_create();
    colaSuspendidoListo = queue_create();
    colaFinalizado = queue_create();
}

void iniciar_planificadores()
{
    pthread_create(&hilo_planificador_largo_plazo, NULL, planificador_largo_plazo, NULL);
    pthread_detach(hilo_planificador_largo_plazo);

    if (strcmp(KERNEL_CONFIG.ALGORITMO_PLANIFICACION, "FIFO") != 0)
    {
        pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo_srt, NULL);
        pthread_detach(hilo_planificador_corto_plazo);
    }
    else
    {
        pthread_create(&hilo_planificador_corto_plazo, NULL, planificador_corto_plazo_fifo, NULL);
        pthread_detach(hilo_planificador_corto_plazo);
    }

    pthread_create(&hilo_dispositivo_io, NULL, dispositivo_io, NULL);
    pthread_detach(hilo_dispositivo_io);
}

/*Funciones del proceso*/

void ejecutar(Pcb *proceso)
{
    enviar_pcb(proceso, socketDispatch);

    log_info(logger, "Envio proceso con PID: [%d] por CPU-Dispatch.", proceso->pid);

    CodigoOperacion codOperacion = obtener_codigo_operacion(socketDispatch);

    Pcb *procesoRecibido;

    switch (codOperacion)
    {
    case PCB:
        procesoRecibido = deserializar_pcb(socketDispatch);
        log_info(logger, "Recibi proceso con PID: %d de CPU-Dispatch.", proceso->pid);
        manejar_proceso_recibido(procesoRecibido, socketDispatch);

        break;

    case DESCONEXION:
        log_info(logger, "Se desconectó el CPU-Dispatch. %d", codOperacion);
        break;

    default:
        log_warning(logger, "Operación desconocida.");
        break;
    }
}

void manejar_proceso_recibido(Pcb *pcb, int socketDispatch)
{
    Pcb *actual = sacar_proceso_ejecutando();
    liberar_pcb(actual);

    int pid;
    Paquete *paquete;
    Semaforo semaforoSuspension;
    // imprimir_pcb(pcb);

    switch (pcb->escenario->estado)
    {
    case INTERRUPCION_EXTERNA:

        // log_info(loggerPlanificacion, "Tiempo ejecucion Proceso: [%d] %d seg.", pcb->pid, obtener_tiempo_actual() - pcb->tiempoInicioEjecucion);
        manejar_proceso_interrumpido(pcb);

        break;

    case BLOQUEADO_IO:

        sem_init(&semaforoSuspension, 0, 1);
        pcb->confirmacionSuspencion = semaforoSuspension;

        agregar_proceso_bloqueado(pcb);

        // Comienza el analisis de suspension (10 segundos)
        Hilo hiloMonitorizacionSuspension;
        pthread_create(&hiloMonitorizacionSuspension, NULL, (void *)monitorizarSuspension, pcb);
        pthread_detach(hiloMonitorizacionSuspension);
        break;

    case TERMINADO:
        agregar_proceso_finalizado(pcb);

        pid = pcb->pid;
        paquete = crear_paquete(FINALIZAR_PROCESO);
        agregar_a_paquete(paquete, &pid, sizeof(unsigned int));

        decrementar_cantidad_procesos_memoria();

        // Libero el pcb a medida que va finalizando
        liberar_pcb(queue_pop(colaFinalizado));

        // Aviso finalizacion a consola
        int socketConsola = (int)list_get(socketsConsola, pid);
        enviar_paquete_a_cliente(paquete, socketConsola);

        eliminar_paquete(paquete);
        free(confirmacion);

        break;

    default:
        log_warning(loggerPlanificacion, "El proceso %d con operacion desconocida.", pcb->pid);
        break;
    }
}

void manejar_proceso_interrumpido(Pcb *pcb)
{
    Pcb *pcbEjecutar = pcb;
    // comparar lo que le falta con las estimaciones de los listos

    int tiempoQueYaEjecuto = obtener_tiempo_actual() - pcb->tiempoInicioEjecucion;

    int estimacionEnSegundos = obtener_tiempo_de_trabajo_actual(pcb) / 1000;

    float tiempoRestanteEnSegundos = estimacionEnSegundos - tiempoQueYaEjecuto;

    // log_info(loggerPlanificacion, "[INTERRUPCION] Proceso: [%d] ,(cpu: %d), estimación restante: %.2f.", pcb->pid, tiempoQueYaEjecuto, tiempoRestanteEnSegundos);

    if (!list_is_empty(colaListos))
    {
        // log_info(loggerPlanificacion, "[INTERRUPCION] Proceso: [%d] fue INTERRUPIDO.", pcb->pid);
        //  Ordeno segun esos valores
        list_sort(colaListos, &ordenar_segun_tiempo_de_trabajo);

        log_info(loggerPlanificacion, "[INTERRUPCION] Se reordenó la cola Listos.");

        log_info(loggerPlanificacion, "[INTERRUPCION] Analizando si suspender o ejecutar al actual.");
        // Falta aplicar mutua exclusion
        Pcb *pcbMasCortoDeListos = list_get(colaListos, 0);

        float tiempoPcbMasCortoEnSegundos = obtener_tiempo_de_trabajo_actual(pcbMasCortoDeListos) / 1000;
        // log_info(loggerPlanificacion, "[INTERRUPCION]El menor de los LISTOS [%d], con estimacion: %.2f.", pcbMasCortoDeListos->pid, tiempoPcbMasCortoEnSegundos);

        if (tiempoRestanteEnSegundos > tiempoPcbMasCortoEnSegundos)
        {
            log_info(loggerPlanificacion, "[INTERRUPCION] Proceso: [%d]  es desalojado por [%d]! con menor SRT.", pcbEjecutar->pid, pcbMasCortoDeListos->pid);
            pcbEjecutar = sacar_proceso_mas_corto(colaListos);

            pthread_mutex_lock(&mutexColaListos);

            pcb->escenario->estado = LISTO;
            list_add(colaListos, pcb);
            log_info(loggerPlanificacion, "Proceso: [%d] se movio LISTO.", pcb->pid);

            pthread_mutex_unlock(&mutexColaListos);
            // aca pongo a ejecutar al mas corto,caso contrario sigue ejecutando el otro
        }
    }

    // log_info(loggerPlanificacion, "[INTERRUPCION] Proceso:[%d] se ejecuta con puntero en %d", pcbEjecutar->pid, pcbEjecutar->contadorPrograma);

    agregar_proceso_ejecutando(pcbEjecutar);

    ejecutar(pcbEjecutar);
}
void *monitorizarSuspension(Pcb *proceso)
{

    int pid = proceso->pid;
    int tiempoMaximoBloqueo = KERNEL_CONFIG.TIEMPO_MAXIMO_BLOQUEADO;
    // Pasados 10 segundos de bloqueado ,se suspende.
    int tiempoMaximoBloqueoEnMicrosegundos = tiempoMaximoBloqueo * 1000;

    usleep(tiempoMaximoBloqueoEnMicrosegundos);
    int valor = 0;
    sem_getvalue(&(proceso->confirmacionSuspencion), &valor);

    if (valor == 1) // esto indica que aun sigue en la cola de bloqueados.
    {

        Paquete *paquete = crear_paquete(SUSPENDER_PROCESO);

        agregar_a_paquete(paquete, &(proceso->pid), sizeof(int));
        sem_wait(&(proceso->confirmacionSuspencion));
        sem_wait(&comunicacionMemoria);

        enviar_paquete_a_servidor(paquete, socketMemoria);
        log_info(loggerPlanificacion, "Se envio el proceso %d a la memoria para suspender", pid);

        char *confirmacion = obtener_mensaje_del_servidor(socketMemoria); // confirmacion de suspension
        log_info(loggerPlanificacion, "Recibi respuesta de memoria para suspender proceso %s.", confirmacion);
        sem_post(&comunicacionMemoria);

        log_info(loggerPlanificacion, "Proceso: [%d],se movio a SUSPENDIDO-BLOQUEADO", proceso->pid);
        proceso->escenario->estado = SUSPENDIDO;
        proceso->vieneDeSuspension = true;

        decrementar_cantidad_procesos_memoria();

        sem_post(&(proceso->confirmacionSuspencion));

        free(confirmacion);
        eliminar_paquete(paquete);
    }
    return NULL;
}

bool procesoSigueBloqueado(int _pid)
{
    if (lectura_cola_mutex(colaBloqueados, &mutexColaBloqueados) > 0)
    {
        if (buscar_pcb_cola(_pid))
        {
            return true;
        }
        else
        {
            return false;
        }
    }

    return false;
}
bool buscar_pcb_cola(int _pid)
{

    for (int i = 0; i < queue_size(colaBloqueados); i++)
    {
        if (((Pcb *)queue_peek_at(colaBloqueados, i))->pid == _pid)
        {
            return true;
        }
    }
    return false;
}

/*Varios*/

void enviar_pcb(Pcb *proceso, int socketDispatch)
{
    Paquete *paquete = crear_paquete(PCB);

    serializar_pcb(paquete, proceso);

    enviar_paquete_a_servidor(paquete, socketDispatch);

    eliminar_paquete(paquete);
}

void *queue_peek_at(t_queue *self, int index)
{
    return list_get(self->elements, index);
}

/*Planificadores*/

void *dispositivo_io()
{
    while (1)
    {

        sem_wait(&contadorBloqueados);

        Pcb *proceso = queue_peek(colaBloqueados); // Aun sigue en la cola de bloqueado.

        int tiempoBloqueo = proceso->escenario->tiempoBloqueadoIO;

        log_info(loggerPlanificacion, "----------[DISP I/O] Proceso: [%d] ,se bloqueara %d segundos.----------", proceso->pid, tiempoBloqueo / 1000);
        // Bloqueo el proceso.
        int tiempoBloqueoEnMicrosegundos = tiempoBloqueo * 1000;

        usleep(tiempoBloqueoEnMicrosegundos);

        log_info(loggerPlanificacion, "----------[DISP I/O] Proceso: [%d] ,termino I/O %d segundos.----------", proceso->pid, tiempoBloqueo / 1000);
        // Si el proceso actualmente bloqueado ,espera swapeo.
        sem_wait(&(proceso->confirmacionSuspencion));
        sem_post(&(proceso->confirmacionSuspencion));

        // Si el proceso actualmente bloqueado ,espera swapeo.
        sem_wait(&(proceso->confirmacionSuspencion)); // setea el valor de del semaforo en 0 si no necesita suspension,porque ya termino de IO

        proceso = sacar_proceso_bloqueado();

        // Analizo si fue suspendido.
        if (proceso->escenario->estado == SUSPENDIDO)
        {
            agregar_proceso_suspendido_listo(proceso);
        }
        else
        {
            agregar_proceso_listo(proceso);
        }
    }
}
bool es_SRT()
{
    return strcmp(KERNEL_CONFIG.ALGORITMO_PLANIFICACION, "SRT") == 0;
}

void *planificador_largo_plazo()
{
    log_info(loggerPlanificacion, "Inicio planificacion LARGO PLAZO en [%s]", KERNEL_CONFIG.ALGORITMO_PLANIFICACION);
    while (1)
    {
        sem_wait(&despertarPlanificadorLargoPlazo);
        // log_info(loggerPlanificacion, "[LARGO-PLAZO] Procesos en MEMORIA: %d", cantidadProcesosEnMemoria);

        if (sePuedeAgregarMasProcesos())
        {
            Pcb *procesoSaliente;

            procesoSaliente = queue_is_empty(colaSuspendidoListo) ? extraer_proceso_nuevo() : extraer_proceso_suspendido_listo();

            // Agrego verificacion de que sea nuevo,porque puede que entre a listo de suspendido.
            if (esProcesoNuevo(procesoSaliente))
            {
                int tablaPaginasPrimerNivel = tabla_pagina_primer_nivel(procesoSaliente->pid, procesoSaliente->tamanio);
                procesoSaliente->tablaPaginas = tablaPaginasPrimerNivel;
            }

            agregar_proceso_listo(procesoSaliente);

            incrementar_cantidad_procesos_memoria();
        }
    }
}
bool esProcesoNuevo(Pcb *proceso)
{
    return proceso->escenario->estado == NUEVO;
}

bool sePuedeAgregarMasProcesos()
{
    return (cantidad_procesos_memoria() < KERNEL_CONFIG.GRADO_MULTIPROGRAMACION) && (lectura_cola_mutex(colaNuevos, &mutexColaNuevos) > 0 || lectura_cola_mutex(colaSuspendidoListo, &mutexColaSuspendidoListo) > 0);
}

void *planificador_corto_plazo_fifo()
{
    // log_info(loggerPlanificacion, "INICIO PLANIFICACION FIFO");
    while (1)
    {
        sem_wait(&semaforoProcesoListo);
        sem_wait(&semaforoCantidadProcesosEjecutando);

        Pcb *procesoEjecutar = sacar_proceso_listo();

        agregar_proceso_ejecutando(procesoEjecutar);

        ejecutar(procesoEjecutar);
    }
}

void *planificador_corto_plazo_srt()
{
    // log_info(logger, "INICIO PLANIFICACION SRT");

    while (1)
    {

        sem_wait(&semaforoProcesoListo);
        sem_wait(&semaforoCantidadProcesosEjecutando);

        Pcb *procesoEjecutar = sacar_proceso_mas_corto();

        agregar_proceso_ejecutando(procesoEjecutar);

        ejecutar(procesoEjecutar);
    }
}
Pcb *sacar_proceso_mas_corto()
{

    Pcb *pcbSaliente;
    /*Replanifico la cola de listos*/

    pthread_mutex_lock(&mutexColaListos);

    list_sort(colaListos, &ordenar_segun_tiempo_de_trabajo);

    pcbSaliente = list_remove(colaListos, 0);

    // log_info(loggerPlanificacion, "\n[REPLANIFICACION] Proceso:[%d] rafaga anterior: %d ,estimacion anterior: %.2f-> estimacion actual: %.2f \n", pcbSaliente->pid, pcbSaliente->tiempoRafagaRealAnterior, (pcbSaliente->estimacionRafaga / 1000), obtener_tiempo_de_trabajo_actual(pcbSaliente) / 1000);

    pthread_mutex_unlock(&mutexColaListos);

    return pcbSaliente;
}

/*Funciones para aniadir proceso a cola*/

void agregar_proceso_nuevo(Pcb *procesoNuevo)
{
    pthread_mutex_lock(&mutexColaNuevos);

    queue_push(colaNuevos, procesoNuevo);
    log_info(loggerPlanificacion, "Proceso: [%d] se movio a NUEVO.", procesoNuevo->pid);

    pthread_mutex_unlock(&mutexColaNuevos);

    // Despierto al Planificador de Largo Plazo
    sem_post(&despertarPlanificadorLargoPlazo);

    // imprimir_colas();
}
void agregar_proceso_listo(Pcb *procesoListo)
{
    pthread_mutex_lock(&mutexColaListos);

    procesoListo->escenario->estado = LISTO;
    list_add(colaListos, procesoListo);
    log_info(loggerPlanificacion, "Proceso: [%d] se movio LISTO.", procesoListo->pid);

    pthread_mutex_unlock(&mutexColaListos);
    // Envio interrupcion por cada vez que que entra uno a ready

    if (es_SRT())
    {
        enviar_interrupcion(socketInterrupt);
    }
    sem_post(&semaforoProcesoListo);

    // imprimir_colas();
}

int tabla_pagina_primer_nivel(int pid, int tamanio)
{

    Paquete *paquete = crear_paquete(PROCESO_NUEVO);

    agregar_a_paquete(paquete, &pid, sizeof(unsigned int));
    agregar_a_paquete(paquete, &tamanio, sizeof(unsigned int));
    sem_wait(&comunicacionMemoria);
    enviar_paquete_a_servidor(paquete, socketMemoria);

    log_info(logger, "Se envio el proceso %d a la memoria", pid);

    char *mensajeDeMemoria;
    int tablaPrimerNivel;

    mensajeDeMemoria = obtener_mensaje_del_servidor(socketMemoria);
    sem_post(&comunicacionMemoria);
    tablaPrimerNivel = atoi(mensajeDeMemoria);
    log_info(logger, "Se recibio de memoria la tabla de primer nivel %d del proceso", tablaPrimerNivel);

    eliminar_paquete(paquete);
    free(mensajeDeMemoria);

    return tablaPrimerNivel;
}

void agregar_proceso_ejecutando(Pcb *procesoEjecutando)
{
    pthread_mutex_lock(&mutexColaEjecutando);
    if (procesoEjecutando->escenario->estado != INTERRUPCION_EXTERNA)
    {
        // Aca si pongo tiempo inicio ejecucion asi no cambia por cada interrupcion(acepta que venga de LISTO)
        procesoEjecutando->tiempoInicioEjecucion = obtener_tiempo_actual();
        // log_info(loggerPlanificacion, "Proceso: [%d] inicio EJECUCION .", procesoEjecutando->pid);
    }
    procesoEjecutando->escenario->estado = EJECUTANDO;

    queue_push(colaEjecutando, procesoEjecutando);
    log_info(loggerPlanificacion, "Proceso: [%d] se movio EJECUTANDO.", procesoEjecutando->pid);

    pthread_mutex_unlock(&mutexColaEjecutando);

    // imprimir_colas();
}

int calcular_tiempo_rafaga_real_anterior(Pcb *proceso)
{
    return obtener_tiempo_actual() - proceso->tiempoInicioEjecucion;
}
void agregar_proceso_bloqueado(Pcb *procesoBloqueado)
{
    procesoBloqueado->estimacionRafaga = obtener_tiempo_de_trabajo_actual(procesoBloqueado);
    procesoBloqueado->tiempoRafagaRealAnterior = calcular_tiempo_rafaga_real_anterior(procesoBloqueado);
    // log_info(loggerPlanificacion, "Proceso: [%d] (CPU ant. %d ,Rafaga ant.  %.2f)", procesoBloqueado->pid, procesoBloqueado->tiempoRafagaRealAnterior, (procesoBloqueado->estimacionRafaga / 1000));
    //  log_info(loggerPlanificacion, "Proceso [%d] estimacion rafaga anterior:  %.2f", procesoBloqueado->pid, (procesoBloqueado->estimacionRafaga / 1000));

    pthread_mutex_lock(&mutexColaBloqueados);

    procesoBloqueado->escenario->estado = BLOQUEADO_IO;

    queue_push(colaBloqueados, procesoBloqueado);
    log_info(loggerPlanificacion, "Proceso: [%d] se movio a BLOQUEADO.", procesoBloqueado->pid);

    pthread_mutex_unlock(&mutexColaBloqueados);

    // Despierto dispositivo I/O
    sem_post(&contadorBloqueados);

    // Despierto al planificador de largo plazo

    sem_post(&despertarPlanificadorLargoPlazo);

    // imprimir_colas();
}

void agregar_proceso_finalizado(Pcb *procesoFinalizado)
{
    pthread_mutex_lock(&mutexColaFinalizado);

    queue_push(colaFinalizado, procesoFinalizado);
    log_info(loggerPlanificacion, "Proceso: [%d] se encuentra FINALIZADO.", procesoFinalizado->pid);

    pthread_mutex_unlock(&mutexColaFinalizado);

    // Despierto al planificador de mediano plazo.
    sem_post(&despertarPlanificadorLargoPlazo);

    // imprimir_colas();
}

void agregar_proceso_suspendido_listo(Pcb *procesoSuspendidoListo)
{
    pthread_mutex_lock(&mutexColaSuspendidoListo);

    procesoSuspendidoListo->escenario->estado = LISTO;
    queue_push(colaSuspendidoListo, procesoSuspendidoListo);
    log_info(loggerPlanificacion, "Proceso: [%d] se encuentra SUSPENDIDO-LISTO.", procesoSuspendidoListo->pid);

    pthread_mutex_unlock(&mutexColaSuspendidoListo);

    // Despierto al Planificador de Largo Plazo
    sem_post(&despertarPlanificadorLargoPlazo);

    // imprimir_colas();
}

/*Funciones para sacar procesos a cola.*/

Pcb *sacar_proceso_ejecutando()
{
    Pcb *pcbSaliente;
    pthread_mutex_lock(&mutexColaEjecutando);

    pcbSaliente = queue_pop(colaEjecutando);
    log_info(loggerPlanificacion, "Proceso: [%d] salío de EJECUTANDO.", pcbSaliente->pid);

    pthread_mutex_unlock(&mutexColaEjecutando);

    // le aviso al planificador de corto plazo
    sem_post(&semaforoCantidadProcesosEjecutando);

    // imprimir_colas();
    return pcbSaliente;
}

Pcb *sacar_proceso_bloqueado()
{
    Pcb *pcbSaliente;

    pthread_mutex_lock(&mutexColaBloqueados);

    pcbSaliente = queue_pop(colaBloqueados);
    log_info(loggerPlanificacion, "Proceso: [%d] salío de BLOQUEADO. (real ant : %d)", pcbSaliente->pid, pcbSaliente->tiempoRafagaRealAnterior);

    pthread_mutex_unlock(&mutexColaBloqueados);

    return pcbSaliente;
}

Pcb *sacar_proceso_listo()
{

    pthread_mutex_lock(&mutexColaListos);
    Pcb *pcbSaliente = list_remove(colaListos, 0);
    log_info(loggerPlanificacion, "Proceso: [%d] salío de LISTO.", pcbSaliente->pid);
    pthread_mutex_unlock(&mutexColaListos);

    return pcbSaliente;
}

Pcb *extraer_proceso_nuevo()
{
    pthread_mutex_lock(&mutexColaNuevos);

    Pcb *pcbSaliente = queue_pop(colaNuevos);
    log_info(loggerPlanificacion, "Proceso: [%d] salío de NUEVO.", pcbSaliente->pid);

    pthread_mutex_unlock(&mutexColaNuevos);

    return pcbSaliente;
}

Pcb *extraer_proceso_suspendido_listo()
{
    pthread_mutex_lock(&mutexColaSuspendidoListo);

    Pcb *pcbSaliente = queue_pop(colaSuspendidoListo);
    log_info(loggerPlanificacion, "Proceso: [%d] salío de SUSPENDIDO-LISTO.", pcbSaliente->pid);

    pthread_mutex_unlock(&mutexColaSuspendidoListo);

    return pcbSaliente;
}

/*Monitores*/
void incrementar_cantidad_procesos_memoria()
{
    pthread_mutex_lock(&mutexcantidadProcesosMemoria);
    cantidadProcesosEnMemoria++;
    pthread_mutex_unlock(&mutexcantidadProcesosMemoria);
}
void decrementar_cantidad_procesos_memoria()
{
    pthread_mutex_lock(&mutexcantidadProcesosMemoria);
    cantidadProcesosEnMemoria--;
    pthread_mutex_unlock(&mutexcantidadProcesosMemoria);

    sem_post(&despertarPlanificadorLargoPlazo);
}
int cantidad_procesos_memoria()
{
    int cantidad;
    pthread_mutex_lock(&mutexcantidadProcesosMemoria);
    cantidad = cantidadProcesosEnMemoria;
    pthread_mutex_unlock(&mutexcantidadProcesosMemoria);
    return cantidad;
}

int lectura_cola_mutex(t_queue *cola, pthread_mutex_t *semaforoMutex)
{
    int tamanio;
    pthread_mutex_lock(semaforoMutex);
    tamanio = queue_size(cola);
    pthread_mutex_unlock(semaforoMutex);
    return tamanio;
}

// Varios
void imprimir_colas()
{
    // Un forma bonita de impirmirlo.
    // system("clear");
    char *strcolaNuevos = leer_cola(colaNuevos);
    char *strcolaListos = leer_lista(colaListos);
    char *strcolaEjecutando = leer_cola(colaEjecutando);
    char *strcolaBloqueados = leer_cola(colaBloqueados);
    char *strcolaSuspendidoListo = leer_cola(colaSuspendidoListo);
    char *strcolaFinalizado = leer_cola(colaFinalizado);

    log_info(logger, "\
    \n\tCola nuevos: %s \
    \n\tCola listos: %s \
    \n\tCola ejecutando: %s \
    \n\tCola bloqueados: %s\
    \n\tCola suspended - ready: %s\
    \n\tCola terminados: %s",
             strcolaNuevos,
             strcolaListos, strcolaEjecutando, strcolaBloqueados, strcolaSuspendidoListo, strcolaFinalizado);

    free(strcolaNuevos);
    free(strcolaListos);
    free(strcolaEjecutando);
    free(strcolaBloqueados);
    free(strcolaSuspendidoListo);
    free(strcolaFinalizado);
}

char *leer_lista(t_list *cola)
{
    char *out = string_new();

    for (int i = 0; i < list_size(cola); i++)
    {
        Pcb *proceso_actual = list_get(cola, i);

        string_append(&out, "[");
        char *pid = string_itoa(proceso_actual->pid);
        string_append(&out, pid);
        string_append(&out, "]");
        free(pid);
    }
    return out;
}
char *leer_cola(t_queue *cola)
{
    char *out = string_new();

    for (int i = 0; i < queue_size(cola); i++)
    {
        Pcb *proceso_actual = queue_peek_at(cola, i);

        string_append(&out, "[");
        char *pid = string_itoa(proceso_actual->pid);
        string_append(&out, pid);
        string_append(&out, "]");
        free(pid);
    }
    return out;
}

float obtener_tiempo_de_trabajo_actual(Pcb *proceso)
{
    float alfa = KERNEL_CONFIG.ALFA;
    float estimacionAnterior = proceso->estimacionRafaga;
    int rafagaAnterior = proceso->tiempoRafagaRealAnterior * 1000;

    float resultado = alfa * rafagaAnterior + (1 - alfa) * estimacionAnterior;

    return resultado;
}

bool ordenar_segun_tiempo_de_trabajo(void *procesoA, void *procesoB)
{
    return obtener_tiempo_de_trabajo_actual((Pcb *)procesoA) < obtener_tiempo_de_trabajo_actual((Pcb *)procesoB);
}

// Funciones para liberar memoria

void liberar_estructuras()
{

    list_destroy(colaListos);

    queue_destroy(colaNuevos);

    queue_destroy(colaBloqueados);

    queue_destroy(colaEjecutando);

    queue_destroy(colaSuspendidoListo);

    queue_destroy(colaFinalizado);

    list_destroy(socketsConsola);
}

void imprimir_pcb(Pcb *proceso)
{
    log_info(logger, "PCB Nº %d", proceso->pid);
    log_info(logger, "Tamaño del Proceso: %d", proceso->tamanio);
    log_info(logger, "Estimación de Ráfaga: %.2f", proceso->estimacionRafaga);
    log_info(logger, "REAL de Ráfaga: %d", proceso->tiempoRafagaRealAnterior);
    log_info(logger, "Estado: %d", proceso->escenario->estado);
    log_info(logger, "Tiempo de Bloqueo de I/O: %d", proceso->escenario->tiempoBloqueadoIO);
    log_info(logger, "Tiempo de Inicio ejecucion (hace n segundos): %d", obtener_tiempo_actual() - proceso->tiempoInicioEjecucion);

    LineaInstruccion *lineaInstruccion;

    for (int i = 0; i < list_size(proceso->instrucciones); i++)
    {
        lineaInstruccion = list_get(proceso->instrucciones, i);
        log_info(logger, "Instrucción Nº %d: %s\t- Parámetro 1: %d\t- Parámetro 2: %d", i, lineaInstruccion->identificador, lineaInstruccion->parametros[0], lineaInstruccion->parametros[1]);
    }
}

void liberar_semaforos()
{
    pthread_mutex_destroy(&mutexNumeroProceso);
    pthread_mutex_destroy(&mutexProcesoListo);
    pthread_mutex_destroy(&mutexColaNuevos);
    pthread_mutex_destroy(&mutexColaListos);
    pthread_mutex_destroy(&mutexColaBloqueados);
    pthread_mutex_destroy(&mutexColaEjecutando);
    pthread_mutex_destroy(&mutexColaFinalizado);
    pthread_mutex_destroy(&mutexColaSuspendidoListo);
    pthread_mutex_destroy(&mutex_cola);
    pthread_mutex_destroy(&mutexcantidadProcesosMemoria);

    sem_destroy(&semaforoProcesoNuevo);
    sem_destroy(&semaforoProcesoListo);
    sem_destroy(&semaforoProcesoEjecutando);
    sem_destroy(&contadorBloqueados);
    sem_destroy(&analizarSuspension);
    sem_destroy(&suspensionFinalizada);
    sem_destroy(&despertarPlanificadorLargoPlazo);
    sem_destroy(&semaforoCantidadProcesosEjecutando);
}

void liberar_conexiones()
{
    apagar_servidor(socketKernel);
    log_info(logger, "Servidor Kernel finalizado.");

    liberar_conexion_con_servidor(socketDispatch);
    log_info(logger, "Saliendo del Servidor CPU-Dispatch.");

    liberar_conexion_con_servidor(socketInterrupt);
    log_info(logger, "Saliendo del Servidor CPU-Interrupt.");
}
