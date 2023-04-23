# Tarmaggeddon


Hemos decidido crear un programa principal para la gestión de las distintas actividades relacionadas a GNU Tar que nos han sido encomendadas.

Para compilar, se puede simplemente ejecutar el comando make (y make clean para limpiar), pero hemos preparado unas herramientas un poco elaboradas para una mejor experiencia a la hora de realizar pruebas.

Tarmaggeddon deberá de recibir 3 argumentos:

    1) Modo: 0) Creación 1) Inserción 2) Extracción
    2) Nombre de fichero
    3) Ruta del tar.

Ejemplo: ./tarmaggeddon 1 testpackage.tar myfile.dat

Hemos preparado un makefile que por defecto compila sin warnings (flag -x a GCC), aunque estas flags se sobreescribiran si se utiliza el shell script de verificación que después se explicará.

Durante el proceso de verificación, se crearán dos carpetas:

    1) tarmaggeddon-tests: Aquí se escribirá el .tar resultante y se producirán unos archivos de texto con citas geniales de películas aún más geniales, para poder proceder a su inserción uno a uno; en el tar.
    2) extracts: Durante el proceso de extracción se depositarán aquí los ficheros extraídos, para poder verificar que la operación ha sido exitosa.

Estructura de directorio:

├── Makefile

├── README.md

├── verificaFinal.sh


└── src

        ├── create_mytar.c

        ├── extraer_fichero.c

        ├── inserta_fichero.c

        ├── s_mytarheader.h

        └── tarmaggeddon.c

## $ sh verificaFinal.sh

El proceso de verificación consistirá de varias fases, las cuales están claramente diferenciadas en distintos mensajes coloreados en la terminal.
El script limpiará al comienzo tests previos, compilará los ficheros fuentes y procederá a la creación de directorios y ficheros, así como a realizar las distintas operaciones requeridas en la actividad.

## $ sh verificaFinal.sh --debug

El argumento --debug, permitirá visualizar trazas que han sido definidos mediante directivas de preprocesamiento en el código. Mediante éste argumento el script invocará el comando make con las flags correspondientes.

## $ sh verificaFinal.sh -x

En caso de desear ver la ejecución del shell script de verificación de manera más detallada, se puede indicar el modo verbose.


Nótese que los argumentos son excluyentes, dado que si no la información mostrada en pantalla sera excesiva y redundante.

## Make

Podremos utilizar make para automatizar la compilación de Tarmaggeddon.

Como es usual, make clean limpiará ficheros objeto/símbolos de haberlos, y ejecutables. De manera adicional, make clean limpiará los directorios relacionados a las diferentes fases de la verificación.

Por defecto realiza la compilación indicando a gcc que no muestre warnings "-w", salvo que el script de verificación lo sobreescriba, en cuyo caso agregará flags extra.

## Autores
    Aingeru García (Basajaun): https://github.com/geru-scotland 
    Martin Sebastian Valderrabano: https://github.com/farolei
    Unai Artano Denche: https://github.com/UnaiAD22
