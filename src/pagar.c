#include "pagar.h"
#include "clientes.h"
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <sqlite3.h>  

// 📌 Variables globales para la búsqueda
static GtkWidget *entry_busqueda, *label_resultado;
// 📌 Variable global para la ficha del cliente
static GtkWidget *box_ficha_cliente = NULL;
// 📌 Función para normalizar texto (sin acentos y en minúsculas)
char* normalizar_texto(const char *texto) {
    if (!texto) return NULL;

    // 📌 Convertir a minúsculas y normalizar a la forma NFD (descompuesta)
    gchar *texto_normalizado = g_utf8_normalize(texto, -1, G_NORMALIZE_NFD);
    if (!texto_normalizado) return NULL;

    GString *resultado = g_string_new(NULL);

    for (const gchar *p = texto_normalizado; *p; p = g_utf8_next_char(p)) {
        gunichar c = g_utf8_get_char(p);
        
        // 📌 Si es un carácter base (sin tilde), lo agregamos
        if (!g_unichar_combining_class(c)) {
            c = g_unichar_tolower(c); // Convertir a minúsculas
            g_string_append_unichar(resultado, c);
        }
    }

    g_free(texto_normalizado);
    return g_string_free(resultado, FALSE);
}

gboolean buscar_cliente(const char *busqueda, Cliente *cliente_encontrado) {
    sqlite3 *db;
    int rc = sqlite3_open("clientes.db", &db);
    if (rc != SQLITE_OK) {
        g_print("Error al abrir la BD: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return FALSE;
    }

    // Nueva consulta para buscar en ambas tablas clientes y pago_cliente
    const char *sql = 
        "SELECT c.id, c.nombre, c.telefono, c.correo, c.tipo_plan, c.fecha_inicio, c.fecha_fin, "
        "p.saldo_pendiente, p.meses_adelantados, p.años_adelantados "
        "FROM clientes c "
        "JOIN pago_cliente p ON c.id = p.cliente_id "
        "WHERE c.id = ? OR c.nombre LIKE ? OR c.telefono = ? OR c.correo = ? LIMIT 1;";

    sqlite3_stmt *stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    if (rc != SQLITE_OK) {
        g_print("Error al preparar consulta: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return FALSE;
    }

    // Intentamos buscar por ID (si la cadena es numérica) o por coincidencia parcial en el nombre
    int id = atoi(busqueda);
    sqlite3_bind_int(stmt, 1, id);
    // Usamos LIKE para buscar por nombre con comodín
    char busqueda_like[100];
    snprintf(busqueda_like, sizeof(busqueda_like), "%%%s%%", busqueda);
    sqlite3_bind_text(stmt, 2, busqueda_like, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, busqueda, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, busqueda, -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        cliente_encontrado->id = sqlite3_column_int(stmt, 0);
        strncpy(cliente_encontrado->nombre, (const char *)sqlite3_column_text(stmt, 1), sizeof(cliente_encontrado->nombre));
        strncpy(cliente_encontrado->telefono, (const char *)sqlite3_column_text(stmt, 2), sizeof(cliente_encontrado->telefono));
        strncpy(cliente_encontrado->correo, (const char *)sqlite3_column_text(stmt, 3), sizeof(cliente_encontrado->correo));
        strncpy(cliente_encontrado->tipo_plan, (const char *)sqlite3_column_text(stmt, 4), sizeof(cliente_encontrado->tipo_plan));
        strncpy(cliente_encontrado->fecha_inicio, (const char *)sqlite3_column_text(stmt, 5), sizeof(cliente_encontrado->fecha_inicio));
        strncpy(cliente_encontrado->fecha_fin, (const char *)sqlite3_column_text(stmt, 6), sizeof(cliente_encontrado->fecha_fin));
        cliente_encontrado->saldo_pendiente = sqlite3_column_int(stmt, 7);
        cliente_encontrado->meses_adelantados = sqlite3_column_int(stmt, 8);
        cliente_encontrado->años_adelantados = sqlite3_column_int(stmt, 9);

        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return TRUE;
    } else {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return FALSE;
    }
}

void actualizar_busqueda(GtkWidget *widget, gpointer data) {
    const char *busqueda = gtk_entry_get_text(GTK_ENTRY(entry_busqueda));
    
    if (strlen(busqueda) == 0) {
        gtk_label_set_text(GTK_LABEL(label_resultado), "Ingrese un dato válido...");
        gtk_widget_set_visible(box_ficha_cliente, FALSE); // 🔹 Ocultar ficha si no hay texto
        return;
    }

    Cliente cliente;
    if (buscar_cliente(busqueda, &cliente)) {
        // 📌 Llamar a la función para actualizar la ficha del cliente
        actualizar_ficha_cliente(&cliente);
    } else {
        gtk_label_set_text(GTK_LABEL(label_resultado), "❌ Cliente no encontrado");
        gtk_widget_set_visible(box_ficha_cliente, FALSE); // 🔹 Ocultar ficha si no hay resultado
    }
}
GtkWidget* crear_busqueda() {
    GtkWidget *box, *label_busqueda, *entry_box, *icono_busqueda;
    
    // Contenedor vertical para centrar la búsqueda
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(box, "busqueda-container");

    // Label de instrucción
    label_busqueda = gtk_label_new("Ingrese ID, Nombre, Correo o Teléfono:");
    gtk_widget_set_name(label_busqueda, "label-busqueda");

    // Contenedor horizontal para el campo de búsqueda y el ícono
    entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_name(entry_box, "busqueda-box");

    // Entrada de texto para buscar
    entry_busqueda = gtk_entry_new();
    gtk_widget_set_name(entry_busqueda, "busqueda-entry");
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_busqueda), "Buscar cliente...");

    // Ícono de búsqueda
    icono_busqueda = gtk_image_new_from_icon_name("system-search-symbolic", GTK_ICON_SIZE_BUTTON);
    gtk_widget_set_name(icono_busqueda, "icono-busqueda");

    // Configurar autocompletado
    GtkEntryCompletion *completion = gtk_entry_completion_new();
    // Crear un modelo de lista que contendrá cadenas (nombre, teléfono y correo)
    GtkListStore *store = gtk_list_store_new(1, G_TYPE_STRING);

    // Abrir la base de datos y llenar el modelo con datos
    sqlite3 *db;
    int rc = sqlite3_open("clientes.db", &db);
    if (rc == SQLITE_OK) {
        const char *sql = "SELECT nombre, telefono, correo FROM clientes;";
        sqlite3_stmt *stmt;
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        if (rc == SQLITE_OK) {
            while (sqlite3_step(stmt) == SQLITE_ROW) {
                const char *nombre = (const char *)sqlite3_column_text(stmt, 0);
                const char *telefono = (const char *)sqlite3_column_text(stmt, 1);
                const char *correo = (const char *)sqlite3_column_text(stmt, 2);
                GtkTreeIter iter;
                // Agregar nombre
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter, 0, nombre, -1);
                // Agregar teléfono
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter, 0, telefono, -1);
                // Agregar correo
                gtk_list_store_append(store, &iter);
                gtk_list_store_set(store, &iter, 0, correo, -1);
            }
            sqlite3_finalize(stmt);
        } else {
            g_print("❌ Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_close(db);
    } else {
        g_print("❌ Error al abrir la base de datos: %s\n", sqlite3_errmsg(db));
    }

    // Configurar el objeto de completado con el modelo creado
    gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(store));
    gtk_entry_completion_set_text_column(completion, 0);
    gtk_entry_completion_set_inline_completion(completion, TRUE);
    gtk_entry_set_completion(GTK_ENTRY(entry_busqueda), completion);

    // Empaquetar la entrada y el ícono en el contenedor horizontal
    gtk_box_pack_start(GTK_BOX(entry_box), entry_busqueda, TRUE, TRUE, 10);
    gtk_box_pack_start(GTK_BOX(entry_box), icono_busqueda, FALSE, FALSE, 5);

    // Empaquetar el label y el contenedor horizontal en el contenedor principal
    gtk_box_pack_start(GTK_BOX(box), label_busqueda, FALSE, FALSE, 5);
    gtk_box_pack_start(GTK_BOX(box), entry_box, FALSE, FALSE, 5);

    // Conectar la señal "changed" para actualizar la búsqueda en tiempo real
    g_signal_connect(entry_busqueda, "changed", G_CALLBACK(actualizar_busqueda), NULL);

    return box;
}

