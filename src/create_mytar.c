/* *
 * * @file create_mytar.c
 * * @author G.A.
 * * @date 10/02/2023
 * * @brief First version of mytar
 * * @details  Create a tar file with only one "data file"
 * *
 * */
#include <unistd.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <strings.h>
#include <stdlib.h>

#include "s_mytarheader.h"


// #define ERROR_OPEN_DAT_FILE (2)
// #define ERROR_OPEN_TAR_FILE (3)
// #define ERROR_GENERATE_TAR_FILE (4)
// #define ERROR_GENERATE_TAR_FILE2 (5)


// #define HEADER_OK (1)
// #define HEADER_ERR (2)

// #define FILE_HEADER_SIZE     512
// #define DATAFILE_BLOCK_SIZE  512
// #define END_TAR_ARCHIVE_ENTRY_SIZE  (512*2)
// #define TAR_FILE_BLOCK_SIZE  ((unsigned long) (DATAFILE_BLOCK_SIZE*20))

struct passwd *pws;
struct group  *grp;

char FileDataBlock[DATAFILE_BLOCK_SIZE];


//-----------------------------------------------------------------------------
// Return UserName (string) from uid (integer). See man 2 stat and man getpwuid
char * getUserName(uid_t uid)
{
    pws = getpwuid(uid);
    return pws->pw_name;
}

//------------------------------------------------------------------------------
// Return GroupName (string) from gid (integer). See man 2 stat and man getgrgid
char * getGroupName(gid_t gid)
{
    grp = getgrgid(gid);
    return grp->gr_name;
}

//----------------------------------------
// Return Mode type entry  (tar Mode type)
char mode_tar( mode_t Mode)
{
    if  (S_ISREG(Mode))  return  '0';

    if  (S_ISLNK(Mode))  return  '2';
    if  (S_ISCHR(Mode))  return  '3';
    if  (S_ISBLK(Mode))  return  '4';
    if  (S_ISDIR(Mode))  return  '5';
    if  (S_ISFIFO(Mode))  return  '6';
    if  (S_ISSOCK(Mode))  return  '7';
    return '0';
}


// ------------------------------------------------------------------------
// (1.0) Build my_tardat structure with FileName stat info (See man 2 stat)
int BuildTarHeader(char *FileName, struct c_header_gnu_tar *pTarHeader)
{
    struct stat  stat_file;

    ssize_t  Symlink_Size;
    int n, i;
    char  *pTarHeaderBytes;
    unsigned int  Checksum;
    char link_target[PATH_MAX];

    bzero(pTarHeader, sizeof(struct c_header_gnu_tar));

    if (lstat (FileName, &stat_file) == -1)  return HEADER_ERR;

    strcpy(pTarHeader->name, FileName);
    sprintf(pTarHeader->mode, "%07o", stat_file.st_mode & 07777);  // Only  the least significant 12 bits
#ifdef DEBUG
    printf("st_mode del archivo %s %07o\n",FileName, stat_file.st_mode & 07777); // Only  the least significant 12 bits
#endif
    sprintf(pTarHeader->uid, "%07o", stat_file.st_uid);
    sprintf(pTarHeader->gid, "%07o", stat_file.st_gid);
    sprintf(pTarHeader->size, "%011lo", stat_file.st_size);
    sprintf(pTarHeader->mtime, "%011lo", stat_file.st_mtime);
    // checksum  the last     sprintf(pTarHeader->checksum, "%06o", Checksum);

    pTarHeader->typeflag[0] = mode_tar(stat_file.st_mode);

    //  Symlinks.
    if(S_ISLNK(stat_file.st_mode)){
        ssize_t link_len = readlink(FileName, link_target, sizeof(link_target) - 1);
        link_target[link_len] = '\0';
        strncpy(pTarHeader->linkname, link_target, sizeof(pTarHeader->linkname) - 1);
        sprintf(pTarHeader->size, "%011o", 0);
        pTarHeader->typeflag[0] = '2';
    } else if(S_ISDIR(stat_file.st_mode)){
        sprintf(pTarHeader->size, "%011o", 0);
    }

    strncpy(pTarHeader->magic, "ustar ",6);  // "ustar" followed by a space (without null char)
    strcpy(pTarHeader->version, " ");  //   space character followed by a null char.
    strcpy(pTarHeader->uname, getUserName(stat_file.st_uid));
    strcpy(pTarHeader->gname, getGroupName(stat_file.st_gid));
    //  devmayor (not used)
    //  devminor (not used)
    sprintf(pTarHeader->atime, "%011lo", stat_file.st_atime);
    sprintf(pTarHeader->ctime, "%011lo", stat_file.st_ctime);
    //  offset (not used)
    //  longnames (not used)
    //  unused (not used)
    //  sparse (not used)
    //  isextended (not used)
    //  realsize (not used)
    //  pad (not used)

    // compute checksum (the last)
    memset(pTarHeader->checksum, ' ', 8);   // Initialize to blanc spaces
    pTarHeaderBytes = (unsigned char *) pTarHeader;

    for (i=0,Checksum=0 ; i < sizeof(struct c_header_gnu_tar); i++)
        Checksum = Checksum + pTarHeaderBytes[i];

    sprintf(pTarHeader->checksum, "%06o", Checksum);    // six octal digits followed by a null and a space character

    return HEADER_OK;
}


