#include "interfaz.h"
#include "sidebar.h"
#include "clientes.h"
#include "pagar.h"
#include <gtk/gtk.h>

/* VARIABLES GLOBALES/ESTÁTICAS */
static GtkWidget *ventana_principal = NULL;
static GtkWidget *stack             = NULL;

static GtkWidget *widget_empty    = NULL;
static GtkWidget *widget_registro = NULL;
static GtkWidget *widget_pagar    = NULL;
static GtkWidget *widget_buscar   = NULL;
static GtkWidget *widget_acceso   = NULL;


GtkWidget* crear_pantalla_vacia(void){
    GtkWidget *lbl = gtk_label_new("");
    return lbl;
}

// Crea la pantalla de Registro (usa tu formulario antiguo)
GtkWidget* crear_pantalla_registro(void){
    GtkWidget *formulario = crear_formulario_registro(obtener_ventana_principal());
    return formulario;
}

#include "pagar.h"  // Incluir el header de pagar

GtkWidget* crear_pantalla_pagar(void) {
    return generar_pago(obtener_ventana_principal()); 
}


// Crea la pantalla de Buscar
GtkWidget* crear_pantalla_buscar(void){
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *lbl = gtk_label_new("Pantalla de búsqueda (Próximamente)...");
    gtk_box_pack_start(GTK_BOX(box), lbl, TRUE, TRUE, 0);
    return box;
}

// Crea la pantalla de Acceso
GtkWidget* crear_pantalla_acceso(void){
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    GtkWidget *lbl = gtk_label_new("Pantalla de control de acceso (Próximamente)...");
    gtk_box_pack_start(GTK_BOX(box), lbl, TRUE, TRUE, 0);
    return box;
}

void construir_interfaz()
{
    GtkWidget *contenedor, *sidebar;
    GtkCssProvider *cssProvider;

    // Crear la ventana principal
    ventana_principal = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(ventana_principal), "GymAccess");
    gtk_window_set_default_size(GTK_WINDOW(ventana_principal), 1000, 800);
    g_signal_connect(ventana_principal, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Contenedor horizontal para: [Sidebar] | [Stack]
    contenedor = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(ventana_principal), contenedor);

    // Construir la barra lateral
    sidebar = construir_sidebar();
    gtk_box_pack_start(GTK_BOX(contenedor), sidebar, FALSE, FALSE, 0);

    // Crear el stack y configurarlo
    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_CROSSFADE);
    gtk_stack_set_transition_duration(GTK_STACK(stack), 300);

    // Crea todas las pantallas (una sola vez)
    widget_empty    = crear_pantalla_vacia();
    widget_registro = crear_pantalla_registro();
    widget_pagar    = crear_pantalla_pagar();
    widget_buscar   = crear_pantalla_buscar();
    widget_acceso   = crear_pantalla_acceso();

    // Añadirlas al stack con un "nombre"
    gtk_stack_add_named(GTK_STACK(stack), widget_empty,    "empty");
    gtk_stack_add_named(GTK_STACK(stack), widget_registro, "registro");
    gtk_stack_add_named(GTK_STACK(stack), widget_pagar,    "pagar");
    gtk_stack_add_named(GTK_STACK(stack), widget_buscar,   "buscar");
    gtk_stack_add_named(GTK_STACK(stack), widget_acceso,   "acceso");

    // Por defecto, mostrar la página "empty" (pantalla vacía)
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "empty");

    // Empaquetar el stack en el contenedor (al lado derecho)
    gtk_box_pack_start(GTK_BOX(contenedor), stack, TRUE, TRUE, 0);

    // (Opcional) Cargar CSS
    cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, "styles/style.css", NULL);
    gtk_style_context_add_provider_for_screen(
        gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(cssProvider),
        GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
    );

    // Mostrar la ventana principal
    gtk_widget_show_all(ventana_principal);
}
void mostrar_pantalla_registro(GtkWidget *button, gpointer data)
{

    gtk_stack_set_visible_child_name(GTK_STACK(stack), "registro");
}
void mostrar_pantalla_pagos(GtkWidget *button, gpointer data)
{
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "pagar");
}
void mostrar_pantalla_buscar(GtkWidget *button, gpointer data)
{
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "buscar");
}
void mostrar_pantalla_acceso(GtkWidget *button, gpointer data)
{
    gtk_stack_set_visible_child_name(GTK_STACK(stack), "acceso");
}

GtkWidget* obtener_ventana_principal()
{
    return ventana_principal;
}