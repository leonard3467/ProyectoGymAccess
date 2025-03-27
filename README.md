# Compilacion
 gcc -o build/gymaccess     src/main.c     src/interfaz.c      gcc -o build/gymaccess src/main.c src/interfaz.c src/sidebar.c src/clientes.c src/pagar.c src/db_setup.c     `pkg-config --cflags --libs gtk+-3.0` -lsqlite3
# Ejecucion
./build/gymaccess
# Instalación

Para poder ejecutar este código, requeriremos la instalación de **MSYS2** y **GTK** para compilar y ejecutar la aplicación. A continuación, se detallan los pasos necesarios para instalar y configurar el entorno.

## Pasos para instalar MSYS2

1. **Descargar MSYS2**:
   - Instala **MSYS2** desde [aquí](https://www.msys2.org/).
   - Una vez instalado **MSYS2**, abre la terminal de MSYS2 desde el menú de inicio.

2. **Actualizar MSYS2**:
   - Es importante asegurarse de que MSYS2 esté actualizado. Para ello, ejecuta los siguientes comandos en la terminal de MSYS2:
     
bash
     pacman -Syu


3. **Instalar GTK+ 3.0**:
   - Ejecuta el siguiente comando para instalar GTK+ 3.0 y las bibliotecas necesarias para usar la interfaz gráfica en la aplicación:
     
bash
     pacman -S mingw-w64-ucrt-x86_64-gtk3

   - Asegúrate de que el proceso termine sin errores.

4. **Verificar la instalación de GTK**:
   - Para asegurarte de que GTK se ha instalado correctamente, ejecuta el siguiente comando para verificar que el compilador pkg-config esté configurado correctamente:
     
bash
     pkg-config --cflags --libs gtk+-3.0


5. **Instalar herramientas adicionales**:
   - Además de GTK+ 3.0, necesitarás algunas herramientas adicionales para compilar. Ejecuta los siguientes comandos para instalarlas:
     
bash
     pacman -S mingw-w64-ucrt-x86_64-gcc
     pacman -S mingw-w64-ucrt-x86_64-pkg-config


Con estos pasos, se habrá instalado GTK+ 3.0 y las herramientas necesarias para compilar la aplicación en el entorno UCRT64 de MSYS2.

# GymAccess - Aplicación de Control de Usuarios para Gimnasio

**GymAccess** es una aplicación diseñada para gestionar la entrada de usuarios a un gimnasio, controlar sus pagos y suscripciones. Esta aplicación está pensada para ser manejada desde una PC con acceso de administrador, facilitando la administración de clientes y suscripciones de manera eficiente y fácil de usar.

## Características Principales

- **Gestión de Usuarios:** Registro de nuevos clientes, pago de suscripciones y acceso a información detallada sobre sus membresías.
- **Control de Acceso:** Varios métodos para que los clientes ingresen al gimnasio (ID manual, huella digital, tarjeta de acceso).
- **Restricciones de Acceso:** Los usuarios con pagos vencidos no podrán acceder al gimnasio hasta regularizar su situación.
- **Notificaciones Automáticas:** Posibilidad de integrar un sistema de notificaciones automáticas vía WhatsApp para recordar a los usuarios sobre pagos, promociones y eventos especiales.

## Interfaz de Usuario

La interfaz de **GymAccess** está organizada de manera sencilla y eficiente, con una barra lateral que incluye las siguientes opciones:

### 1. Registrar Usuario
Permite agregar nuevos clientes al sistema, ingresando sus datos personales y los detalles de la membresía. Esto incluye información básica como nombre, correo electrónico, teléfono, y el tipo de plan de suscripción elegido.

### 2. Pagar Plan
Facilita el proceso de pago de suscripciones o renovaciones de planes. El sistema permite realizar pagos de manera directa y automáticamente actualiza el estado de la membresía del usuario.

### 3. Buscar Información de Usuario
Permite consultar los datos de los clientes registrados, incluyendo su estado de membresía, historial de pagos, y detalles adicionales. Esto facilita la gestión y seguimiento de los usuarios de manera eficiente.

## Control de Acceso

El sistema de control de acceso permitirá a los usuarios ingresar al gimnasio mediante tres métodos diferentes:

- **ID Manual:** Ingresando un identificador único que puede ser una clave o número asignado al cliente.
- **Huella Digital:** Mediante un lector de huellas digitales que identifica al usuario de manera biométrica.
- **Tarjeta de Acceso:** Usando una tarjeta física que se vincula a la cuenta del usuario.

### Comunicación con Dispositivos Externos

La implementación de estas opciones de control de acceso se realizará utilizando dispositivos como el **ESP32**, que permitirá gestionar la autenticación de los usuarios de manera remota y segura.

## Restricciones de Acceso

El sistema de control de acceso estará restringido para usuarios que no tengan sus pagos al día. Si un cliente tiene una cuota vencida, el sistema no permitirá su entrada hasta que se regularice el pago de la suscripción. Esta restricción asegura que solo los clientes que han cumplido con sus obligaciones puedan acceder al gimnasio.

## Propuesta Futura: Integración de Notificaciones

En el futuro, se propone la integración de un sistema de notificaciones automáticas a través de **WhatsApp**. Este sistema enviará recordatorios a los usuarios sobre fechas de pago, promociones, eventos especiales y otras actualizaciones relevantes del gimnasio. 

### Funciones del sistema de notificaciones:

- **Recordatorios de pago:** Notificar a los usuarios antes de la fecha límite de su suscripción.
- **Promociones y eventos:** Enviar mensajes sobre descuentos especiales, eventos exclusivos y nuevos servicios.
- **Actualizaciones importantes:** Notificar sobre cambios en horarios, actividades, o políticas del gimnasio.

## Requisitos del Sistema

La aplicación está diseñada para funcionar en un entorno de escritorio, con las siguientes especificaciones mínimas:

- **Sistema Operativo:** Windows, macOS o Linux.
- **Requisitos de Hardware:** 
  - PC con acceso a internet.
  - Lector de huellas digitales (si se implementa).
  - Tarjetas de acceso o dispositivos compatibles con el sistema (si se implementa).
  
## Instalación

### 1. Clonar el Repositorio

bash
git clone https://github.com/tu-usuario/gymaccess.git