// aqui se genera la ventana para poder usar los pagos
GtkWidget* generar_pago(GtkWidget *parent_window) {
    GtkWidget *box, *busqueda;

    // 📌 Crear contenedor principal
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_name(box, "formulario-pagos");
    gtk_widget_set_halign(box, GTK_ALIGN_FILL);
    gtk_widget_set_valign(box, GTK_ALIGN_FILL);
    gtk_widget_set_size_request(box, 100, -1); // Ajustamos ancho

    // 📌 Crear la barra de búsqueda
    busqueda = crear_busqueda();
    gtk_box_pack_start(GTK_BOX(box), busqueda, FALSE, FALSE, 0);

    // 📌 Crear e inicializar label_resultado
    label_resultado = gtk_label_new("");  
    gtk_widget_set_name(label_resultado, "label_resultado");
    gtk_box_pack_start(GTK_BOX(box), label_resultado, FALSE, FALSE, 0);

    // 📌 Crear un contenedor para la ficha del cliente (vacío al inicio)
    box_ficha_cliente = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_name(box_ficha_cliente, "box-ficha-cliente");

    // 🔹 Evitar que se muestre automáticamente
    gtk_widget_set_no_show_all(box_ficha_cliente, TRUE);

    // 🔹 Establecer tamaño fijo y centrar
    gtk_widget_set_size_request(box_ficha_cliente, 400, -1); // 📏 Máximo 400px de ancho
    gtk_widget_set_halign(box_ficha_cliente, GTK_ALIGN_CENTER); // 📌 Centrarlo en la ventana

    // 📌 Agregar dentro de `box`
    gtk_box_pack_start(GTK_BOX(box), box_ficha_cliente, FALSE, FALSE, 10);

    return box;
}

