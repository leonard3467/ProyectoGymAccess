#include "clientes.h"
#include "pagar.h"  
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <glib.h>  // expresiones regulares
#include <sqlite3.h>  // usaremos esta libreria paara cambiar el .txt por una base de datos local

// 📌 Variables globales para los GtkEntry y ComboBox
static GtkWidget *entry_nombre, *entry_telefono, *entry_correo, *combo_plan, *entry_inicio, *entry_fin;
static GtkWidget *label_warn_telefono, *label_warn_correo, *label_warn_plan,*label_warn_general;

// 📌 Función para recuperar clientes desde el archivo "clientes.txt"
int recuperar_clientes(Cliente clientes[], int max_clientes) {
    FILE *archivo = fopen(ARCHIVO_CLIENTES, "r");
    if (!archivo) {
        printf("No se pudo abrir el archivo de clientes.\n");
        return 0;
    }

    int contador = 0;
    char buffer[1024];

    while (fgets(buffer, sizeof(buffer), archivo) && contador < max_clientes) {
        Cliente *c = &clientes[contador];

        sscanf(buffer, "%d|%49[^|]|%14[^|]|%49[^|]|%19[^|]|%10[^|]|%10[^|]|%d|%d|%d|%d",
            &c->id, c->nombre, c->telefono, c->correo, c->tipo_plan,
            c->fecha_inicio, c->fecha_fin, &c->asistencia_total, &c->saldo_pendiente,
            &c->meses_adelantados, &c->años_adelantados);


        contador++;
    }

    fclose(archivo);
    return contador;
}

// 📌 Función para obtener el siguiente ID
int obtener_siguiente_id() {
    FILE *archivo = fopen(ARCHIVO_CLIENTES, "r");
    if (!archivo) {
        return 1;
    }

    int id, ultimo_id = 0;
    char buffer[256];

    while (fgets(buffer, sizeof(buffer), archivo)) {
        sscanf(buffer, "%d|", &id);
        if (id > ultimo_id) {
            ultimo_id = id;
        }
    }

    fclose(archivo);
    return ultimo_id + 1;
}

// 📌 Función para validar el Teléfono (mínimo 10 dígitos)
bool validar_telefono(const char *telefono) {
    return (strlen(telefono) >= 10);
}

// 📌 Función para validar el Correo (contiene "@" y ".")
bool validar_correo(const char *correo) {
    // Expresión regular para correos electrónicos válidos
    const char *patron = "^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$";
    
    GRegex *regex = g_regex_new(patron, G_REGEX_OPTIMIZE, 0, NULL);
    gboolean resultado = g_regex_match(regex, correo, 0, NULL);
    g_regex_unref(regex); // Liberar memoria

    return resultado; // Devuelve TRUE si el correo es válido, FALSE si es inválido
}

void actualizar_validacion_telefono(GtkWidget *widget, gpointer data) {
    const char *telefono = gtk_entry_get_text(GTK_ENTRY(entry_telefono));

    if (strlen(telefono) == 0) {
        gtk_label_set_text(GTK_LABEL(label_warn_telefono), "");
        return;
    }

    if (!validar_telefono(telefono)) {
        gtk_label_set_markup(GTK_LABEL(label_warn_telefono), "<span foreground='orange'>⚠️ Mínimo 10 dígitos</span>");
    } else {
        gtk_label_set_markup(GTK_LABEL(label_warn_telefono), "<span foreground='green'>✔️ Teléfono válido</span>");
    }
}


void actualizar_validacion_correo(GtkWidget *widget, gpointer data) {
    const char *correo = gtk_entry_get_text(GTK_ENTRY(entry_correo));

    if (strlen(correo) == 0) {
        gtk_label_set_text(GTK_LABEL(label_warn_correo), "");
        return;
    }

    if (strchr(correo, '@') == NULL) {
        gtk_label_set_markup(GTK_LABEL(label_warn_correo), "<span foreground='orange'>⚠️ Falta '@' en el correo</span>");
        return;
    }

    char *arroba = strchr(correo, '@');
    if (strchr(arroba, '.') == NULL) {
        gtk_label_set_markup(GTK_LABEL(label_warn_correo), "<span foreground='orange'>⚠️ Falta '.' después del '@'</span>");
        return;
    }

    if (!validar_correo(correo)) {
        gtk_label_set_markup(GTK_LABEL(label_warn_correo), "<span foreground='orange'>⚠️ Formato inválido (ejemplo: usuario@email.com)</span>");
        return;
    }

    gtk_label_set_markup(GTK_LABEL(label_warn_correo), "<span foreground='green'>✔️ Correo válido</span>");
}



