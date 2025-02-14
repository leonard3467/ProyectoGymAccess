#ifndef CLIENTES_H
#define CLIENTES_H

#include <stdbool.h>
#include <gtk/gtk.h>  // Asegurar que GTK esté disponible

#define MAX_DIAS_MES 31
#define ARCHIVO_CLIENTES "clientes.txt"

// Definición de la estructura Cliente
typedef struct {
    int id;
    char nombre[50];
    char telefono[15];
    char correo[50];
    char tipo_plan[20];
    char fecha_inicio[11];
    char fecha_fin[11];
    int asistencia_total;
    bool saldo_pendiente;
    int meses_adelantados;  
    int años_adelantados;  
} Cliente;

// 📌 Declaraciones de funciones

// Funciones de recuperación y almacenamiento de clientes
int recuperar_clientes(Cliente clientes[], int max_clientes);
int obtener_siguiente_id();
void guardar_cliente(GtkWidget *widget, gpointer data);

// 📌 Funciones de validación generales (devuelven bool)
bool validar_telefono(const char *telefono);
bool validar_correo(const char *correo);

// 📌 Funciones para actualizar advertencias en tiempo real en la UI (GTK)
void actualizar_validacion_telefono(GtkWidget *widget, gpointer data);
void actualizar_validacion_correo(GtkWidget *widget, gpointer data);
void abrir_calendario(GtkWidget *widget, gpointer data) ;
void calcular_fecha_fin(int day, int month, int year);
void actualizar_fecha_fin(GtkWidget *widget, gpointer data);
// 📌 Creación del formulario de registro
GtkWidget* crear_formulario_registro(GtkWidget *parent_window);
void limpiar_formulario(GtkWidget *widget, gpointer data);
void filtrar_telefono(GtkEditable *editable, const gchar *text, gint length, gint *position, gpointer data);
bool existe_dato_en_archivo(const char *dato, int tipo);
gint generar_pago_alta(GtkWidget *widget, gpointer data);
static void on_cancel_pago(GtkWidget *widget, gpointer dialog);
#endif