void actualizar_ficha_cliente(Cliente *cliente) {
    if (!cliente) {
        gtk_widget_set_no_show_all(box_ficha_cliente, TRUE);
        gtk_widget_hide(box_ficha_cliente);
        return;
    }

    // Limpiar la ficha anterior
    gtk_container_foreach(GTK_CONTAINER(box_ficha_cliente), (GtkCallback)gtk_widget_destroy, NULL);
    gtk_widget_set_no_show_all(box_ficha_cliente, FALSE);
    gtk_widget_show(box_ficha_cliente);

    GtkWidget *info_box, *label_nombre, *label_plan, *label_vencimiento, *label_saldo, *label_cuota_vencida, *btn_pagar, *icono_carrito;

    // Contenedor de la ficha con un tamaño más pequeño
    info_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_widget_set_name(info_box, "ficha-info");

    // Establecer tamaño de la ficha
    gtk_widget_set_size_request(info_box, 400, -1);
    gtk_widget_set_halign(info_box, GTK_ALIGN_CENTER); // Centrar en la ventana

    // Nombre del cliente
    char texto_nombre[100];
    snprintf(texto_nombre, sizeof(texto_nombre), "<span font='14' weight='bold'>%s</span>", cliente->nombre);
    label_nombre = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_nombre), texto_nombre);
    gtk_widget_set_halign(label_nombre, GTK_ALIGN_CENTER);

    // Tipo de plan
    char texto_plan[100];
    snprintf(texto_plan, sizeof(texto_plan), "<span font='12'>Plan: %s</span>", cliente->tipo_plan);
    label_plan = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_plan), texto_plan);
    gtk_widget_set_halign(label_plan, GTK_ALIGN_CENTER);

    // Vencimiento
    char texto_vencimiento[100];
    snprintf(texto_vencimiento, sizeof(texto_vencimiento), "<span font='12'>Vencimiento: %s</span>", cliente->fecha_fin);
    label_vencimiento = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_vencimiento), texto_vencimiento);
    gtk_widget_set_halign(label_vencimiento, GTK_ALIGN_CENTER);

    // Saldo pendiente
    char texto_saldo[100];
    snprintf(texto_saldo, sizeof(texto_saldo), "<span font='12'>Saldo pendiente: %d</span>", cliente->saldo_pendiente);
    label_saldo = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_saldo), texto_saldo);
    gtk_widget_set_halign(label_saldo, GTK_ALIGN_CENTER);

    // Mostrar "CUOTA VENCIDA" si el saldo es mayor a 0
    if (cliente->saldo_pendiente > 0) {
        label_cuota_vencida = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(label_cuota_vencida), "<span font='14' foreground='red' weight='bold'>CUOTA VENCIDA</span>");
        gtk_widget_set_halign(label_cuota_vencida, GTK_ALIGN_CENTER);
        gtk_box_pack_start(GTK_BOX(info_box), label_cuota_vencida, FALSE, FALSE, 5);
    }

    // Crear copia dinámica del cliente para usarla en el callback
    Cliente *cliente_copia = malloc(sizeof(Cliente));
    if (cliente_copia == NULL) {
        fprintf(stderr, "Error al asignar memoria para cliente_copia.\n");
        return;
    }
    *cliente_copia = *cliente;  // Copia todos los campos sin casteos

    // Botón de pago más pequeño y centrado
    btn_pagar = gtk_button_new_with_label(" Pagar");

    // Aplicar estilo CSS con una clase
    GtkStyleContext *context = gtk_widget_get_style_context(btn_pagar);
    gtk_style_context_add_class(context, "boton-pago");

    // Reducir tamaño del botón
    gtk_widget_set_size_request(btn_pagar, 150, 40);
    gtk_widget_set_halign(btn_pagar, GTK_ALIGN_CENTER);

    // Conectar la señal "clicked" al callback proceder_pago, pasando la copia dinámica del Cliente
    g_signal_connect(btn_pagar, "clicked", G_CALLBACK(procederPago), cliente_copia);
    // Agregar icono de carrito en el botón
    icono_carrito = gtk_image_new_from_icon_name("emblem-money", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btn_pagar), icono_carrito);

    // Agregar widgets al contenedor
    gtk_box_pack_start(GTK_BOX(info_box), label_nombre, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(info_box), label_plan, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(info_box), label_vencimiento, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(info_box), label_saldo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(info_box), btn_pagar, FALSE, FALSE, 10); // Agregar espacio debajo

    // Agregar la ficha al contenedor principal
    gtk_box_pack_start(GTK_BOX(box_ficha_cliente), info_box, FALSE, FALSE, 0);

    // Mostrar la ficha con todos sus elementos
    gtk_widget_show_all(box_ficha_cliente);
}

