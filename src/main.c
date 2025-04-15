#include <gtk/gtk.h>
#include "interfaz.h"
#include "clientes.h"
#include "db_setup.h" 
#include "asistencia.h"
#include <process.h>  // Para _beginthread

#define MAX_CLIENTES 100  // Límite de clientes a recuperar


int main(int argc, char *argv[]) {
    //para hacer que se genera la base de datos apenas se ejecute el programa esto:
    if (inicializar_base_datos() != 0) {
        g_print("Error al inicializar la base de datos.\n");
        return 1;
    }
    gtk_init(&argc, &argv);

    _beginthread((void(*)(void*))iniciar_lector_serial, 0, NULL);
   
    construir_interfaz();
    gtk_main();

    return 0;
}