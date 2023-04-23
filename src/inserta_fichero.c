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
#include "s_mytarheader.h"
#include "stdio.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <strings.h>
#include <sys/stat.h>
#include <dirent.h>

unsigned int ret;

/**
 * Función para buscar la posición de inserción
 * que corresponde.
 * @param tar_fd Descriptor de fichero abierto
 * @return -1 si existe algún error. En caso contrario
 *         se devolverá la posición donde se deberá de
 *         insertar el fichero en cuestión.
 */
int search_position(int tar_fd){
    struct c_header_gnu_tar header;
    int header_bytes;
    unsigned long numero_bloques;
    int ins_pos = -1;
    unsigned long fsize;

    ins_pos = (int)lseek(tar_fd, 0, SEEK_CUR);
    header_bytes = (int)read(tar_fd, &header, DATAFILE_BLOCK_SIZE);

    /**
     * Si no hemos podido leer datos del header, o si el campo
     * magic del header es distinto de ustar , devolvemos -1.
     * El tar posee un formato incorrecto.
     */
    if(header_bytes < 0 || (strcmp(header.magic, "ustar  ") != 0)) {
        ins_pos = -1;
    }else{

        while(strcmp(header.magic, "ustar  ") == 0){

            sscanf(header.size, "%011lo", &fsize);

#ifdef DEBUG
            printf("Header tiene: %i\n", header_bytes);
            printf("Magic: %s\n", header.magic);
            printf("Size: %lu\n", fsize);
#endif

            /**
             * Calculamos el número de bloques que debemos de avanzar.
             */
            numero_bloques = fsize / DATAFILE_BLOCK_SIZE;

            if (fsize % DATAFILE_BLOCK_SIZE != 0) {
                numero_bloques++;
            }

#ifdef DEBUG
            printf("Numero de bloques a recorrer: %lu\n", numero_bloques);
#endif
            /**
             * Avanzamos hasta el siguiente header para continuar analizando.
             */
            lseek(tar_fd, (off_t)(numero_bloques)*DATAFILE_BLOCK_SIZE, SEEK_CUR);

            /**
             * Guardamos la posición actual, en caso de que el siguiente header
             * no cumpla los requisitos, sera ésta la posición donde el fichero
             * se tendrá que insertar.
             */
            ins_pos = (int)lseek(tar_fd, 0, SEEK_CUR);
            header_bytes = (int)read(tar_fd, &header, DATAFILE_BLOCK_SIZE);

            if(header_bytes < 0)
                return E_TARFORM;
        }
    }

    return ins_pos;
}

/**
 * Función encarga de procesar la inserción de un fichero dado
 * al final de un tar.
 * @param file Fichero en cuestión que se tendrá que insertar.
 * @param tar Ruta al tar donde se tendrá que insertar el fichero.
 * @return Se devolverá 0 en caso de no haber ningún error. En caso
 *         contrario se devolverá el número de error para ser procesado.
 */
int insert_file(char* file, char* tar){
    struct c_header_gnu_tar header;
    int tar_fd, dat_fd;
    int ins_pos;
    struct stat st;
    char link_target[PATH_MAX];
    struct dirent* entry;
    char* file_path;
    DIR* dir;
    unsigned long tar_size = 0;
    ssize_t bytes_used = 0;

    tar_fd = open(tar, O_RDWR);

    if (tar_fd < 0) {
        close(tar_fd);
        return E_OPEN2;
    }

    if(lstat(file, &st) < 0) {
        close(tar_fd);
        return E_OPEN1;
    }

    ins_pos = search_position(tar_fd);
    tar_size = ins_pos;

    if (ins_pos == -1) {
        CreateNewTar(file, tar);
    } else{
        if (S_ISDIR(st.st_mode)) {

            lseek(tar_fd, ins_pos, SEEK_SET);
            memset(&header, 0, sizeof(struct c_header_gnu_tar));
            BuildTarHeader(file, &header);

            bytes_used = write(tar_fd, &header, sizeof(struct c_header_gnu_tar));
            tar_size += bytes_used;

            dir = opendir(file);

            if(dir == NULL) {
                close(tar_fd);
                return E_OPEN1;
            }

            while((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_REG) {
                    file_path = (char *)malloc(strlen(file) + strlen(entry->d_name) + 2);
                    strcpy(file_path, file);
                    strcat(file_path, "/");
                    strcat(file_path, entry->d_name);

                    dat_fd = open(file_path, O_RDONLY);
                    if(dat_fd < 0) {
                        close(tar_fd);
                        closedir(dir);
                        free(file_path);
                        return E_OPEN1;
                    }

                    tar_size += WriteFile(dat_fd, tar_fd, file_path, tar);
                    close(dat_fd);
                    free(file_path);
                }
            }

            tar_size = WriteEndTarArchive(tar_fd);
            tar_size = WriteCompleteTarSize(tar_size, tar_fd);
            closedir(dir);

        }else{ // Si es un archivo regular
            dat_fd = open(file, O_RDONLY);
            if (dat_fd < 0) {
                close(tar_fd);
                close(dat_fd);
                return E_OPEN1;
            }

            lseek(tar_fd, ins_pos, SEEK_SET);
            tar_size = WriteFile(dat_fd, tar_fd, file, tar);
            tar_size = WriteEndTarArchive(tar_fd);
            tar_size = WriteCompleteTarSize(tar_size, tar_fd);
            close(dat_fd);
        }
    }

    close(tar_fd);
    return 0;
}