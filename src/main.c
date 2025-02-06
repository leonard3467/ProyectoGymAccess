#include <gtk/gtk.h>
#include "interfaz.h"

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    construir_interfaz();  // Inicializa la UI
    gtk_main();
    return 0;
}