void cambiar_plan_callback(GtkWidget *widget, gpointer data) {
    GtkWidget **widgets = (GtkWidget **)data;
    GtkWidget *combo_plan = widgets[0];
    GtkWidget *label_monto = widgets[1];
    GtkWidget *label_fin = widgets[2];  
    gboolean alta = (gboolean)(intptr_t)widgets[3];
    const gchar *fecha_inicio = (const gchar *)widgets[4];
    Cliente *cliente = (Cliente *)widgets[5]; // ✅ Cliente obtenido correctamente

    const char *plan = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_plan));
    
    if (!plan) return; // Evita errores si el plan es NULL

    // ✅ Actualizar el plan del cliente
    strncpy(cliente->tipo_plan, plan, sizeof(cliente->tipo_plan));

    // ✅ Calcular nueva fecha_fin
    int day, month, year;
    sscanf(fecha_inicio, "%d/%d/%d", &day, &month, &year);
    
    if (strcmp(plan, "Mensual") == 0) {
        month += 1;
        if (month > 12) { month = 1; year++; }
    } else if (strcmp(plan, "Anual") == 0) {
        year += 1;
    }

    // ✅ Actualizar fecha_fin en el cliente
    snprintf(cliente->fecha_fin, sizeof(cliente->fecha_fin), "%02d/%02d/%04d", day, month, year);

    // ✅ Actualizar la interfaz con la nueva fecha de fin
    char fecha_fin_text[50];
    snprintf(fecha_fin_text, sizeof(fecha_fin_text), "<span font='13'>Fecha de Fin: %02d/%02d/%04d</span>", day, month, year);
    gtk_label_set_markup(GTK_LABEL(label_fin), fecha_fin_text);


    // ✅ Actualizar el monto a pagar
    int monto_base = g_strcmp0(plan, "Mensual") == 0 ? 400 : 3800;
    int monto_final = alta ? (monto_base + 50) : monto_base;

    gchar *texto_monto = alta
        ? g_strdup_printf("<span font='15' weight='bold' foreground='#9bc09f'>Monto a Pagar: $%d (Incluye $50 por registro)</span>", monto_final)
        : g_strdup_printf("<span font='15' weight='bold' foreground='#9bc09f'>Monto a Pagar: $%d</span>", monto_final);
    gtk_label_set_markup(GTK_LABEL(label_monto), texto_monto);
    g_free(texto_monto);
    }

