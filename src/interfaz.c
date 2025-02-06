#include "interfaz.h"
#include "sidebar.h"
#include <gtk/gtk.h>

void construir_interfaz() {
    GtkWidget *ventana, *contenedor, *sidebar, *contenido;
    GtkCssProvider *cssProvider;
    
    ventana = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(ventana), "GymAccess");
    gtk_window_set_default_size(GTK_WINDOW(ventana), 900, 600);
    g_signal_connect(ventana, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    // Contenedor principal
    contenedor = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(ventana), contenedor);

    // Sidebar
    sidebar = construir_sidebar();
    gtk_box_pack_start(GTK_BOX(contenedor), sidebar, FALSE, FALSE, 0);

    // Área de contenido
    contenido = gtk_label_new("Bienvenido a GymAccess");
    gtk_widget_set_name(contenido, "contenido");
    gtk_box_pack_start(GTK_BOX(contenedor), contenido, TRUE, TRUE, 0);

    // 🖌️ Cargar CSS
    cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, "styles/style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_widget_show_all(ventana);
}