void guardar_cliente_sqlite(GtkWidget *widget, gpointer data) {
    // 1) Leer campos de los GtkEntry
    const char *nombre = gtk_entry_get_text(GTK_ENTRY(entry_nombre));
    const char *telefono = gtk_entry_get_text(GTK_ENTRY(entry_telefono));
    const char *correo = gtk_entry_get_text(GTK_ENTRY(entry_correo));
    const char *plan = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_plan));
    const char *inicio = gtk_entry_get_text(GTK_ENTRY(entry_inicio));
    const char *fin = gtk_entry_get_text(GTK_ENTRY(entry_fin));

    // 2) Validaciones antes de guardar
    if (strlen(nombre) == 0 || strlen(telefono) == 0 || strlen(correo) == 0 ||
        plan == NULL || strlen(inicio) == 0 || strlen(fin) == 0) {
        gtk_label_set_markup(GTK_LABEL(label_warn_general),
            "<span foreground='#ff4d4d'><b>⚠️ Todos los campos deben estar llenos.</b></span>");
        return;
    }

    if (!validar_telefono(telefono)) {
        gtk_label_set_markup(GTK_LABEL(label_warn_general),
            "<span foreground='#ff4d4d'><b>⚠️ El teléfono debe tener al menos 10 dígitos.</b></span>");
        return;
    }

    if (!validar_correo(correo)) {
        gtk_label_set_markup(GTK_LABEL(label_warn_general),
            "<span foreground='#ff4d4d'><b>⚠️ Correo inválido. Debe contener '@' y un dominio.</b></span>");
        return;
    }

    // 3) Verificar duplicados (en BD). Puedes omitir esto y capturar el error UNIQUE
    //    del INSERT, pero aquí un ejemplo con SELECT:

    sqlite3 *db;
    char *errMsg = NULL;
    int rc = sqlite3_open("clientes.db", &db);
    if (rc != SQLITE_OK) {
        g_print("❌ Error al abrir la base de datos: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Consulta para ver si teléfono o correo ya existen
    const char *sql_busqueda = 
        "SELECT 1 FROM clientes WHERE telefono = ? OR correo = ? LIMIT 1;";
    sqlite3_stmt *stmt_busqueda;
    rc = sqlite3_prepare_v2(db, sql_busqueda, -1, &stmt_busqueda, NULL);
    if (rc != SQLITE_OK) {
        g_print("❌ Error al preparar la consulta de duplicados: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return;
    }

    // Bind de parámetros (teléfono, correo)
    sqlite3_bind_text(stmt_busqueda, 1, telefono, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_busqueda, 2, correo,   -1, SQLITE_STATIC);

    rc = sqlite3_step(stmt_busqueda);
    if (rc == SQLITE_ROW) {
        // Significa que encontró un registro => duplicado
        gtk_label_set_markup(GTK_LABEL(label_warn_general),
            "<span foreground='#ff4d4d'><b>⚠️ El teléfono o correo ya fue dado de alta.</b></span>");
        sqlite3_finalize(stmt_busqueda);
        sqlite3_close(db);
        return;
    }
    // Si rc == SQLITE_DONE => no hay duplicado
    sqlite3_finalize(stmt_busqueda);
    // No cerramos todavía db, la usaremos tras generar_pago

    // 4) Crear el struct Cliente en memoria
    Cliente *nuevo_cliente = malloc(sizeof(Cliente));
    if (!nuevo_cliente) {
        g_print("❌ Error: no se pudo asignar memoria para el cliente\n");
        sqlite3_close(db);
        return;
    }

    // Asignar valores. (ID no es necesario si en la BD es AUTOINCREMENT).
    nuevo_cliente->id = 0; // se autogenerará
    strncpy(nuevo_cliente->nombre, nombre, sizeof(nuevo_cliente->nombre));
    strncpy(nuevo_cliente->telefono, telefono, sizeof(nuevo_cliente->telefono));
    strncpy(nuevo_cliente->correo,  correo,  sizeof(nuevo_cliente->correo));
    strncpy(nuevo_cliente->tipo_plan, plan,  sizeof(nuevo_cliente->tipo_plan));
    strncpy(nuevo_cliente->fecha_inicio, inicio, sizeof(nuevo_cliente->fecha_inicio));
    strncpy(nuevo_cliente->fecha_fin,    fin,    sizeof(nuevo_cliente->fecha_fin));
    nuevo_cliente->asistencia_total  = 0;
    nuevo_cliente->saldo_pendiente  = 1;  // al inicio 1 => “tiene que pagar”
    nuevo_cliente->meses_adelantados= 0;
    nuevo_cliente->años_adelantados = 0;

    // 5) Abrir ventana de pago
    gint response = generar_pago_alta(widget, nuevo_cliente);

    // 6) Si el usuario canceló el pago
    if (response == GTK_RESPONSE_CANCEL || response == GTK_RESPONSE_DELETE_EVENT) {
        gtk_label_set_markup(GTK_LABEL(label_warn_general),
            "<span foreground='#ff4d4d'><b>⚠️ Cliente no registrado porque no se realizó el pago.</b></span>");
        free(nuevo_cliente);
        sqlite3_close(db);
        return;
    }

    // 7) Insertar en la BD (pago realizado => saldo_pendiente = 0)
    nuevo_cliente->saldo_pendiente = 0;

    const char *sql_insert = 
        "INSERT INTO clientes (nombre, telefono, correo, tipo_plan, fecha_inicio, fecha_fin, "
        "asistencia_total, saldo_pendiente, meses_adelantados, años_adelantados) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?);";

    sqlite3_stmt *stmt_insert;
    rc = sqlite3_prepare_v2(db, sql_insert, -1, &stmt_insert, NULL);
    if (rc != SQLITE_OK) {
        g_print("❌ Error al preparar INSERT: %s\n", sqlite3_errmsg(db));
        free(nuevo_cliente);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt_insert, 1, nuevo_cliente->nombre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert, 2, nuevo_cliente->telefono, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert, 3, nuevo_cliente->correo, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert, 4, nuevo_cliente->tipo_plan, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert, 5, nuevo_cliente->fecha_inicio, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt_insert, 6, nuevo_cliente->fecha_fin, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt_insert,  7, nuevo_cliente->asistencia_total);
    sqlite3_bind_int(stmt_insert,  8, nuevo_cliente->saldo_pendiente);
    sqlite3_bind_int(stmt_insert,  9, nuevo_cliente->meses_adelantados);
    sqlite3_bind_int(stmt_insert, 10, nuevo_cliente->años_adelantados);

    rc = sqlite3_step(stmt_insert);
    if (rc != SQLITE_DONE) {
        g_print("❌ Error al insertar el cliente: %s\n", sqlite3_errmsg(db));
        gtk_label_set_markup(GTK_LABEL(label_warn_general),
            "<span foreground='#ff4d4d'><b>⚠️ Error al insertar en la BD</b></span>");
    } else {
        g_print("✔️ Cliente registrado exitosamente en la base de datos.\n");
        gtk_label_set_text(GTK_LABEL(label_warn_general),
            "✔️ Cliente registrado con éxito y pago realizado (en la BD).");
    }

    sqlite3_finalize(stmt_insert);
    sqlite3_close(db);

    // 8) Liberar memoria del struct
    free(nuevo_cliente);
}


void abrir_calendario(GtkWidget *widget, gpointer data) {
    GtkWidget *dialog, *calendar, *content_area;
    GtkWidget *parent_window = GTK_WIDGET(data);
    gint year, month, day;

    // Crear el diálogo modal con un calendario
    dialog = gtk_dialog_new_with_buttons("Seleccionar Fecha",
                                         GTK_WINDOW(parent_window),
                                         GTK_DIALOG_MODAL,
                                         "_Aceptar", GTK_RESPONSE_ACCEPT,
                                         "_Cancelar", GTK_RESPONSE_CANCEL,
                                         NULL);

    // **Reducir aún más el tamaño del diálogo**
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    gtk_widget_set_size_request(dialog, 280, 200);  // 📏 Aún más pequeño

    // Asignar nombre al diálogo y calendario para aplicar estilos
    gtk_widget_set_name(dialog, "calendario-dialog");

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    // Crear el calendario
    calendar = gtk_calendar_new();
    gtk_widget_set_name(calendar, "calendario-widget");  // **Nombre para CSS**
    gtk_box_pack_start(GTK_BOX(content_area), calendar, TRUE, TRUE, 0);

    // **Aplicar estilos CSS**
    GtkCssProvider *cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, "styles/style.css", NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
        GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    gtk_widget_show_all(dialog);

    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        gtk_calendar_get_date(GTK_CALENDAR(calendar), &year, &month, &day);
        month += 1;

        char fecha_inicio[11];
        snprintf(fecha_inicio, sizeof(fecha_inicio), "%02d/%02d/%04d", day, month, year);
        gtk_entry_set_text(GTK_ENTRY(entry_inicio), fecha_inicio);

        calcular_fecha_fin(day, month, year);
    }

    gtk_widget_destroy(dialog);
}


void calcular_fecha_fin(int day, int month, int year) {
    const char *plan = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo_plan));

    if (plan == NULL) {
        gtk_label_set_markup(GTK_LABEL(label_warn_plan), "<span foreground='orange'>⚠️ Primero selecciona un plan</span>");
        gtk_entry_set_text(GTK_ENTRY(entry_fin), ""); // Limpiar el campo
        return;
    }

    gtk_label_set_markup(GTK_LABEL(label_warn_plan), "<span foreground='green'>✔️ Plan seleccionado</span>"); // Quitar advertencia

    if (strcmp(plan, "Mensual") == 0) {
        month += 1;  
        if (month > 12) {
            month = 1;
            year++;
        }
    } else if (strcmp(plan, "Anual") == 0) {
        year += 1;  
    }

    char fecha_fin[11];
    snprintf(fecha_fin, sizeof(fecha_fin), "%02d/%02d/%04d", day, month, year);
    gtk_entry_set_text(GTK_ENTRY(entry_fin), fecha_fin);
}


