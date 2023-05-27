#include <kernel_config.h>

void rellenar_configuracion_kernel(Config *config)
{
  KERNEL_CONFIG.IP = config_get_string_value(config, "IP");
  KERNEL_CONFIG.IP_CPU = config_get_string_value(config, "IP_CPU");
  KERNEL_CONFIG.IP_MEMORIA = config_get_string_value(config, "IP_MEMORIA");
  KERNEL_CONFIG.PUERTO_MEMORIA = config_get_string_value(config, "PUERTO_MEMORIA");
  KERNEL_CONFIG.PUERTO_CPU_DISPATCH = config_get_string_value(config, "PUERTO_CPU_DISPATCH");
  KERNEL_CONFIG.PUERTO_CPU_INTERRUPT = config_get_string_value(config, "PUERTO_CPU_INTERRUPT");
  KERNEL_CONFIG.PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
  KERNEL_CONFIG.ALGORITMO_PLANIFICACION = config_get_string_value(config, "ALGORITMO_PLANIFICACION");
  KERNEL_CONFIG.ESTIMACION_INICIAL = config_get_int_value(config, "ESTIMACION_INICIAL");
  KERNEL_CONFIG.ALFA = config_get_double_value(config, "ALFA");
  KERNEL_CONFIG.GRADO_MULTIPROGRAMACION = config_get_int_value(config, "GRADO_MULTIPROGRAMACION");
  KERNEL_CONFIG.TIEMPO_MAXIMO_BLOQUEADO = config_get_int_value(config, "TIEMPO_MAXIMO_BLOQUEADO");
}