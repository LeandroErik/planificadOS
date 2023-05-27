#include <kernel_thread.h>

void esperar_consola(int socketKernel)
{
  while (true)
  {

    log_info(logger, "Esperando conexiones de Consola...");
    int socketConsola = esperar_cliente(socketKernel);

    if (socketConsola < 0)
    {
      log_warning(logger, "Consola desconectada.");

      return;
    }

    log_info(logger, "Conexión de Consola establecida.");

    list_add(socketsConsola, (void *)socketConsola);

    Hilo hiloConsola;
    pthread_create(&hiloConsola, NULL, (void *)manejar_paquete_consola, (void *)socketConsola);
    pthread_join(hiloConsola, NULL);
    pthread_detach(hiloConsola);
  }
}

void manejar_paquete_consola(int socketConsola)
{
  while (true)
  {
    char *mensaje;
    switch (obtener_codigo_operacion(socketConsola))
    {
    case LINEAS_CODIGO:
      log_info(logger, "Lineas de Código recibidas de Consola.");
      agregar_proceso_nuevo(generar_pcb(socketConsola));
      return;

      break;
    case MENSAJE:
      log_info(logger, "Mensaje recibido de Consola.");
      mensaje = obtener_mensaje_del_cliente(socketConsola);
      log_info(logger, "Mensaje: %s", mensaje);
      free(mensaje);

      break;
    case DESCONEXION:
      log_warning(logger, "Conexión de Consola terminada.");

      return;
    default:
      log_warning(logger, "Operacion desconocida.");

      break;
    }
  }
}

void enviar_interrupcion(int socketInterrupt)
{
  Paquete *paquete = crear_paquete(INTERRUPCION);

  enviar_paquete_a_servidor(paquete, socketInterrupt);

  log_info(logger, "[INTERRUPCION] Se envio Interrupción a CPU...");

  eliminar_paquete(paquete);
}
