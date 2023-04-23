#!/bin/bash

#
# This file is part of the Sistemas Operativos project.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; either version 3 of the License, or (at your
# option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see <http://www.gnu.org/licenses/>.
#
#
# @authors - Geru-Scotland (https://github.com/geru-scotland)
#          - Farolei (https://github.com/farolei)
#          - UnaiAD22 (https://github.com/UnaiAD22)
#

# Constantes
readonly EXEC="tarmaggeddon"
readonly PRUEBAS_DIR="tarmaggeddon-tests"
readonly EXTRACTS_DIR="extracts"

readonly CREAR=0
readonly INSERTAR=1
readonly EXTRAER=2

readonly TAR_BASE="tarmaggeddon.tar"

readonly STAR_WARS="star-wars.dat"
readonly LOTR="el-senor-de-los-anillos.txt"
readonly FUNDACION="fundacion-asimov.txt"
readonly GOT="game-of-thrones.txt"
readonly MATRIX="matrix.dat"

readonly BASELINK="beam-me-up.dat"
readonly SYMLINK="beam-me-up-lnk"

readonly DIRINSERCION="enterprise"
readonly DIRINSERCIONDOS="real-enterprise"
readonly FICHEROENDIR="will-not-extract.txt"
readonly FICHEROENDIRDOS="extract-trek.txt"

echo "\n\033[35m#################################\n## TARMAGGEDDON - VERIFICACION ##\n#################################\033[0m\n"


# Limpiar primero.
make clean

echo "\n\033[33m[FASE: LIMPIEZA]\033[0m\n"

# Comprobar si ha habido algún error.
if [ $? -eq 0 ]
then
  echo "\033[32mMake clean sin problemas.\033[0m"
else
  echo "\033[31mError al realizar la limpieza automática con make clean.[0m"
  exit 1
fi

echo "\n\033[33m[FASE: COMPILACIÓN]\033[0m\n"

# Se puede pasar un argumento. Compruebo si es --debug para entonces activar el modo
# debug (ojo, no -g para insertar símbolos para el gdb etc, si no que es para indicar
# que se procesen las directivas de preprocesamiento, DEBUG activo).
# Por otro lado, si no es --debug, se comprueba si es -x, en cuyo caso ejecutará este script
# en modo verbose (ambos simultaneamente no, que ensucia la terminal bastante).
# Si no se pasa argumento, nada, simplemente ejecuta make.
# Nótese que el makefile por defecto incluye flag -w para evitar mostrar warnings de compilación
if [ "$1" = "--debug" ]; then
    make CFLAGS="-w -DDEBUG"
elif [ "$1" = "-x" ]; then
    set -x
    make
else
    make
fi

# Comprobar si make sido ejecutado correctamente
# TODO: Crear carpeta "logs" y producir un fichero error.log con información.
if [ $? -eq 0 ]
then
  echo "\n\033[32mTarmaggeddon compilado exitosamente.\033[0m"
else
  echo "\n\033[31mError al compilar Tarmaggeddon.[0m\n"
  exit 1
fi

echo "\n\033[33m[FASE: CREACIÓN DE FICHEROS]\033[0m\n"

# Borro restos de tests previos.
rm -rf $PRUEBAS_DIR

echo "Creando el directorio '$PRUEBAS_DIR'...\n"
mkdir "$PRUEBAS_DIR"

# Creo ficheros temporales con texto y sus referencias, .dat y .txt, por cambiar.
# Touch no es necesario, pero así "jugamos" un poco.
touch $STAR_WARS
echo "Que la fuerza te acompañe, Luke Skywalker." > $STAR_WARS

echo "Un Anillo para gobernarlos a todos. Un Anillo para encontrarlos,
      un Anillo para atraerlos a todos y atarlos en las tinieblas." > $LOTR
echo "La fundación de la humanidad se encuentra en manos de Hari Seldon." > $FUNDACION
echo "Winter is coming. La lucha por el trono de hierro ha comenzado." > $GOT
echo "Let me tell you why you're here. You're here because you know something. What you know, you can't explain. But you feel it. You've felt it your entire life. That there's something wrong with the world. You don't know what it is, but it's there. Like a splinter in your mind, driving you mad. It is this feeling that has brought you to me. Do you know what I'm talking about?

      The Matrix.

      Do you want to know what it is?

      The Matrix is everywhere. It is all around us. Even now, in this very room. You can see it when you look out your window or when you turn on your television. You can feel it when you go to work... when you go to church... when you pay your taxes. It is the world that has been pulled over your eyes to blind you from the truth.

      What truth?

      That you are a slave, Neo. Like everyone else, you were born into bondage, born into a prison that you cannot smell or taste or touch. A prison for your mind. Unfortunately, no one can be told what the Matrix is. You have to see it for yourself. This is your last chance. After this, there is no turning back. You take the blue pill - the story ends, you wake up in your bed and believe whatever you want to believe. You take the red pill - you stay in Wonderland and I show you how deep the rabbit-hole goes." > $MATRIX

echo "Nuevo fichero '$STAR_WARS' creado."
echo "Nuevo fichero '$LOTR' creado."
echo "Nuevo fichero '$FUNDACION' creado."
echo "Nuevo fichero '$GOT' creado."
echo "Nuevo fichero '$MATRIX' creado."

# Link Simbólico
echo "\n Rellenando con algo de texto el fichero que va a ser apuntado por un enlace simbolico" > $BASELINK
ln -s "$BASELINK" "$SYMLINK"
echo "\n Creando fichero '$BASELINK' y el symlink '$SYMLINK' que le apunta..."

