/*
 * This file is part of the Sistemas Operativos project.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @authors - Geru-Scotland (https://github.com/geru-scotland)
 *          - Farolei (https://github.com/farolei)
 *          - UnaiAD22 (https://github.com/UnaiAD22)
 */
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h>
#include <libgen.h>
#include "s_mytarheader.h"

/**
 * Función que construye un path o ruta.
 * @param base_path Parámetro pasado por referencia que define
 *        la ruta base. Se modificará alojando así la ruta final
 *        tal que: ruta-base/nombrefichero
 * @param file_name Nombre del fichero que se agregará a la ruta base.
 */
void build_path(char* base_path, const char* file_name){
    int length = 0;
    int i = 0;
    /**
     * Hallamos el tamaño de la ruta base.
     * Podíamos haberlo calculado también con:
     * length = strlen(DIR_PREFIX);
     * Pero vamos a generalizarlo mejor.
     */
    while (base_path[length] != '\0') {
        length++;
    }

    /**
     * Agregamos el nombre del fichero a la ruta base.
     */
    while(file_name[i] != '\0'){
        base_path[length] = file_name[i];
        length++;
        i++;
    }

    /**
     * Importante en toda string, terminar siempre con caracter nulo.
     * Si utilizamos después funciones de strings, es fundamental.
     */
    base_path[length] = '\0';
}

/**
 * Función encargada de extraer un fichero de un tar dado. Creará un
 * directorio en la ruta actual, definido por DIR_PREFIX.
 * @param file nombre del fichero a extraer.
 * @param tar ruta del fichero tar donde buscar el fichero y que será
 *        extraído en caso de ser encontrado.
 * @return Se devolverá 0 en caso de no haber ningún error. En caso
 *        contrario se devolverá el número de error para ser procesado.
 */
int extract_file(char* file, char* tar) {
    struct c_header_gnu_tar header;
    unsigned long numero_bloques;
    unsigned long fsize;
    char name[100];
    int tar_fd;
    int bytes_read;
    int new_file_fd;
    char buffer[DATAFILE_BLOCK_SIZE];
    char path[100] = DIR_PREFIX;
    char dir_path[100] = DIR_PREFIX;
    char name_copy[PATH_MAX];
    char* dir_name;
    /**
     * Declaro indices de bucle aquí; en máquinas personales no hay problemas,
     * pero sí en el servidor de la universidad. Tras investigar un poco, el
     * declarar variables/indices en la definición del bucle for, es una
     * característica introducida con C99, a partir de la versión 4.0.0 de gcc.
     */
    int i, j = 0;

    tar_fd = open(tar, O_RDONLY);

    if(tar_fd < 0) {
        close(tar_fd);
        printf("Error en tar: %s", tar);
        return E_OPEN2;
    }

    bytes_read = (int)read(tar_fd, &header, DATAFILE_BLOCK_SIZE);

    /**
     * Si no hemos podido leer datos del header, o si el campo
     * magic del header es distinto de ustar , abortamos. El
     * tar posee un formato incorrecto.
     */
    if(bytes_read < 0 || (strcmp(header.magic, "ustar  ") != 0)) {
        perror("[TAR] Header con formato incorrecto.");
        close(tar_fd);
        return E_OPEN2;
    } else{
        mkdir(DIR_NAME, 0777);

        while(strcmp(header.magic, "ustar  ") == 0) {
            sscanf(header.name, "%s", name);
            sscanf(header.size, "%011lo", &fsize);

            numero_bloques = fsize / DATAFILE_BLOCK_SIZE;

            if(fsize % DATAFILE_BLOCK_SIZE != 0) {
                numero_bloques++;
            }

            /**
             * Si encontramos el fichero dentro del tar, procedemos
             * a la creación de uno nuevo que vaya a contener el mismo
             * contenido.
             */
            if(strcmp(name, file) == 0) {
                build_path(path, name);

                if(header.typeflag[0] == '5') {
                    mkdir(path, 0777);
                } else if(header.typeflag[0] == '2') { // Si es un enlace simbólico
                    symlink(header.linkname, path);
                }else {
                    /*
                     * Si es un archivo regular.
                     * Crear directorio padre si no existe.
                     */
                    strncpy(name_copy, name, PATH_MAX);
                    dir_name = dirname(name_copy);
                    build_path(dir_path, dir_name);

                    if(access(dir_path, F_OK) != 0) {
                        mkdir(dir_path, 0777);
                    }

                    new_file_fd = open(path, O_RDWR | O_CREAT, 0777);

                    if(new_file_fd < 0) {
                        close(tar_fd);
                        close(new_file_fd);
                        return E_OPEN1;
                    }

                    /**
                     * Escribimos todos los bloques de datos. Para el último
                     * de ellos, existe la posibilidad de que únicamente un
                     * determinado número de bytes (< 512) contenga datos útiles.
                     *
                     * Ergo, el resto serán caracter nulos ya que los bloques de
                     * datos fueron rellenados con buffers inicializados a \0.
                     */
                    for(i = 0; i < numero_bloques; i++) {
                        bytes_read = read(tar_fd, buffer, DATAFILE_BLOCK_SIZE);

                        for(j = 0; j < bytes_read; j++) {
                            if (buffer[j] == '\0')
                                break;
                            write(new_file_fd, &buffer[j], sizeof(char));
                        }
                    }
                    // Recuperar los permisos originales para archivos y directorios
                    mode_t file_mode;
                    sscanf(header.mode, "%ho", &file_mode);
                    chmod(path, file_mode);
                    close(new_file_fd);
                }

                break;
            }else {
                /**
                 * En caso de que el header no contenga el fichero que
                 * estamos buscando, avanzamos hasta el siguiente header
                 * directamente.
                 */
                lseek(tar_fd, (off_t)(numero_bloques)*DATAFILE_BLOCK_SIZE, SEEK_CUR);
            }

            bytes_read = (int)read(tar_fd, &header, DATAFILE_BLOCK_SIZE);

            if(bytes_read < 0)
                return E_TARFORM;
        }
    }

    close(tar_fd);
    return 0;
}