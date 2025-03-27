#include <stdio.h>
#include <sqlite3.h>
#include "db_setup.h"  

// Función para crear la base de datos y las tablas necesarias
int inicializar_base_datos() {
    sqlite3 *db;
    char *errMsg = 0;

    // Abrir o crear la base de datos
    int rc = sqlite3_open("clientes.db", &db);
    if (rc != SQLITE_OK) {
        printf("❌ Error al abrir/crear la base de datos: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    // Crear la tabla de clientes (solo con los datos básicos)
    const char *sql_clientes = 
        "CREATE TABLE IF NOT EXISTS clientes ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "nombre TEXT NOT NULL, "
        "telefono TEXT UNIQUE NOT NULL, "
        "correo TEXT UNIQUE NOT NULL, "
        "tipo_plan TEXT NOT NULL, "
        "fecha_inicio DATE NOT NULL, "
        "fecha_fin DATE NOT NULL"
        ");";

    // Ejecutar la creación de la tabla clientes
    rc = sqlite3_exec(db, sql_clientes, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        printf("Error al crear la tabla 'clientes': %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 2;
    } else {
        printf("Tabla 'clientes' creada correctamente o ya existente.\n");
    }

    // Crear la tabla de pago_cliente (para detalles de pago y plan)
    const char *sql_pago_cliente = 
        "CREATE TABLE IF NOT EXISTS pago_cliente ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "cliente_id INTEGER NOT NULL, "
        "saldo_pendiente BOOLEAN DEFAULT 1, "
        "meses_adelantados INTEGER DEFAULT 0, "
        "años_adelantados INTEGER DEFAULT 0, "
        "FOREIGN KEY(cliente_id) REFERENCES clientes(id)"
        ");";

    // Ejecutar la creación de la tabla pago_cliente
    rc = sqlite3_exec(db, sql_pago_cliente, 0, 0, &errMsg);
    if (rc != SQLITE_OK) {
        printf("Error al crear la tabla 'pago_cliente': %s\n", errMsg);
        sqlite3_free(errMsg);
        sqlite3_close(db);
        return 2;
    } else {
        printf("Tabla 'pago_cliente' creada correctamente o ya existente.\n");
    }

    sqlite3_close(db);
    printf("Base de datos inicializada con éxito.\n");
    return 0;
}