void actualizar_fecha_fin_label(const gchar *fecha_inicio, const gchar *plan, GtkWidget *label_fin) {
    int day, month, year;
    sscanf(fecha_inicio, "%d/%d/%d", &day, &month, &year);

    if (g_strcmp0(plan, "Mensual") == 0) {
        month += 1;
        if (month > 12) { month = 1; year++; }
    } else if (g_strcmp0(plan, "Anual") == 0) {
        year += 1;
    }

    gchar fecha_fin[50];
    snprintf(fecha_fin, sizeof(fecha_fin), "<span font='12'>Fecha de Fin: %02d/%02d/%04d</span>", day, month, year);
    gtk_label_set_markup(GTK_LABEL(label_fin), fecha_fin);
}

// Función principal para crear la ventana de pago
void procederPago(GtkWidget *widget, gpointer data) {
    Cliente *cliente = (Cliente *)data;
    gboolean alta = FALSE; // Indica que no es un alta nueva
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Procesar Pago",
                                                    GTK_WINDOW(gtk_widget_get_toplevel(widget)),
                                                    GTK_DIALOG_MODAL,
                                                    "_Cancelar", GTK_RESPONSE_CANCEL,
                                                    "_Pagar", GTK_RESPONSE_ACCEPT,
                                                    NULL);

    // Verificamos que el diálogo es válido
    if (!GTK_IS_DIALOG(dialog)) {
        g_warning("El diálogo no es válido.");
        return;
    }

    gtk_widget_set_name(dialog, "dialog-proceder-pago");

    GtkWidget *btn_cancelar = gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
    GtkStyleContext *context_cancelar = gtk_widget_get_style_context(btn_cancelar);
    gtk_style_context_add_class(context_cancelar, "boton-cancelar");

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_widget_set_name(content_area, "content-area-proceder-pago");

    GtkWidget *box_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box_main), 10);
    gtk_box_pack_start(GTK_BOX(content_area), box_main, TRUE, TRUE, 0);

    // Información del Cliente
    char texto_nombre[150];
    snprintf(texto_nombre, sizeof(texto_nombre), "<span font='16' weight='bold'>%s</span>", cliente->nombre);
    GtkWidget *label_nombre = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_nombre), texto_nombre);
    gtk_box_pack_start(GTK_BOX(box_main), label_nombre, FALSE, FALSE, 5);

    char texto_inicio[100], texto_fin[100];
    snprintf(texto_inicio, sizeof(texto_inicio), "<span font='12'>Fecha de Inicio: %s</span>", cliente->fecha_inicio);
    snprintf(texto_fin, sizeof(texto_fin), "<span font='12'>Fecha de Fin: %s</span>", cliente->fecha_fin);

    GtkWidget *label_inicio = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_inicio), texto_inicio);
    gtk_box_pack_start(GTK_BOX(box_main), label_inicio, FALSE, FALSE, 5);

    // Este label mostrará la fecha de fin y se actualizará si cambia el plan
    GtkWidget *label_fin = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_fin), texto_fin);
    gtk_box_pack_start(GTK_BOX(box_main), label_fin, FALSE, FALSE, 5);

    // Métodos de Pago
    GtkWidget *box_pago = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(box_pago, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(box_main), box_pago, FALSE, FALSE, 5);

    GtkWidget *radio_efectivo = gtk_radio_button_new_with_label(NULL, " Efectivo 💵");
    gtk_box_pack_start(GTK_BOX(box_pago), radio_efectivo, FALSE, FALSE, 0);

    GtkWidget *radio_tarjeta = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_efectivo), " Tarjeta 💳");
    gtk_box_pack_start(GTK_BOX(box_pago), radio_tarjeta, FALSE, FALSE, 0);

    // --- Tipo de Plan y Monto a Pagar (modificado para actualizar fecha y monto)
    // Se reemplaza el comentario incorrecto con //.
    GtkWidget *grid_info = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid_info), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid_info), 10);
    gtk_box_pack_start(GTK_BOX(box_main), grid_info, FALSE, FALSE, 0);

    gtk_grid_attach(GTK_GRID(grid_info), gtk_label_new("Tipo de Plan:"), 0, 0, 1, 1);
    GtkWidget *combo_plan = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_plan), "Mensual");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_plan), "Anual");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_plan), strcmp(cliente->tipo_plan, "Anual") == 0 ? 1 : 0);
    gtk_widget_set_sensitive(combo_plan, FALSE);
    gtk_grid_attach(GTK_GRID(grid_info), combo_plan, 1, 0, 1, 1);

    GtkWidget *btn_cambiar = gtk_button_new_with_label("Cambiar Plan");
    GtkStyleContext *context_cambiar = gtk_widget_get_style_context(btn_cambiar);
    gtk_style_context_add_class(context_cambiar, "boton-cambiar");
    gtk_grid_attach(GTK_GRID(grid_info), btn_cambiar, 2, 0, 1, 1);
    g_signal_connect(btn_cambiar, "clicked", G_CALLBACK(activar_combo_plan), combo_plan);

    // Monto a Pagar
    GtkWidget *label_monto = gtk_label_new(NULL);
    int monto_base = strcmp(cliente->tipo_plan, "Anual") == 0 ? 3800 : 400;

    gchar *texto_monto = g_strdup_printf(
        "<span font='15' weight='bold' foreground='#9bc09f'>Monto a Pagar: $%d </span>", 
        monto_base
    );
    gtk_label_set_markup(GTK_LABEL(label_monto), texto_monto);
    g_free(texto_monto);
    gtk_box_pack_start(GTK_BOX(box_main), label_monto, FALSE, FALSE, 10);
    
    // Conectar el combo al callback que actualiza la fecha de fin y el monto
    const gchar *fecha_inicio_str = cliente->fecha_inicio;
    GtkWidget *widgets[6] = {
        combo_plan,                             // [0]: Combo de plan
        label_monto,                            // [1]: Label del monto a pagar
        label_fin,                              // [2]: Label de fecha de fin
        (GtkWidget *)(intptr_t)alta,            // [3]: Bandera de alta (convertida a pointer)
        (GtkWidget *)fecha_inicio_str,          // [4]: Fecha de inicio (cadena)
        (GtkWidget *)cliente                    // [5]: Cliente
    };
    g_signal_connect(combo_plan, "changed", G_CALLBACK(cambiar_plan_callback), widgets);
    
    // NOTA: Se elimina la duplicación del bloque de actualización del label_monto,
    // ya que éste ya se creó y empacó en box_main.
    
    gtk_widget_show_all(dialog);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    if (response == GTK_RESPONSE_ACCEPT) {
        // Acción de pago exitosa
        g_print("Pago realizado con éxito\n");
    } else {
        // Si el usuario canceló
        g_print("Pago cancelado\n");
    }

    gtk_widget_destroy(dialog);  // Asegúrate de destruir el diálogo al final
}

void activar_combo_plan(GtkWidget *widget, gpointer data) {
    GtkWidget *combo_plan = GTK_WIDGET(data);
    gtk_widget_set_sensitive(combo_plan, TRUE);
}