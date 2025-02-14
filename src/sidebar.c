#include "sidebar.h"
#include "interfaz.h"   
#include <gtk/gtk.h>

GtkWidget* cargar_imagen_pequena(const char* ruta_imagen, int width) {
    GdkPixbuf *pixbuf;
    GtkWidget *imagen;
    int height;

    // Cargar la imagen desde archivo
    pixbuf = gdk_pixbuf_new_from_file(ruta_imagen, NULL);
    if (pixbuf == NULL) {
        g_print("No se pudo cargar la imagen.\n");
        return NULL;  // En caso de error al cargar la imagen
    }

    // Obtener las dimensiones originales de la imagen
    height = gdk_pixbuf_get_height(pixbuf);

    // Calcular la nueva altura manteniendo la proporción
    int new_height = (height * width) / gdk_pixbuf_get_width(pixbuf); // Altura proporcional

    // Redimensionar la imagen
    pixbuf = gdk_pixbuf_scale_simple(pixbuf, width, new_height, GDK_INTERP_BILINEAR);

    // Crear una GtkImage a partir del pixbuf redimensionado
    imagen = gtk_image_new_from_pixbuf(pixbuf);

    // Liberar pixbuf para evitar fugas de memoria
    g_object_unref(pixbuf);

    return imagen;
}

GtkWidget* construir_sidebar() {
    GtkWidget *box, *titulo, *btn_registrar, *btn_pagar, *btn_buscar, *logo;
    GtkWidget *hbox;
    GtkWidget *imagen_registrar, *imagen_pagar, *imagen_buscar, *imagen_acceso;

    // Crear la caja vertical principal (sidebar)
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_name(box, "sidebar");

    // Crear una caja horizontal para el título y el logo
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_name(hbox, "hbox-titulo-logo");

    // Cargar el logo
    logo = cargar_imagen_pequena("images/logo.png", 75);
    gtk_widget_set_name(logo, "logo");

    // Empacar el logo en la caja horizontal
    gtk_box_pack_start(GTK_BOX(hbox), logo, FALSE, FALSE, 0);

    // Crear y añadir el título
    titulo = gtk_label_new("GymAccess");
    gtk_widget_set_name(titulo, "titulo");
    gtk_box_pack_start(GTK_BOX(hbox), titulo, FALSE, FALSE, 0);

    // Agregar la caja horizontal (logo + título) al sidebar
    gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 10);

    // Cargar imágenes para los botones
    imagen_registrar = cargar_imagen_pequena("images/registrar.png", 20);
    imagen_pagar = cargar_imagen_pequena("images/pagar.png", 20);
    imagen_buscar = cargar_imagen_pequena("images/buscar.png", 20);
    imagen_acceso = cargar_imagen_pequena("images/acceso.png", 20);

    // Crear el botón de Acceso con la imagen
    GtkWidget *btn_acceso = gtk_button_new();
    GtkWidget *box_acceso = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(btn_acceso, "boton_sidebar");
    gtk_box_pack_start(GTK_BOX(box_acceso), imagen_acceso, FALSE, FALSE, 5);
    GtkWidget *label_acceso = gtk_label_new("Acceso");
    gtk_box_pack_start(GTK_BOX(box_acceso), label_acceso, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(btn_acceso), box_acceso);

    g_signal_connect(btn_acceso, "clicked", G_CALLBACK(mostrar_pantalla_acceso), NULL);

    gtk_box_pack_start(GTK_BOX(box), btn_acceso, FALSE, FALSE, 5);
    
 


    // Crear el botón de Registrar con la imagen
    btn_registrar = gtk_button_new();
    GtkWidget *box_registrar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(btn_registrar, "boton_sidebar");
    gtk_box_pack_start(GTK_BOX(box_registrar), imagen_registrar, FALSE, FALSE, 5);
    GtkWidget *label_registrar = gtk_label_new("Registrar Usuario");
    gtk_box_pack_start(GTK_BOX(box_registrar), label_registrar, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(btn_registrar), box_registrar);

    g_signal_connect(btn_registrar, "clicked", G_CALLBACK(mostrar_pantalla_registro), NULL);

    gtk_box_pack_start(GTK_BOX(box), btn_registrar, FALSE, FALSE, 5);



    // Crear el botón de Pagar con la imagen
    btn_pagar = gtk_button_new();
    GtkWidget *box_pagar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(btn_pagar, "boton_sidebar");
    gtk_box_pack_start(GTK_BOX(box_pagar), imagen_pagar, FALSE, FALSE, 5);
    GtkWidget *label_pagar = gtk_label_new("Pagar Plan");
    gtk_box_pack_start(GTK_BOX(box_pagar), label_pagar, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(btn_pagar), box_pagar);

    g_signal_connect(btn_pagar, "clicked", G_CALLBACK(mostrar_pantalla_pagos), NULL);

    gtk_box_pack_start(GTK_BOX(box), btn_pagar, FALSE, FALSE, 5);

    // Crear el botón de Buscar con la imagen
    btn_buscar = gtk_button_new();
    GtkWidget *box_buscar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_name(btn_buscar, "boton_sidebar");
    gtk_box_pack_start(GTK_BOX(box_buscar), imagen_buscar, FALSE, FALSE, 5);
    GtkWidget *label_buscar = gtk_label_new("Buscar Usuario");
    gtk_box_pack_start(GTK_BOX(box_buscar), label_buscar, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(btn_buscar), box_buscar);

    g_signal_connect(btn_buscar, "clicked", G_CALLBACK(mostrar_pantalla_buscar), NULL);
    
    gtk_box_pack_start(GTK_BOX(box), btn_buscar, FALSE, FALSE, 5);


    return box;
}
