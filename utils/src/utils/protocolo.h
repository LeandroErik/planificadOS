#ifndef PROTOCOLO_H
#define PROTOCOLO_H

#include <utils/socket/socket.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <semaphore.h>

typedef t_config Config;
typedef t_list Lista;
typedef t_log Logger;
typedef pthread_t Hilo;
typedef sem_t Semaforo;

typedef enum CodigoOperacion
{
  DESCONEXION = -1,
  MENSAJE,
  LINEAS_CODIGO,
  PCB,
  INTERRUPCION,
  PROCESO_NUEVO,
  SUSPENDER_PROCESO,
  FINALIZAR_PROCESO,
  PEDIDO_TABLA_SEGUNDO_NIVEL,
  PEDIDO_MARCO,
  ESCRIBIR_EN_MEMORIA,
  LEER_DE_MEMORIA,
  COPIAR_EN_MEMORIA,
  ESTRUCTURAS_MEMORIA
} CodigoOperacion;

typedef enum Estado
{
  EJECUTANDO,
  BLOQUEADO_IO,
  INTERRUPCION_EXTERNA,
  TERMINADO,
  SUSPENDIDO,
  LISTO,
  NUEVO
} Estado;

typedef enum Instruccion
{
  DESCONOCIDA = -1,
  NOOP,
  IO,
  READ,
  COPY,
  WRITE,
  EXIT
} Instruccion;
typedef struct Escenario
{
  Estado estado;
  int tiempoBloqueadoIO;
} Escenario;

typedef struct Pcb
{
  unsigned int pid;
  unsigned int tamanio;
  unsigned int contadorPrograma;
  unsigned int tablaPaginas;
  float estimacionRafaga;
  unsigned int tiempoInicioEjecucion;
  unsigned int tiempoRafagaRealAnterior;
  Escenario *escenario;
  Lista *instrucciones;
  Semaforo confirmacionSuspencion;
  bool vieneDeSuspension;
} Pcb;

typedef struct LineaInstruccion
{
  char *identificador;
  int parametros[2];
} LineaInstruccion;

typedef struct Buffer
{
  int tamanio;
  void *stream;
} Buffer;

typedef struct Paquete
{
  CodigoOperacion codigoOperacion;
  Buffer *buffer;
} Paquete;

// ------- Funciones de Cliente y Servidor  -------

/**
 * @brief Elimina un paquete y su buffer.
 *
 * @param paquete Paquete a eliminar.
 */
void eliminar_paquete(Paquete *paquete);

/**
 * @brief Libera una línea de instrucción.
 *
 * @param lineaInstruccion Línea de instrucción.
 */
void eliminar_linea_instruccion(LineaInstruccion *lineaInstruccion);

/**
 * @brief Elimina un PCB.
 *
 * @param pcb PCB.
 */
void eliminar_pcb(Pcb *pcb);

// ------- Funciones de Cliente -------

/**
 * @brief Inicializa un buffer en un paquete.
 *
 * @param paquete Paquete.
 */
void inicializar_buffer(Paquete *paquete);

/**
 * @brief Crea un paquete pasando un código de operación.
 *
 * @param codigoOperacion Código de operación.
 *
 * @return Paquete.
 */
Paquete *crear_paquete(CodigoOperacion codigoOperacion);

/**
 * @brief Agrega un elemento a un paquete.
 *
 * @param paquete Paquete.
 * @param valor Valor a agregar.
 * @param tamanio Tamaño del paquete.
 */
void agregar_a_paquete(Paquete *paquete, void *valor, int tamanio);

/**
 * @brief Serializa un paquete.
 *
 * @param paquete Paquete a serializar.
 * @param bytes Bytes a alocar para el paquete serializado.
 *
 * @return Paquete serializado.
 */
void *serializar_paquete(Paquete *paquete, int bytes);

/**
 * @brief Envía un paquete al servidor conectado.
 *
 * @param paquete Paquete a enviar.
 * @param socketCliente Socket del cliente.
 */
void enviar_paquete_a_servidor(Paquete *paquete, int socketCliente);

/**
 * @brief Envía un mensaje al servidor conectado.
 *
 * @param mensaje Mensaje a enviar.
 * @param socketCliente Socket del cliente.
 */
void enviar_mensaje_a_servidor(char *mensaje, int socketCliente);

// ------- Funciones de Servidor -------

/**
 * @brief Obtiene el código de operación del paquete recibido.
 *
 * @param socketCliente Socket del cliente.
 *
 * @return Código de operación.
 */
CodigoOperacion obtener_codigo_operacion(int socketCliente);

/**
 * @brief Obtiene el buffer enviado por el cliente.
 *
 * @param tamanio Tamaño del buffer.
 * @param socketCliente Socket del cliente.
 *
 * @return Buffer.
 */
void *obtener_buffer_del_cliente(int *tamanio, int socketCliente);

/**
 * @brief Obtiene el mensaje enviado por el cliente.
 *
 * @param socketCliente Socket del cliente.
 *
 * @return Mensaje.
 */
char *obtener_mensaje_del_cliente(int socketCliente);

/**
 * @brief Obtiene el paquete como una lista.
 *
 * @param socketCliente Socket del cliente.
 *
 * @return Contenido del paquete como una lista.
 */
Lista *obtener_paquete_como_lista(int socketCliente);

/**
 * @brief Serializa la lista de instrucciones para ser enviada.
 *
 * @param paquete Paquete a rellenar.
 * @param listaInstrucciones Lista de instrucciones.
 * @param tamanioProceso Tamaño del proceso.
 */
void serializar_lista_de_instrucciones(Paquete *paquete, Lista *listaInstrucciones, int tamanioProceso);

/**
 * @brief Deserializa las líneas de instrucción dadas por una lista para agregar esas instrucciones
 * a una lista plana con las propiedades de cada línea de instrucción y un índice para comenzar.
 *
 * @param listaInstrucciones Lista de instrucciones.
 * @param listaPlana Lista plana.
 * @param indiceTamanio Índice donde se aloca el tamaño de la lista.
 * @param indiceLista Índice donde se inicia la lista plana de instrucciones.
 */
void deserializar_lista_de_instrucciones(Lista *listaInstrucciones, Lista *listaPlana, int indiceTamanio, int indiceLista);

/**
 * @brief Serializa el PCB para ser enviado.
 *
 * @param paquete Paquete a rellenar.
 * @param pcb PCB.
 */
void serializar_pcb(Paquete *paquete, Pcb *pcb);

/**
 * @brief Deserializa el PCB enviado por el cliente.
 *
 * @param socketCliente Socket del cliente.
 *
 * @return PCB.
 */
Pcb *deserializar_pcb(int socketCliente);

/**
 * @brief Envía un paquete al cliente conectado.
 *
 * @param paquete Paquete a enviar.
 * @param socketCliente Socket del cliente.
 */
void enviar_paquete_a_cliente(Paquete *paquete, int socketCliente);

/**
 * @brief Envia un mensaje al cliente conectado
 *
 * @param mensaje Mensaje a enviar.
 * @param socketCliente Socket del cliente.
 */
void enviar_mensaje_a_cliente(char *mensaje, int socketCliente);

char *obtener_mensaje_del_servidor(int socketServidor);

/**
* @brief Devuelve el tiempo actual

  @return Tiempo actual.
*/
int obtener_tiempo_actual();

void liberar_instruccion(LineaInstruccion *);
void liberar_pcb(Pcb *);

#endif