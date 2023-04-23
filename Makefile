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

CC = gcc
# Ojo, he agregado -w para desactivar warnings.
# Modo debug, lo he preparado para poder activar/desactivar con
# argumento desde script.
CFLAGS = -w
SRC = src/create_mytar.c src/inserta_fichero.c src/extraer_fichero.c src/tarmaggeddon.c
EXEC = tarmaggeddon
CREATE_TESTS = create-tests
TEST_DIR = tarmaggeddon-tests
EXTRACT_DIR = extracts
DIR_INS = enterprise real-enterprise
SYMLINK = beam-me-up-lnk beam-me-up.dat

$(EXEC): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(EXEC)

clean:
	rm -f $(EXEC)
	rm -rf $(CREATE_TESTS)
	rm -rf $(TEST_DIR)
	rm -rf $(EXTRACT_DIR)
	rm -rf $(DIR_INS)
	rm -f $(SYMLINK)

#run:
#	./$(EXEC)