# Fichero dentro de directorio
mkdir $DIRINSERCION
mkdir $DIRINSERCIONDOS

echo "\n Algo de texto para el fichero que está dentro del directorio." > $DIRINSERCION/$FICHEROENDIR
echo "\n Este es otro fichero el cual se extraera de manera independiente con el fin de que la utilidad cree el directorio donde se encuentra durante la extracción del mismo. Y bueno, ¡pues un poco de humor! Extract-trek ->  Start-Trek (un poco malo, eso sí)." > $DIRINSERCIONDOS/$FICHEROENDIRDOS

echo "\n Creado directorio y ficheros: '$DIRINSERCION/$FICHEROENDIR'"
echo "\n Creado directorio y ficheros: '$DIRINSERCIONDOS/$FICHEROENDIRDOS'"

# Creamos el tar con nuestra utilidad de crear tar.
echo "\n\033[33m[FASE: EMPAQUETANDO TAR]\033[0m\n"
echo "Creando nuevo tar: '$PRUEBAS_DIR/$TAR_BASE ' con fichero base $STAR_WARS...\n"
./$EXEC $CREAR $PRUEBAS_DIR/$TAR_BASE $STAR_WARS

echo "\n\033[33m[FASE: INSERCIÓN EN TAR]\033[0m\n"

# Ejecutar nuestro Tarmaggedon para cada uno de los ficheros creados
./$EXEC $INSERTAR $PRUEBAS_DIR/$TAR_BASE $LOTR
./$EXEC $INSERTAR $PRUEBAS_DIR/$TAR_BASE $MATRIX
./$EXEC $INSERTAR $PRUEBAS_DIR/$TAR_BASE $FUNDACION
./$EXEC $INSERTAR $PRUEBAS_DIR/$TAR_BASE $GOT

# Insertar también el directorio y un fichero que esté situado dentro de un directorio.
./$EXEC $INSERTAR $PRUEBAS_DIR/$TAR_BASE $DIRINSERCION
./$EXEC $INSERTAR $PRUEBAS_DIR/$TAR_BASE $DIRINSERCIONDOS/$FICHEROENDIRDOS

# Por último, insertar el enlace simbólico.
./$EXEC $INSERTAR $PRUEBAS_DIR/$TAR_BASE $SYMLINK

echo "Fichero '$LOTR' insertado en tar $PRUEBAS_DIR/$TAR_BASE."
echo "Fichero '$MATRIX' insertado en tar $PRUEBAS_DIR/$TAR_BASE."
echo "Fichero '$FUNDACION' insertado en tar $PRUEBAS_DIR/$TAR_BASE."
echo "Fichero '$GOT' insertado en tar $PRUEBAS_DIR/$TAR_BASE."

# Limpiamos ficheros.
echo "\nEliminando ficheros temporales: $STAR_WARS $LOTR $MATRIX $FUNDACION $GOT"
rm $STAR_WARS $LOTR $MATRIX $FUNDACION $GOT

echo "\nExtrayendo el contenido de '$PRUEBAS_DIR/$TAR_BASE'...\n"

# Extraemos con utilidad tar el fichero, para comprobar que efectivamente
# su estructura es estándar (GNU TAR) y funciona correctamente y con su
# utilidad oficial.
tar xvf "$PRUEBAS_DIR/$TAR_BASE" -C $PRUEBAS_DIR

echo "\n\033[33m[FASE: VERIFICACIÓN CONTENIDOS - HEXDUMP]\033[0m\n"
hexdump -C "$PRUEBAS_DIR/$TAR_BASE"

# Muestro de manera recursiva el directorio creado.
echo "\nVerificando que los atributos y los datos del fichero extraído son correctos...\n"
ls -alR $PRUEBAS_DIR

# Extraemos todos los ficheros, PERO, uno a uno.
# Nótese que en el extraer_fichero.c podríamos hacer que
# se extrajesen todos los ficheros contenidos en el tar con una pequeña modificación.
echo "\n\033[33m[FASE: EXTRACCIÓN FICHERO]\033[0m\n"
./$EXEC $EXTRAER $PRUEBAS_DIR/$TAR_BASE $STAR_WARS
./$EXEC $EXTRAER $PRUEBAS_DIR/$TAR_BASE $LOTR
./$EXEC $EXTRAER $PRUEBAS_DIR/$TAR_BASE $MATRIX
./$EXEC $EXTRAER $PRUEBAS_DIR/$TAR_BASE $FUNDACION
./$EXEC $EXTRAER $PRUEBAS_DIR/$TAR_BASE $GOT

# Insertar también el directorio y un fichero que esté situado dentro de un directorio.
./$EXEC $EXTRAER $PRUEBAS_DIR/$TAR_BASE $DIRINSERCION
./$EXEC $EXTRAER $PRUEBAS_DIR/$TAR_BASE $DIRINSERCIONDOS/$FICHEROENDIRDOS

# Por último, insertar el enlace simbólico.
./$EXEC $EXTRAER $PRUEBAS_DIR/$TAR_BASE $SYMLINK

echo "Mostrando contenido extraído: \n"
ls -al $EXTRACTS_DIR

# Si la máquina donde se ejecuta el script dispone del comando "tree", lo ejecutamos.
# Proporciona de manera visual una noción bastante buena de la estructura del directorio
# post-ejecución.
if command -v tree >/dev/null 2>&1; then
    echo "\nMostrando árbol de directorio: \n"
    tree
fi

echo "\n (Ejecuta: 'make clean' si deseas limpiar los directorios y symlink creados)"
echo "\n\033[35m[TARMAGGEDDON VERIFICACION FINALIZADA.]\033[0m\n"


