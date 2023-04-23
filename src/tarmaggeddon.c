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
#include <stdlib.h>
#include "s_mytarheader.h"

/**
 * Argumento 1: Acción [inserción/extracción]
 * Argumento 2: Path a fichero.
 * Argumento 3: Path a tar.
 */
int main(int argc, char* argv[]){

    int action = -1;
    char* tar_name;
    char* file_name;
    int output = 0;

    if(argc == 4){
        action = atoi(argv[1]);
        tar_name = argv[2];
        file_name = argv[3];
        switch(action){
            case ACTION_CREATE:
                output = CreateNewTar(file_name, tar_name);
                break;
            case ACTION_INSERT:
                output = insert_file(file_name, tar_name);
                break;
            case ACTION_EXTRACT:
                output = extract_file(file_name, tar_name);
                break;
            default:
                break;
        }
    } else{
        return -1;
    }

    switch(output){
        case E_OPEN1:
            perror("Error abriendo el fichero DAT");
            return output;
        case E_OPEN2:
            perror("Error abriendo fichero TAR");
            return output;
        default:
            return 0;
    }
}