#ifndef ASISTENCIA_H
#define ASISTENCIA_H

void iniciar_lector_serial();             // Inicia el hilo lector NFC
void activar_modo_lectura_escritura();    // Envia "escritura" y espera ID
const char* obtener_id_nfc();
gboolean verificar_lectura_nfc_en_acceso(gpointer data);
gboolean verificar_lectura_nfc_simple(gpointer data);
void mostrar_ficha_bienvenida_con_mensaje(const char *nombre);
#endif