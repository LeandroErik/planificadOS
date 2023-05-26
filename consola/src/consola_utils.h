#ifndef CONSOLA_UTILS_H
#define CONSOLA_UTILS_H

#include <utils/socket/cliente.h>
// #include <consola_config.h>
#include <commons/string.h>
#include <commons/txt.h>

typedef struct ConsolaConfig
{
    char *IP_KERNEL;
    char *PUERTO_KERNEL;
} ConsolaConfig;

extern ConsolaConfig CONSOLA_CONFIG;

/**
 * @brief Rellena la variable CONSOLE_CONFIG con las configuraciones de la consola por archivo.
 *
 * @param config Configuración.
 */
void rellenar_configuracion_consola(Config *config);
//----------

/**
 * @brief Lee una linea de un pseudocódigo.
 *
 * @param archivo Archivo que apunta al pseudocódigo.
 *
 * @return Línea leída.
 */
char *leer_linea(FILE *archivo);

/**
 * @brief Crea una conexión con el servidor Kernel.
 *
 * @return Socket del cliente.
 */
int conectar_con_kernel();

/**
 * @brief Elimina el salto de línea (\n) de una línea.
 *
 * @param linea Línea.
 */
void eliminar_salto_linea(char *linea);

/**
 * @brief Devuelve la cantidad de veces que un carácter se repite en una cadena.
 *
 * @param string String.
 * @param caracter Carácter.
 *
 * @return Veces que aparece el carácter en la cadena.
 */
int veces(char *string, char caracter);

/**
 * @brief Devuelve la cantidad de tokens que tiene una línea de pseudocódigo.
 *
 * @param string Línea de pseudocódigo.
 *
 * @return Cantidad de tokens que tiene la línea.
 */
int cantidad_de_tokens(char *string);

/**
 * @brief Rellena los parámetros de la instrucción en caso de que no haya suficientes tokens. (Deben ser 3)
 *
 * @param tokens Tokens a rellenar.
 * @param cantidadTokens Cantidad de tokens que tiene la línea.
 */
void rellenar_parametros_de_instruccion(char **tokens, int cantidadTokens);

/**
 * @brief Obtiene los tokens de una línea de pseudocódigo. (Siempre tienen que ser 3)
 *
 * @param string Línea de pseudocódigo.
 *
 * @return Tokens de la línea.
 */
char **obtener_tokens(char *string);

/**
 * @brief Asigna los tokens a una LineaInstruccion.
 *
 * @param lineaInstruccion Línea de instrucción.
 * @param tokens Tokens.
 */
void asignar_tokens_a_linea_instruccion(LineaInstruccion *lineaInstruccion, char **tokens);

/**
 * @brief Agrega una línea de instrucción a una lista.
 *        Si la instrucción es un NO_OP, setea su params[0] a -1
 *        y agrega la línea n veces (n = params[0]).
 *
 *        ```c
 *        Ejemplo: NO_OP 3.
 *
 *        InstrucciónLinea* noOp  -> Instrucción = NO_OP
 *                                -> Params[0] = -1
 *                                -> Params[1] = -1
 *
 *        Lista: [noOp, noOp, noOp]
 *        ```
 *
 * @param lineaInstruccion Línea de instrucción.
 * @param lista Lista.
 */
void agregar_linea_de_instruccion_a_lista(LineaInstruccion *lineaInstruccion, Lista *lista);

/**
 * @brief Devuelve una línea de instrucción leída del archivo.
 *
 * @param archivo Archivo.
 *
 * @return Línea de instrucción.
 */
LineaInstruccion *leer_linea_de_instruccion(FILE *archivo);

/**
 * @brief Rellena una lista de instrucciones leyendo un pseudocódigo.
 *
 * @param lista Lista de instrucciones.
 * @param file Pseudocódigo.
 */
void rellenar_lista_de_instrucciones(Lista *lista, FILE *file);

/**
 * @brief Termina un módulo de Consola.
 *
 * @param socket Socket.
 * @param config Configuración.
 * @param logger Logger.
 * @param lista Lista.
 * @param archivo Archivo.
 * @param paquete Paquete.
 */
void terminar_consola(int socket, Config *config, Logger *logger, Lista *lista, FILE *archivo, Paquete *paquete);

#endif