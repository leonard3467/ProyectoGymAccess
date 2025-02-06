#include "sidebar.h"
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
    GtkWidget *hbox; // Caja horizontal para el título y el logo
    GtkWidget *imagen_registrar, *imagen_pagar, *imagen_buscar; // Imágenes para los botones

    // Crear la caja vertical principal (sidebar)
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_name(box, "sidebar");

    // Crear una caja horizontal para el título y el logo
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);  // Sin espaciado entre los widgets
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

    // Alineación del logo y título (centrado)
    gtk_widget_set_halign(logo, GTK_ALIGN_START);  // Alinea el logo a la izquierda
    gtk_widget_set_halign(titulo, GTK_ALIGN_START); // Alinea el título a la izquierda
    gtk_widget_set_valign(logo, GTK_ALIGN_CENTER);  // Centra el logo verticalmente
    gtk_widget_set_valign(titulo, GTK_ALIGN_CENTER); // Centra el título verticalmente

    // Agregar la caja horizontal (logo + título) al sidebar
    gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 10);

    // Cargar imágenes para los botones
    imagen_registrar = cargar_imagen_pequena("images/registrar.png", 20); // 20px de ancho
    imagen_pagar = cargar_imagen_pequena("images/pagar.png", 20); // 20px de ancho
    imagen_buscar = cargar_imagen_pequena("images/buscar.png", 20); // 20px de ancho

    // Crear el botón de Registrar con la imagen
    btn_registrar = gtk_button_new();
    GtkWidget *box_registrar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5); // Caja horizontal dentro del botón
    gtk_widget_set_name(btn_registrar, "boton_sidebar");
    gtk_box_pack_start(GTK_BOX(box_registrar), imagen_registrar, FALSE, FALSE, 5); // Imagen
    GtkWidget *label_registrar = gtk_label_new("Registrar Usuario");
    gtk_box_pack_start(GTK_BOX(box_registrar), label_registrar, FALSE, FALSE, 0); // Texto
    gtk_container_add(GTK_CONTAINER(btn_registrar), box_registrar); // Empacamos la caja dentro del botón
    gtk_box_pack_start(GTK_BOX(box), btn_registrar, FALSE, FALSE, 5);

    // Crear el botón de Pagar con la imagen
    btn_pagar = gtk_button_new();
    GtkWidget *box_pagar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5); // Caja horizontal dentro del botón
    gtk_widget_set_name(btn_pagar, "boton_sidebar");
    gtk_box_pack_start(GTK_BOX(box_pagar), imagen_pagar, FALSE, FALSE, 5); // Imagen
    GtkWidget *label_pagar = gtk_label_new("Pagar Plan");
    gtk_box_pack_start(GTK_BOX(box_pagar), label_pagar, FALSE, FALSE, 0); // Texto
    gtk_container_add(GTK_CONTAINER(btn_pagar), box_pagar); // Empacamos la caja dentro del botón
    gtk_box_pack_start(GTK_BOX(box), btn_pagar, FALSE, FALSE, 5);

    // Crear el botón de Buscar con la imagen
    btn_buscar = gtk_button_new();
    GtkWidget *box_buscar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5); // Caja horizontal dentro del botón
    gtk_widget_set_name(btn_buscar, "boton_sidebar");
    gtk_box_pack_start(GTK_BOX(box_buscar), imagen_buscar, FALSE, FALSE, 5); // Imagen
    GtkWidget *label_buscar = gtk_label_new("Buscar Usuario");
    gtk_box_pack_start(GTK_BOX(box_buscar), label_buscar, FALSE, FALSE, 0); // Texto
    gtk_container_add(GTK_CONTAINER(btn_buscar), box_buscar); // Empacamos la caja dentro del botón
    gtk_box_pack_start(GTK_BOX(box), btn_buscar, FALSE, FALSE, 5);

    return box;
}