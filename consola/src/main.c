#include <consola_utils.h>
#include <commons/log.h>

int main(int argc, char *argv[])
{
  Config *config = config_create("consola.config");
  Logger *logger = log_create("consola.log", "Consola", true, LOG_LEVEL_INFO);

  if (argc < 3)
  {
    log_error(logger, "Los argumentos son dos.");
    return EXIT_FAILURE;
  }

  rellenar_configuracion_consola(config);

  char *direccionArchivo = argv[1];
  int tamanioProceso = atoi(argv[2]);

  log_info(logger, "Conectando con Servidor Kernel...");
  int socketKernel = conectar_con_kernel();

  if (socketKernel < 0)
  {
    log_error(logger, "Conexión rechazada. El Servidor Kernel no está disponible.");
    return EXIT_FAILURE;
  }

  log_info(logger, "Conexión aceptada. Iniciando cliente...");

  FILE *archivoInstrucciones = fopen(direccionArchivo, "r");

  if (archivoInstrucciones == NULL)
  {
    log_error(logger, "No se pudo abrir el archivo.");
    return EXIT_FAILURE;
  }

  Lista *listaInstrucciones = list_create();
  rellenar_lista_de_instrucciones(listaInstrucciones, archivoInstrucciones);

  Paquete *paquete = crear_paquete(LINEAS_CODIGO);

  serializar_lista_de_instrucciones(paquete, listaInstrucciones, tamanioProceso);
  enviar_paquete_a_servidor(paquete, socketKernel);

  log_info(logger, "Enviando Lista de Instrucciones al Servidor Kernel...");

  obtener_codigo_operacion(socketKernel);

  log_warning(logger, "Saliendo del Servidor Kernel...");

  terminar_consola(socketKernel, config, logger, listaInstrucciones, archivoInstrucciones, paquete);

  return EXIT_SUCCESS;
}