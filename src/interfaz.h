#ifndef INTERFAZ_H
#define INTERFAZ_H

#include <gtk/gtk.h>


/** Construye la ventana principal y la interfaz base */

GtkWidget* crear_pantalla_vacia(void);

GtkWidget* crear_pantalla_pagar(void);
GtkWidget* crear_pantalla_buscar(void);
GtkWidget* crear_pantalla_acceso(void);
GtkWidget* crear_pantalla_registro(void);

void construir_interfaz();


void mostrar_pantalla_registro(GtkWidget *button, gpointer data);
void mostrar_pantalla_pagos(GtkWidget *button, gpointer data);
void mostrar_pantalla_buscar(GtkWidget *button, gpointer data);
void mostrar_pantalla_acceso(GtkWidget *button, gpointer data);
/** Devuelve la ventana principal (GtkWindow) para usarla en otros lugares */
GtkWidget* obtener_ventana_principal();

#endif
