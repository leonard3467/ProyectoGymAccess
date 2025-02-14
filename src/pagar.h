#ifndef PAGAR_H
#define PAGAR_H

#include <gtk/gtk.h>
#include "clientes.h"

char* normalizar_texto(const char *texto);
// 📌 Función para buscar un cliente en clientes.txt
gboolean buscar_cliente(const char *busqueda, Cliente *cliente_encontrado);

// 📌 Función para actualizar la etiqueta con el resultado de la búsqueda
void actualizar_busqueda(GtkWidget *widget, gpointer data);

// 📌 Función para crear la barra de búsqueda
GtkWidget* crear_busqueda();

// 📌 Función para generar la pantalla de pago
GtkWidget* generar_pago(GtkWidget *parent_window);

void actualizar_ficha_cliente(Cliente *cliente);
void cambiar_plan_callback(GtkWidget *widget, gpointer data);
void procederPago(GtkWidget *widget, gpointer data);
void activar_combo_plan(GtkWidget *widget, gpointer data);
void actualizar_fecha_fin_label(const gchar *fecha_inicio, const gchar *plan, GtkWidget *label_fin);

#endif // PAGAR_H
