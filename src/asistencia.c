#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <stdbool.h>
#include <gtk/gtk.h>
#include <glib.h>  // expresiones regulares
#include <sqlite3.h>  
#include "pagar.h"
#include "clientes.h"
#include <time.h>  
#include "asistencia.h"
#include "interfaz.h"
// ⚙️ Configuración del lector
#define SERIAL_PORT "COM6"
#define BAUD_RATE 115200

// 📡 Variables globales de control
static HANDLE hSerial;
static volatile bool esperando_nfc = false;
static char ultimo_id_nfc[64] = "";

// Función para abrir el puerto serial
HANDLE openSerialPortWindows(const char *port) {
    // esto debe de revisar la compatibilidad para linux
    HANDLE hSerial = CreateFile(port, GENERIC_READ | GENERIC_WRITE, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);

    if (hSerial == INVALID_HANDLE_VALUE) {
        perror("No se puede abrir el puerto serial");
        exit(1);
    }
    return hSerial;
}

// Configurar el puerto
void configureSerialPortWindows(HANDLE hSerial) {
    DCB dcbSerialParams = {0};
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
    GetCommState(hSerial, &dcbSerialParams);
    dcbSerialParams.BaudRate = BAUD_RATE;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;
    SetCommState(hSerial, &dcbSerialParams);

    SetCommMask(hSerial, EV_RXCHAR);

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 50;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);
}

// Enviar comando al lector
void sendCommand(HANDLE hSerial, const char *command) {
    DWORD bytesWritten;
    WriteFile(hSerial, command, strlen(command), &bytesWritten, NULL);
    WriteFile(hSerial, "\n", 1, &bytesWritten, NULL);
    printf("desde asistencia.c Comando enviado: %s\n", command);
}

// Leer respuesta del lector
void readResponse(HANDLE hSerial) {
    OVERLAPPED ovRead = {0};
    ovRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);  // Evento manual

    char response[100];
    DWORD n = 0;

    BOOL result = ReadFile(hSerial, response, sizeof(response) - 1, &n, &ovRead);

    if (!result) {
        if (GetLastError() == ERROR_IO_PENDING) {
            // Esperar a que termine la lectura
            DWORD wait = WaitForSingleObject(ovRead.hEvent, 2000);  // Espera 2 segundos

            if (wait == WAIT_OBJECT_0) {
                GetOverlappedResult(hSerial, &ovRead, &n, FALSE);
            } else {
                CloseHandle(ovRead.hEvent);
                return;  // Timeout o error
            }
        } else {
            CloseHandle(ovRead.hEvent);
            return;  // Otro error
        }
    }

    if (n > 0) {
        response[n] = '\0';
        printf("Respuesta recibida: %s\n", response);

        // ✅ Siempre guardar el ID leído en cualquier modo
        strncpy(ultimo_id_nfc, response, sizeof(ultimo_id_nfc));

        if (esperando_nfc) {
            esperando_nfc = false;
            printf("ID NFC guardado (modo registro): %s\n", response);
        } else {
            printf("ID NFC actualizado (modo acceso): %s\n", response);
        }
    }

    CloseHandle(ovRead.hEvent);
}


// Hilo lector permanente
void iniciar_lector_serial() {
    hSerial = openSerialPortWindows(SERIAL_PORT);
    configureSerialPortWindows(hSerial);

    // Siempre enviamos "lectura" al inicio
    sendCommand(hSerial, "lectura");

    printf("Escuchando NFC por %s  desde asistencia.c...\n", SERIAL_PORT);

    while (1) {
        readResponse(hSerial);
        Sleep(100);
    }

    CloseHandle(hSerial); // En caso de salir del bucle
}

// Activar modo vinculación NFC
void activar_modo_lectura_escritura() {
    esperando_nfc = true;
    memset((char*)ultimo_id_nfc, 0, sizeof(ultimo_id_nfc));
    sendCommand(hSerial, "agregar");
}

// Obtener el ID NFC recibido
const char* obtener_id_nfc() {
    return ultimo_id_nfc;
}