// 📌 Función para actualizar la fecha de fin cuando se selecciona un plan
void actualizar_fecha_fin(GtkWidget *widget, gpointer data) {
    const char *fecha_inicio = gtk_entry_get_text(GTK_ENTRY(entry_inicio));

    if (strlen(fecha_inicio) == 0) {
        //gtk_label_set_text(GTK_LABEL(label_warn_plan), "⚠️ Primero selecciona una fecha de inicio");
        return;
    }

    int day, month, year;
    sscanf(fecha_inicio, "%d/%d/%d", &day, &month, &year);
    calcular_fecha_fin(day, month, year);
}

GtkWidget* crear_formulario_registro(GtkWidget *parent_window) {
    GtkWidget *box, *grid, *btn_alta, *btn_cancelar, *box_botones;

    // 📌 Caja principal para centrar el formulario
    box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_name(box, "formulario-registro");
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);  // Centrar horizontalmente
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);  // Centrar verticalmente
    gtk_widget_set_size_request(box, 400, -1);     // Tamaño fijo en ancho

    // 📌 Grid para alinear los campos del formulario
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_box_pack_start(GTK_BOX(box), grid, TRUE, TRUE, 0);

    
    // 📌 Campo: Nombre
    GtkWidget *label_nombre = gtk_label_new("Nombre:");
    gtk_widget_set_name(label_nombre, "label-registro");
    entry_nombre = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), label_nombre, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_nombre, 1, 0, 1, 1);

    // 📌 Campo: Teléfono
    GtkWidget *label_telefono = gtk_label_new("Teléfono:");
    gtk_widget_set_name(label_telefono, "label-registro");
    entry_telefono = gtk_entry_new();
    label_warn_telefono = gtk_label_new("");  
    gtk_grid_attach(GTK_GRID(grid), label_telefono, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_telefono, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_warn_telefono, 2, 1, 1, 1);
    g_signal_connect(entry_telefono, "insert-text", G_CALLBACK(filtrar_telefono), NULL);
    g_signal_connect(entry_telefono, "changed", G_CALLBACK(actualizar_validacion_telefono), NULL);

    // 📌 Aplicar configuración para evitar que el texto haga crecer la ventana
    gtk_widget_set_size_request(label_warn_telefono, 200, -1);
    gtk_widget_set_hexpand(label_warn_telefono, FALSE);
    gtk_widget_set_halign(label_warn_telefono, GTK_ALIGN_START);
    gtk_label_set_ellipsize(GTK_LABEL(label_warn_telefono), PANGO_ELLIPSIZE_END);


   // 📌 Campo: Correo
    GtkWidget *label_correo = gtk_label_new("Correo:");
    gtk_widget_set_name(label_correo, "label-registro");
    entry_correo = gtk_entry_new();
    label_warn_correo = gtk_label_new("");  
    gtk_grid_attach(GTK_GRID(grid), label_correo, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_correo, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label_warn_correo, 2, 2, 1, 1);
    g_signal_connect(entry_correo, "changed", G_CALLBACK(actualizar_validacion_correo), NULL);

    // 📌 Aplicar configuración para evitar que el texto haga crecer la ventana
    gtk_widget_set_size_request(label_warn_correo, 200, -1);
    gtk_widget_set_hexpand(label_warn_correo, FALSE);
    gtk_widget_set_halign(label_warn_correo, GTK_ALIGN_START);
    gtk_label_set_ellipsize(GTK_LABEL(label_warn_correo), PANGO_ELLIPSIZE_END);


    // 📌 Campo: Plan
    GtkWidget *label_plan = gtk_label_new("Tipo de Plan:");
    gtk_widget_set_name(label_plan, "label-registro");
    combo_plan = gtk_combo_box_text_new();
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_plan), "Mensual");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo_plan), "Anual");
    gtk_grid_attach(GTK_GRID(grid), label_plan, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo_plan, 1, 3, 1, 1);

    label_warn_plan = gtk_label_new("");
    gtk_grid_attach(GTK_GRID(grid), label_warn_plan, 2, 3, 1, 1);
    g_signal_connect(combo_plan, "changed", G_CALLBACK(actualizar_fecha_fin), NULL);


    // 📌 Campo: Fecha Inicio con botón calendario
    GtkWidget *label_inicio = gtk_label_new("Fecha Inicio (dd/mm/aaaa):");
    gtk_widget_set_name(label_inicio, "label-registro");
    GtkWidget *hbox_inicio = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    entry_inicio = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(entry_inicio), FALSE);
    gtk_widget_set_can_focus(entry_inicio, FALSE);
    gtk_entry_set_text(GTK_ENTRY(entry_inicio), "");
    gtk_widget_set_name(entry_inicio, "entry-fecha");

    GtkWidget *btn_calendario = gtk_button_new();
    GtkWidget *icono_calendario = gtk_image_new_from_icon_name("x-office-calendar", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(btn_calendario), icono_calendario);

    // 📌 Asigna la clase CSS "boton-calendario"
    
    GtkStyleContext *context_calendario = gtk_widget_get_style_context(btn_calendario);
    gtk_style_context_add_class(context_calendario, "boton-calendario");
    
    gtk_box_pack_start(GTK_BOX(hbox_inicio), entry_inicio, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_inicio), btn_calendario, FALSE, FALSE, 0);

    gtk_grid_attach(GTK_GRID(grid), label_inicio, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), hbox_inicio, 1, 4, 1, 1);
    g_signal_connect(btn_calendario, "clicked", G_CALLBACK(abrir_calendario), parent_window);

    // 📌 Campo: Fecha Fin
    GtkWidget *label_fin = gtk_label_new("Fecha Fin (dd/mm/aaaa):");
    gtk_widget_set_name(label_fin, "label-registro");
    entry_fin = gtk_entry_new();
    gtk_editable_set_editable(GTK_EDITABLE(entry_fin), FALSE);
    gtk_widget_set_can_focus(entry_fin, FALSE);
    gtk_widget_set_name(entry_fin, "entry-fecha"); 
    gtk_grid_attach(GTK_GRID(grid), label_fin, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry_fin, 1, 5, 1, 1);

    // 📌 Etiqueta de advertencia general
    label_warn_general = gtk_label_new("");
    gtk_widget_set_name(label_warn_general, "label-warning");
    gtk_grid_attach(GTK_GRID(grid), label_warn_general, 0, 6, 2, 1);

    // 📌 Caja para los botones (horizontal)
    box_botones = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(box_botones), TRUE);

    // 📌 Botón "Dar de Alta" (verde)
    btn_alta = gtk_button_new_with_label("Dar de Alta");
    g_signal_connect(btn_alta, "clicked", G_CALLBACK(guardar_cliente_sqlite), NULL);
    // 📌 Botón "Cancelar" (rojo)
    btn_cancelar = gtk_button_new_with_label("Cancelar");
    g_signal_connect(btn_cancelar, "clicked", G_CALLBACK(limpiar_formulario), NULL);


    // 📌 Agregar clases CSS para colorear los botones
    GtkStyleContext *context_alta = gtk_widget_get_style_context(btn_alta);
    gtk_style_context_add_class(context_alta, "boton-alta");

    GtkStyleContext *context_cancelar = gtk_widget_get_style_context(btn_cancelar);
    gtk_style_context_add_class(context_cancelar, "boton-cancelar");

    // 📌 Agregar botones a la caja horizontal
    gtk_box_pack_start(GTK_BOX(box_botones), btn_alta, TRUE, TRUE, 5);
    gtk_box_pack_start(GTK_BOX(box_botones), btn_cancelar, TRUE, TRUE, 5);

    // 📌 Agregar la caja de botones al formulario
    gtk_box_pack_start(GTK_BOX(box), box_botones, FALSE, FALSE, 10);

    return box;
}
void limpiar_formulario(GtkWidget *widget, gpointer data) {
    // Limpiar todas las entradas de texto
    gtk_entry_set_text(GTK_ENTRY(entry_nombre), "");
    gtk_entry_set_text(GTK_ENTRY(entry_telefono), "");
    gtk_entry_set_text(GTK_ENTRY(entry_correo), "");
    gtk_entry_set_text(GTK_ENTRY(entry_inicio), "");
    gtk_entry_set_text(GTK_ENTRY(entry_fin), "");

    // Resetear el ComboBox de planes
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_plan), -1);

    // Limpiar las etiquetas de advertencias
    gtk_label_set_text(GTK_LABEL(label_warn_telefono), "");
    gtk_label_set_text(GTK_LABEL(label_warn_correo), "");
    gtk_label_set_text(GTK_LABEL(label_warn_plan), "");
    gtk_label_set_text(GTK_LABEL(label_warn_general), "");
}
void filtrar_telefono(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer data) {
    // Obtener el texto actual del campo
    const gchar *current_text = gtk_entry_get_text(GTK_ENTRY(editable));
    gint current_length = strlen(current_text);

    // Si al agregar se exceden 10 dígitos, bloquear la inserción y mostrar advertencia
    if (current_length + length > 10) {
         g_signal_stop_emission_by_name(editable, "insert-text");
         gtk_label_set_markup(GTK_LABEL(label_warn_telefono),
              "<span foreground='orange'>⚠️no exceder 10 dígitos</span>");
         return;
    }

    // Verificar que los caracteres a insertar sean numéricos
    for (int i = 0; i < length; i++) {
         if (!g_ascii_isdigit(text[i])) {
              g_signal_stop_emission_by_name(editable, "insert-text");
              gtk_label_set_markup(GTK_LABEL(label_warn_telefono),
                   "<span foreground='orange'>⚠️ Solo números permitidos</span>");
              return;
         }
    }

    // Si todo es correcto, quitar el mensaje de advertencia
    gtk_label_set_text(GTK_LABEL(label_warn_telefono), "");
}


