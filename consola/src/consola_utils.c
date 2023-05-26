#include <consola_utils.h>

ConsolaConfig CONSOLA_CONFIG;

void rellenar_configuracion_consola(Config *config)
{
  CONSOLA_CONFIG.IP_KERNEL = config_get_string_value(config, "IP_KERNEL");
  CONSOLA_CONFIG.PUERTO_KERNEL = config_get_string_value(config, "PUERTO_KERNEL");
}
//------------------

char *leer_linea(FILE *archivo)
{
  char *linea = string_new();
  int tamanioBuffer = 0;

  getline(&linea, (size_t *restrict)&tamanioBuffer, archivo);
  eliminar_salto_linea(linea);

  return linea;
}

int conectar_con_kernel()
{
  return crear_conexion_con_servidor(CONSOLA_CONFIG.IP_KERNEL, CONSOLA_CONFIG.PUERTO_KERNEL);
}

void eliminar_salto_linea(char *linea)
{
  if (linea[strlen(linea) - 1] == '\n')
    linea[strlen(linea) - 1] = '\0';
}

int veces(char *string, char caracter)
{
  int veces = 0;

  for (int i = 0; i < string_length(string); i++)
    if (string[i] == caracter)
      veces++;

  return veces;
}

int cantidad_de_tokens(char *string)
{
  return veces(string, ' ') + 1;
}

void rellenar_parametros_de_instruccion(char **tokens, int cantidadTokens)
{
  if (cantidadTokens < 2)
    tokens[1] = "-1";
  if (cantidadTokens < 3)
    tokens[2] = "-1";
}

char **obtener_tokens(char *string)
{
  short cantidadTokens = cantidad_de_tokens(string);
  char **tokens = string_n_split(string, cantidadTokens, " ");

  rellenar_parametros_de_instruccion(tokens, cantidadTokens);

  return tokens;
}

void asignar_tokens_a_linea_instruccion(LineaInstruccion *lineaInstruccion, char **tokens)
{
  lineaInstruccion->identificador = strdup(tokens[0]);
  lineaInstruccion->parametros[0] = atoi(tokens[1]);
  lineaInstruccion->parametros[1] = atoi(tokens[2]);

  free(*tokens);
}

void agregar_linea_de_instruccion_a_lista(LineaInstruccion *lineaInstruccion, Lista *lista)
{
  if (!strcmp(lineaInstruccion->identificador, "NO_OP"))
  {
    for (int i = 0; i < lineaInstruccion->parametros[0]; i++)
      list_add(lista, lineaInstruccion);

    lineaInstruccion->parametros[0] = -1;
  }
  else
    list_add(lista, lineaInstruccion);
}

LineaInstruccion *leer_linea_de_instruccion(FILE *archivo)
{
  char *linea = leer_linea(archivo);
  char **tokens = obtener_tokens(linea);

  LineaInstruccion *lineaInstruccion = malloc(sizeof(LineaInstruccion));
  asignar_tokens_a_linea_instruccion(lineaInstruccion, tokens);

  free(linea);

  return lineaInstruccion;
}

void rellenar_lista_de_instrucciones(Lista *lista, FILE *archivo)
{
  while (!feof(archivo))
  {
    LineaInstruccion *lineaInstruccion = leer_linea_de_instruccion(archivo);
    agregar_linea_de_instruccion_a_lista(lineaInstruccion, lista);
  }
}

void terminar_consola(int socket, Config *config, Logger *logger, Lista *lista, FILE *archivo, Paquete *paquete)
{
  terminar_programa(socket, config, logger);
  eliminar_paquete(paquete);

  if (lista != NULL)
    list_destroy(lista);

  if (archivo != NULL)
    txt_close_file(archivo);
}