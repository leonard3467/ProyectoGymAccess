#ifndef SIDEBAR_H
#define SIDEBAR_H

#include <gtk/gtk.h>  

GtkWidget* construir_sidebar();
GtkWidget* cargar_imagen_pequena(const char* ruta_imagen, int width);
#endif