// ----------------------------------------------------------------
// (1.2) write the data file (blocks of 512 bytes)
unsigned long WriteFileDataBlocks(int fd_DataFile, int fd_TarFile)
{
    unsigned long NumWriteBytes;
    int Tam, n;

    // write the data file (blocks of 512 bytes)
    NumWriteBytes = 0;
#ifdef DEBUG
    printf("Datos Escritos: ");   // Traza
#endif
    bzero(FileDataBlock, sizeof(FileDataBlock));
    // Se lee el archivo bloque a bloque (cada uno, 512 bytes).
    // Cada vez que se lee un bloque, se escribe al fichero destino.
    while((n = read(fd_DataFile,  FileDataBlock, sizeof(FileDataBlock))) > 0)
    {
        write(fd_TarFile, FileDataBlock, sizeof(FileDataBlock));
        NumWriteBytes = NumWriteBytes + sizeof(FileDataBlock);
#ifdef DEBUG
        printf("-- %d --", n); // Traza
#endif
        // Se limpia el búffer antes de la siguiente iteración.
        bzero(FileDataBlock, sizeof(FileDataBlock));
    }
#ifdef DEBUG
    printf("\nTotal: Escritos %ld \n", NumWriteBytes); // Traza
#endif
    return NumWriteBytes;
}

// ----------------------------------------------------------------
// (2.1)Write end tar archive entry (2x512 bytes with zeros)
unsigned long WriteEndTarArchive( int fd_TarFile)
{
    unsigned long NumWriteBytes;
    char buffer[END_TAR_ARCHIVE_ENTRY_SIZE];
    int n;

    // write end tar archive entry (2x512 bytes with zeros)
    NumWriteBytes = 0;
    bzero(buffer, END_TAR_ARCHIVE_ENTRY_SIZE);
    write(fd_TarFile, buffer, END_TAR_ARCHIVE_ENTRY_SIZE);
    NumWriteBytes += END_TAR_ARCHIVE_ENTRY_SIZE;

    n = NumWriteBytes/DATAFILE_BLOCK_SIZE;
#ifdef DEBUG
    printf("Escritos (End block): %d blocks total bytes %ld\n", n, NumWriteBytes); // Traza
#endif
    return NumWriteBytes;
}

// ----------------------------------------------------------------
// (2.2) complete Tar file to  multiple of 10KB size block
unsigned long WriteCompleteTarSize(unsigned long TarActualSize,  int fd_TarFile)
{
    unsigned long NumWriteBytes;
    unsigned long offset;
    char* buffer;

    NumWriteBytes = TarActualSize;

    if(TAR_FILE_BLOCK_SIZE > NumWriteBytes){
        offset = TAR_FILE_BLOCK_SIZE - NumWriteBytes;
    }else if(TAR_FILE_BLOCK_SIZE < NumWriteBytes){
        offset = TAR_FILE_BLOCK_SIZE - (NumWriteBytes % TAR_FILE_BLOCK_SIZE);
    }else
        offset = 0;

    // complete to  multiple of 10KB size blocks
#ifdef DEBUG
    printf("TAR_FILE_BLOCK_SIZE=%ld  TarFileSize=%ld  Offsset=%ld\n", TAR_FILE_BLOCK_SIZE, NumWriteBytes, offset);
#endif
    if(offset > 0 ){
        buffer = malloc(offset);
        bzero(buffer, offset);
        write(fd_TarFile, buffer, offset);
        free(buffer);
    }
#ifdef DEBUG
    printf("OK: Generado el EndTarBlocks del archivo tar %ld bytes \n", offset); // Traza
#endif
    return offset;
}

