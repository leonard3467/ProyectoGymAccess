#include <gtk/gtk.h>
#include "interfaz.h"
#include "clientes.h"

#define MAX_CLIENTES 100  // Límite de clientes a recuperar

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);

    // Recuperar clientes y mostrarlos en consola
    Cliente clientes[MAX_CLIENTES];
    int cantidad = recuperar_clientes(clientes, MAX_CLIENTES);

   
    construir_interfaz();
    gtk_main();

    return 0;
}