gboolean verificar_lectura_nfc_simple(gpointer data) {
    static char ultimo_id_visto[64] = "";  // Guarda el último ID procesado
    const char *id_nfc = obtener_id_nfc(); // ID actual leído por readResponse()

    // Si hay algo y es diferente al último
    if (strlen(id_nfc) > 0 && strcmp(id_nfc, ultimo_id_visto) != 0) {
        strcpy(ultimo_id_visto, id_nfc);  // actualizar el último ID visto

        g_print("ID detectado desde verificar_lectura_nfc_simple: %s\n", id_nfc);

        sqlite3 *db;
        if (sqlite3_open("clientes.db", &db) != SQLITE_OK) {
            g_print("Error al abrir base de datos: %s\n", sqlite3_errmsg(db));
            return TRUE;
        }

        const char *sql = 
            "SELECT c.nombre, c.tipo_plan FROM asistencia a "
            "JOIN clientes c ON a.cliente_id = c.id "
            "WHERE a.id_nfc = ?";

        sqlite3_stmt *stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
        sqlite3_bind_text(stmt, 1, id_nfc, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            const char *nombre = (const char *)sqlite3_column_text(stmt, 0);
            const char *plan   = (const char *)sqlite3_column_text(stmt, 1);
            mostrar_ficha_bienvenida_con_mensaje(nombre);
        } else {
            g_print("ID NFC no registrado\n");
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);

        // 🧽 Limpiar el ID guardado para permitir nueva lectura
        memset((char*)ultimo_id_nfc, 0, sizeof(ultimo_id_nfc));
    }

    return TRUE;  // seguir llamándose con g_timeout_add
}

#include "interfaz.h"  // Asegúrate de tener esto arriba de tu archivo asistencia.c

void mostrar_ficha_bienvenida_con_mensaje(const char *nombre) {
    const char *mensajes[] = {
        "💪 Hoy es un gran día para entrenar.",
        "🔥 Supera tus límites.",
        "🏋️ A darlo todo en el gym.",
        "✨ Tu constancia te hace fuerte.",
        "🚀 ¡Actitud al 100%!"
    };

    int cantidad = sizeof(mensajes) / sizeof(mensajes[0]);
    srand(time(NULL));
    const char *mensaje = mensajes[rand() % cantidad];

    // Obtener la ventana principal
    GtkWidget *ventana = obtener_ventana_principal();

    GtkWidget *dialog = gtk_dialog_new();
    gtk_window_set_title(GTK_WINDOW(dialog), "¡Bienvenido!");
    gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
    gtk_window_set_decorated(GTK_WINDOW(dialog), FALSE);
    gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 180);

    // ✅ Centramos respecto a la ventana principal
    if (GTK_IS_WINDOW(ventana)) {
        gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(ventana));
        gtk_window_set_position(GTK_WINDOW(dialog), GTK_WIN_POS_CENTER_ON_PARENT);
    }

    // Aplicar estilo
    gtk_widget_set_name(dialog, "dialog-proceder-pago");

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_widget_set_name(content_area, "content-area-proceder-pago");

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(box), 20);
    gtk_widget_set_halign(box, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(box, GTK_ALIGN_CENTER);

    // Etiqueta principal (nombre)
    char texto_principal[200];
    snprintf(texto_principal, sizeof(texto_principal),
             "<span font='20' weight='bold' foreground='#00ff88'>🎉 ¡Bienvenido, %s! 🎉</span>", nombre);
    GtkWidget *label_nombre = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_nombre), texto_principal);
    gtk_widget_set_halign(label_nombre, GTK_ALIGN_CENTER);

    // Etiqueta motivacional
    GtkWidget *label_mensaje = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_mensaje),
        g_strdup_printf("<span font='13' foreground='white'>%s</span>", mensaje));
    gtk_widget_set_halign(label_mensaje, GTK_ALIGN_CENTER);

    // Empacar
    gtk_box_pack_start(GTK_BOX(box), label_nombre, FALSE, FALSE, 10);
    gtk_box_pack_start(GTK_BOX(box), label_mensaje, FALSE, FALSE, 5);
    gtk_container_add(GTK_CONTAINER(content_area), box);

    gtk_widget_show_all(dialog);

    // Cierre automático en 3 segundos
    g_timeout_add(3000, (GSourceFunc)gtk_widget_destroy, dialog);
}