// Verify Tar file zize to  multiple of 10KB size blocks
int VerifyCompleteTarSize(unsigned long TarActualSize)
{
    // Verify
    if ((TarActualSize % TAR_FILE_BLOCK_SIZE) != 0)
    {
        fprintf(stderr,"Error al generar el fichero tar. Tamanio erroneo %ld\n", TarActualSize);
        return ERROR_GENERATE_TAR_FILE2;
    }
    return 0;
}

int WriteFile(int fd_DatFile, int fd_TarFile, char* FileName, char* TarFileName){
    struct c_header_gnu_tar my_tardat;
    int TarFileSize = 0;
    unsigned long fsize = 0;
    long mode_long;

    bzero(&my_tardat, sizeof(struct c_header_gnu_tar));

    // (1.0) Build my_tardat structure with FileName stat info (See man 2 stat)
    if(BuildTarHeader(FileName, &my_tardat) == HEADER_ERR){
        fprintf(stderr, "Ha habido un problema al construir los headers para el fichero: %s\n", FileName);
        close(fd_DatFile);
        return ERROR_GENERATE_TAR_FILE;
    }

    my_tardat.mode[sizeof(my_tardat.mode) - 1] = '\0';
    mode_long = strtol(my_tardat.mode, NULL, 8);
    mode_t mode = (mode_t) mode_long;

    // ----------------------------------------------------------------
    // (1.1)Write  tar header(of FileName) to the tar file (TarFileName)
    TarFileSize = write(fd_TarFile, &my_tardat, sizeof(struct c_header_gnu_tar));

    if(TarFileSize == -1){
        close(fd_DatFile);
        return ERROR_GENERATE_TAR_FILE;
    }

    // ----------------------------------------------------------------
    // (1.2) write the data file (blocks of 512 bytes)
    sscanf(my_tardat.size, "%011lo", &fsize);
    if(fsize > 0)
        TarFileSize += WriteFileDataBlocks(fd_DatFile, fd_TarFile);
#ifdef DEBUG
    printf("TarFileSize after WriteFileDataBlocks: %ld \n ", TarFileSize);
#endif
    return TarFileSize;
}

int CreateNewTar(char* FileName, char* TarFileName){

    unsigned long TarFileSize, n, offset;
    ssize_t  Symlink_Size;

    char FileDataBlock[DATAFILE_BLOCK_SIZE];

    int i, ret, Tam;

    int fd_DatFile, fd_TarFile;
    char* buffer;

    TarFileSize = 0;

    if ((fd_DatFile=open(FileName, O_RDONLY))== -1)
    {
        fprintf(stderr, "No se puede abrir el fichero de datos %s\n", FileName);
        return ERROR_OPEN_DAT_FILE;
    }

    if ((fd_TarFile=open(TarFileName, O_WRONLY | O_CREAT | O_TRUNC, 0600))== -1)
    {
        fprintf(stderr, "No se puede abrir el fichero tar %s\n", TarFileName);
        close(fd_DatFile);
        return ERROR_OPEN_TAR_FILE;
    }

    TarFileSize += WriteFile(fd_DatFile, fd_TarFile, FileName, TarFileName);
    TarFileSize += WriteEndTarArchive(fd_TarFile);
    TarFileSize += WriteCompleteTarSize(TarFileSize, fd_TarFile);

    // -------------------------------------------------------------
    // (3) Verify Tar file zize to  multiple of 10KB size block
    ret =  VerifyCompleteTarSize(TarFileSize);
    if (ret == ERROR_GENERATE_TAR_FILE2)
    {
        fprintf(stderr,"Error al generar el fichero tar %s. Tamanio erroneo %ld\n", TarFileName, TarFileSize);
        close(fd_DatFile);
        close(fd_TarFile);
        return ERROR_GENERATE_TAR_FILE2;
    }
#ifdef DEBUG
    printf("OK: Generado el fichero tar %s (size=%ld) con el contenido del archivo %s. \n", TarFileName, TarFileSize, FileName);
#endif
    close(fd_DatFile);
    close(fd_TarFile);

    return 0;
}