bool existe_dato_en_archivo(const char *dato, int tipo) {
    FILE *archivo = fopen(ARCHIVO_CLIENTES, "r");
    if (!archivo) {
        return false; // Si no se puede abrir, asumimos que no hay duplicados
    }

    char buffer[1024];
    Cliente c;

    while (fgets(buffer, sizeof(buffer), archivo)) {
        sscanf(buffer, "%d|%49[^|]|%14[^|]|%49[^|]|%19[^|]|%10[^|]|%10[^|]|%d|%d",
               &c.id, c.nombre, c.telefono, c.correo, c.tipo_plan,
               c.fecha_inicio, c.fecha_fin, &c.asistencia_total, &c.saldo_pendiente);

        if (tipo == 1 && strcmp(c.telefono, dato) == 0) {
            fclose(archivo);
            return true; // Teléfono ya existe
        }

        if (tipo == 2 && strcmp(c.correo, dato) == 0) {
            fclose(archivo);
            return true; // Correo ya existe
        }
    }

    fclose(archivo);
    return false;
}
gint generar_pago_alta(GtkWidget *widget, gpointer data) {
    Cliente *cliente = (Cliente *)data;
    gboolean alta = TRUE; // 🔹 Indica que es un alta nueva
    /////////
    //diálogo sin botones predeterminados
    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "Procesar Pago");
    gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 300);

    // Asignar el ID para el diálogo para que se apliquen los estilos CSS
    gtk_widget_set_name(dialog, "dialog-proceder-pago");

    
   // Obtener la ventana principal desde el widget que se pasa (en este caso un botón u otro widget)
    GtkWidget *parent_window = gtk_widget_get_toplevel(widget);  // Esto obtiene la ventana principal
    if (GTK_IS_WINDOW(parent_window)) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(parent_window));  // Establece la ventana principal como "transiente"
        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);  // Centra el diálogo en la pantalla
    }
    // Si quieres que el diálogo sea realmente modal, es recomendable añadir:
    gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER);  
    // 2. Obtenemos el content_area
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    gtk_widget_set_name(content_area, "content-area-proceder-pago");
    GtkWidget *box_main = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box_main), 10);
    gtk_box_pack_start(GTK_BOX(content_area), box_main, TRUE, TRUE, 0);


    // 3. Creamos un GtkButtonBox para los botones (manual, no automático)
    GtkWidget *button_box = gtk_button_box_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_button_box_set_layout(GTK_BUTTON_BOX(button_box), GTK_BUTTONBOX_CENTER);
    // Aumentar el espacio entre los botones
    gtk_box_set_spacing(GTK_BOX(button_box), 20);  // <--- línea añadida
    gtk_box_pack_end(GTK_BOX(box_main), button_box, FALSE, FALSE, 5);

    // 4. Botón Cancelar
    GtkWidget *btn_cancelar = gtk_button_new_with_label("Cancelar");
    gtk_widget_set_size_request(btn_cancelar, 120, 40);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_cancelar), "boton-cancelar");
    gtk_box_pack_start(GTK_BOX(button_box), btn_cancelar, FALSE, FALSE, 10);
    g_signal_connect(btn_cancelar, "clicked", G_CALLBACK(on_cancel_pago), dialog);
    // 5. Botón Pagar
    GtkWidget *btn_pagar = gtk_button_new_with_label("Pagar");
    gtk_widget_set_size_request(btn_pagar, 120, 40);
    gtk_style_context_add_class(gtk_widget_get_style_context(btn_pagar), "boton-pago");
    gtk_box_pack_start(GTK_BOX(button_box), btn_pagar, FALSE, FALSE, 10);
    g_signal_connect_swapped(btn_pagar, "clicked",
                             G_CALLBACK(gtk_dialog_response),
                             dialog); // Responde con GTK_RESPONSE_ACCEPT

    /////////////////////////
 

    // 📌 Nombre del cliente
    char texto_nombre[150];
    snprintf(texto_nombre, sizeof(texto_nombre), "<span font='16' weight='bold'>%s</span>", cliente->nombre);
    GtkWidget *label_nombre = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_nombre), texto_nombre);
    gtk_box_pack_start(GTK_BOX(box_main), label_nombre, FALSE, FALSE, 5);

    //
    // --- Fechas de Inicio y Fin ---
    char texto_inicio[100], texto_fin[100];
    snprintf(texto_inicio, sizeof(texto_inicio), "<span font='12'>Fecha de Inicio: %s</span>", cliente->fecha_inicio);
    snprintf(texto_fin, sizeof(texto_fin), "<span font='12'>Fecha de Fin: %s</span>", cliente->fecha_fin);

    GtkWidget *label_inicio = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_inicio), texto_inicio);
    gtk_box_pack_start(GTK_BOX(box_main), label_inicio, FALSE, FALSE, 5);

    GtkWidget *label_fin = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_fin), texto_fin);
    gtk_box_pack_start(GTK_BOX(box_main), label_fin, FALSE, FALSE, 5);

    // --- Métodos de Pago ---
    GtkWidget *box_pago = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(box_pago, GTK_ALIGN_CENTER);
    gtk_box_pack_start(GTK_BOX(box_main), box_pago, FALSE, FALSE, 5);

    GtkWidget *radio_efectivo = gtk_radio_button_new_with_label(NULL, " Efectivo 💵");
    gtk_box_pack_start(GTK_BOX(box_pago), radio_efectivo, FALSE, FALSE, 0);

    GtkWidget *radio_tarjeta = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_efectivo), " Tarjeta 💳");
    gtk_box_pack_start(GTK_BOX(box_pago), radio_tarjeta, FALSE, FALSE, 0);

    // Modifica calcular_fecha_fin para actualizar fecha_fin al cambiar el combo_plan:
    g_signal_connect(combo_plan, "changed", G_CALLBACK(calcular_fecha_fin), entry_fin);


    // 📌 Grid para organizar Tipo de Plan y Botón de Cambio
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

    // 📌 Monto a Pagar
    GtkWidget *label_monto = gtk_label_new(NULL);
    int monto_base = strcmp(cliente->tipo_plan, "Anual") == 0 ? 3800 : 400;
    int monto_final = monto_base + 50; // 🔹 Se agregan 50 pesos por alta

    gchar *texto_monto = g_strdup_printf(
        "<span font='15' weight='bold' foreground='#9bc09f'>Monto a Pagar: $%d (Incluye $50 por registro)</span>", 
        monto_final
    );
    gtk_label_set_markup(GTK_LABEL(label_monto), texto_monto);
    g_free(texto_monto);
    
    gtk_box_pack_start(GTK_BOX(box_main), label_monto, FALSE, FALSE, 10);

    const gchar *fecha_inicio = cliente->fecha_inicio;
    GtkWidget *widgets[6] = { 
        combo_plan,         // widgets[0]
        label_monto,        // widgets[1]
        label_fin,          // widgets[2]
        (GtkWidget *)(intptr_t)alta, // widgets[3]
        (GtkWidget *)fecha_inicio,    // widgets[4]
        (GtkWidget *)cliente          // widgets[5] <- Cliente añadido
    };

    g_signal_connect(combo_plan, "changed", G_CALLBACK(cambiar_plan_callback), widgets);


    gtk_widget_set_halign(label_nombre, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(label_inicio, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(label_fin, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(box_pago, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(grid_info, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(label_monto, GTK_ALIGN_CENTER);

    // 📌 Mostrar el diálogo y capturar la respuesta
    gtk_widget_show_all(dialog);
    gint response = gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);

    return response;  // 🔹 Se devuelve el resultado del pago (Aceptar o Cancelar)
}
static void on_cancel_pago(GtkWidget *widget, gpointer dialog) {
    gtk_dialog_response(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}