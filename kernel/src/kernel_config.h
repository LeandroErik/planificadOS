#ifndef KERNEL_CONFIG_H
#define KERNEL_CONFIG_H

#include <utils/protocolo.h>

typedef struct KernelConfig
{
  char *IP;
  char *IP_CPU;
  char *PUERTO_CPU_DISPATCH;
  char *PUERTO_CPU_INTERRUPT;
  char *PUERTO_KERNEL;
  char *ALGORITMO_PLANIFICACION;
  unsigned int ESTIMACION_INICIAL;
  float ALFA;
  unsigned int GRADO_MULTIPROGRAMACION;
  unsigned int TIEMPO_MAXIMO_BLOQUEADO;

} KernelConfig;

extern KernelConfig KERNEL_CONFIG;

/**
 * @brief Rellena la variable KERNEL_CONFIG con las configuraciones del kernel por archivo.
 *
 * @param config Configuración.
 */
void rellenar_configuracion_kernel(Config *config);

#endif