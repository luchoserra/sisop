/* Cosas asumidas inicio*/
#define FUSE_USE_VERSION 30
#define DIR -1
#define FILE_T 1
#define MAX_CONTENT 1024
#define MAX_DIRECTORY_SIZE 1024
#define MAX_INODES 80
#define MAX_PATH 200
#define FREE 0
#define OCCUPIED 1
#define ROOT_PATH "/"

#include <fuse.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

// enum inode_type { INODE_FILE, INODE_DIR };

struct inode {
	int type;                  // -1 = directorio, 1 = archivo
	// enum inode_type type;
	mode_t mode;               // permissions
	size_t size;               // size of the file
	uid_t uid;                 // user id
	gid_t gid;                 // group id
	time_t last_access;        // last access time
	time_t last_modification;  // last modification time
	time_t creation_time;      // creation time
	char path[MAX_PATH];
	char content[MAX_CONTENT];      // content of the file
	char directory_path[MAX_PATH];  // path of the directory that contais
	                                // the file. if it's a directory, it's
	                                // the path of the parent directory if
	                                // it's the root, it's an empty string
};

struct super_block {
    struct inode inodes[MAX_INODES];
    int bitmap_inodes[MAX_INODES];  // 0 = libre, 1 = ocupado
};

struct super_block super = {};

// Donde se va a guardar el fs
char fs_file[MAX_PATH] = "fs.fisopfs";

/* AUXILIARES */

char *
remove_slash(const char *path)
{
	size_t len = strlen(path);
	char *path_without_root_slash = malloc(len);
	if (!path_without_root_slash)
		return NULL;

	memcpy(path_without_root_slash, path + 1, len - 1);
	path_without_root_slash[len - 1] = '\0';

	const char *last_slash = strrchr(path, '/');
	if (last_slash == NULL)
		return path_without_root_slash;

	size_t absolute_len = strlen(last_slash + 1);
	char *absolute_path = malloc(absolute_len + 1);
	if (!absolute_path) {
		free(path_without_root_slash);
		return NULL;
	}

	memcpy(absolute_path, last_slash + 1, absolute_len);
	absolute_path[absolute_len] = '\0';

	free(path_without_root_slash);

	return absolute_path;
}

void
get_parent_path(char *parent_path)
{
	char *last_slash = strrchr(parent_path, '/');

	if (last_slash != NULL)
		*last_slash = '\0';
	else
		parent_path[0] = '\0';
}

const char* 
remove_slashh(const char* path) 
{
    if (!path || *path == '\0') {
        return NULL; // Path inválido
    }

    const char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        return path;
    }

    return last_slash + 1; // Devolver el nombre del archivo o directorio
}

int 
next_free_inode_index(const char *path)
{
    for (int i = 0; i < MAX_INODES; i++) {
        // Si el inodo ya existe, devolver error
        if (strcmp(super.inodes[i].path, path) == 0) {
			errno = EEXIST;
			fprintf(stderr, "[debug] Error next_free_inode_index: %s\n", strerror(errno));
            return -EEXIST;
        }
        // Si se encuentra un inodo libre, devolver el índice
        if (super.bitmap_inodes[i] == FREE) {
            return i;
        }
    }

    // No se encontró un inodo libre
	errno = ENOSPC;
	fprintf(stderr, "[debug] Error next_free_inode_index: %s\n", strerror(errno));
    return -ENOSPC;
}

int 
new_inode(const char *path, mode_t mode, int type)
{
	if (strlen(path) - 1 > MAX_CONTENT) {
        fprintf(stderr, "[debug] Error new_inode: %s\n", strerror(errno));
        errno = ENAMETOOLONG;
        return -ENAMETOOLONG;
    }
    
    // Usar remove_slashh en vez de remove_slash
    char *absolute_path = remove_slashh(path);
    if (!absolute_path) {
        return -1;
    }

    int i = next_free_inode_index(absolute_path);
    if (i < 0) {
        return i;
    }

    struct inode new_inode = {
        .type = type,
        .mode = mode,
        .size = 0,
        .uid = getuid(),
        .gid = getgid(),
        .last_access = time(NULL),
        .last_modification = time(NULL),
        .creation_time = time(NULL),
        .content = {0},
    };
    strcpy(new_inode.path, absolute_path);

    if (type == FILE_T) {
        char parent_path[MAX_PATH];
        memcpy(parent_path, path + 1, strlen(path) - 1);
        parent_path[strlen(path) - 1] = '\0';

        get_parent_path(parent_path);

        if (strlen(parent_path) == 0) {
            strcpy(parent_path, ROOT_PATH);
        }

        strcpy(new_inode.directory_path, parent_path);

    } else {
        strcpy(new_inode.directory_path, ROOT_PATH);
    }

    super.inodes[i] = new_inode;
    super.bitmap_inodes[i] = OCCUPIED;

    return 0;
}

struct inode* find_inode(const char* path) {
    // Validar entrada
    if (!path || strcmp(path, "") == 0) {
        return NULL; // Path inválido
    }

    // Manejar el caso especial del directorio raíz
    if (strcmp(path, ROOT_PATH) == 0) {
        return &super.inodes[0]; // El primer inodo es el root
    }

    // Obtener el nombre del archivo/directorio sin slashes
    const char* name = remove_slashh(path);
    if (!name) {
        return NULL; // Error procesando el path
    }

    // Buscar el inodo en el super block
    for (int i = 0; i < MAX_INODES; i++) {
        // Verificar si el inodo está ocupado en el bitmap
        if (super.bitmap_inodes[i] == 1) {
            // Comparar el nombre del archivo/directorio
            if (strcmp(name, super.inodes[i].path) == 0) {
                return &super.inodes[i]; // Retornar puntero al inodo encontrado
            }
        }
    }

    // Si no se encuentra, retornar NULL
    return NULL;
}

int
get_inode_index(const char *path)
{
	// Validar entrada
    if (!path || strcmp(path, "") == 0) {
        return -1; // Path inválido
    }

    // Manejar el caso especial del directorio raíz
    if (strcmp(path, ROOT_PATH) == 0) {
        return 0; // El primer inodo es el root
    }

    // Obtener el nombre del archivo/directorio sin slashes
    const char* name = remove_slashh(path);
    if (!name) {
        return -1; // Error procesando el path
    }

    // Buscar el inodo en el super block
    for (int i = 0; i < MAX_INODES; i++) {
        // Verificar si el inodo está ocupado en el bitmap
        if (super.bitmap_inodes[i] == 1) {
            // Comparar el nombre del archivo/directorio
            if (strcmp(name, super.inodes[i].path) == 0) {
                return i; // Retornar puntero al inodo encontrado
            }
        }
    }

    // Si no se encuentra, retornar NULL
    return -1;
}

// validacion para ver si ese inodo es hijo del directorio
static int is_child_inode(const struct inode *dir_inode, const struct inode *child_inode) {
    return strcmp(child_inode->directory_path, dir_inode->path) == 0;
}

/* */

int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	struct inode* inode = find_inode(path);
	if (!inode) {
		fprintf(stderr, "[debug] Error utimens: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	inode->last_access = tv[0].tv_sec;
	inode->last_modification = tv[1].tv_sec;
	return 0;
}

static int
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_touch - path: %s\n", path);

	return new_inode(path, mode, FILE_T);
}

static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	int index_inodo = get_inode_index(path);
	if (index_inodo == -1) {
		fprintf(stderr, "[debug] Getattr error: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode inode = super.inodes[index_inodo];

	st->st_dev = 0;
	st->st_ino = index_inodo;
	st->st_uid = inode.uid;
	st->st_mode = inode.mode;
	st->st_atime = inode.last_access;
	st->st_mtime = inode.last_modification;
	st->st_ctime = inode.creation_time;
	st->st_size = inode.size;
	st->st_gid = inode.gid;
	st->st_nlink = 2;
	st->st_mode = __S_IFDIR | 0755;
	if (inode.type == FILE_T) {
		st->st_mode = __S_IFREG | 0644;
		st->st_nlink = 1;
	}

	return 0;
}

/* Read size bytes from the given file into the buffer buf, beginning offset bytes into the file. See read(2) for full details. Returns the number of bytes transferred, or 0 if offset was at or beyond the end of the file. Required for any sensible filesystem.
	cat, head, less. */
static int
fisopfs_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi){
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n", path, offset, size);

	if (offset < 0 || size < 0) {
		fprintf(stderr, "[debug] Error read: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}

	struct inode* inode = find_inode(path);
	if (!inode) {
		fprintf(stderr, "[debug] Error read: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	if (inode->type == DIR) {
		fprintf(stderr, "[debug] Error read: %s\n", strerror(errno));
		errno = EISDIR;
		return -EISDIR;
	}

	if (offset >= inode->size) {
        return 0; // Fin del archivo (EOF)
    }

	size_t bytes_to_read = size;
    if (offset + size > inode->size) {
        bytes_to_read = inode->size - offset;
    }

    memcpy(buf, inode->content + offset, bytes_to_read);

    inode->last_access = time(NULL);

    return bytes_to_read;
}

static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink(%s)\n", path);
	
	int index = get_inode_index(path);
	if (index == -1) {
		fprintf(stderr, "[debug] Error unlink: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	struct inode *inode = &super.inodes[index];

	// me fijo si ese nodo ya esta FREE, entonces no hace falta el unlink
	if (inode->type ==FREE){
		fprintf(stderr,"[debug] Error unlink: already free");
		return -ENOENT;
	}

	if (inode->type == DIR) {
		fprintf(stderr, "[debug] Error unlink: %s\n", strerror(errno));
		errno = EISDIR;
		return -EISDIR;
	}

	// primero limpio el inodo y despues lo libero
	memset(inode, 0, sizeof(struct inode));
	super.bitmap_inodes[index] = FREE;

	return 0;
}

int
fisopfs_write(const char *path,const char *data,size_t size_data,off_t offset,struct fuse_file_info *fuse_info)
{
	printf("[debug] fisops_write (%s) \n", path);
	
	if (offset + size_data > MAX_CONTENT) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		errno = EFBIG;
		return -EFBIG;
	}

	int inode_index = get_inode_index(path);
	if (inode_index < 0) {  
		int result = fisopfs_create(path, 33204, fuse_info);
		if (result < 0)
			return result;
		inode_index = get_inode_index(path);
	}

	if (inode_index < 0) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode *inode = &super.inodes[inode_index];
	
	if (inode->size < offset) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}

	if (inode->type != FILE_T) {
		fprintf(stderr, "[debug] Error write: %s\n", strerror(errno));
		errno = EACCES;
		return -EACCES;
	}

	memcpy(inode->content + offset, data, size_data);
	inode->last_access = time(NULL);
	inode->last_modification = time(NULL);
	
	inode->size = strlen(inode->content);
	inode->content[inode->size] = '\0';

	return (int) size_data;
}

static int
fisopfs_readdir(const char *path,
                void *buffer,
                fuse_fill_dir_t filler,
                off_t offset,
                struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_readdir(%s)\n", path);

	filler(buffer, ".", NULL, 0);
	filler(buffer, "..", NULL, 0);
	
	struct inode* inode = find_inode(path);
	if (!inode) {
		fprintf(stderr, "[debug] Error read: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	if (inode->type != DIR) {
		fprintf(stderr, "[debug] Error readdir: %s\n", path);
		return -ENOTDIR;
	}
	
	inode->last_access = time(NULL);

	for (int i = 1; i < MAX_INODES; i++) {
		if (super.bitmap_inodes[i] == OCCUPIED &&
            is_child_inode(&inode, &super.inodes[i])) {
			
				filler(buffer, super.inodes[i].path, NULL, 0);
			
		}
	}

	return 0;
}

static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);

	return new_inode(path, mode, DIR);
}

struct inode **
files_in_dir(const char *path_dir, int *nfiles)
{
	int tope = 0;
	struct inode **files = malloc(MAX_INODES * sizeof(struct inode *));
	char *path_without_root_slash = remove_slash(path_dir);
	if (!path_without_root_slash)
		return NULL;

	for (int i = 0; i < MAX_INODES; i++) {
		if (strcmp(super.inodes[i].directory_path,
		           path_without_root_slash) == 0) {
			files[tope++] = &super.inodes[i];
		}
	}
	free(path_without_root_slash);

	*nfiles = tope;
	return files;
}

static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);

	int index = get_inode_index(path);
	if (index < 0) {
        fprintf(stderr, "[debug] rmdir failed: path not found (%s)\n", strerror(ENOENT));
        errno = ENOENT;
		return -ENOENT;
    }
	struct inode *inode = &super.inodes[index];
	
	if (inode->type != DIR) {
		fprintf(stderr, "[debug] rmdir failed: not a directory (%s)\n", strerror(ENOTDIR));
		errno = ENOTDIR;
		return -ENOTDIR;
	}
	
	int nfiles = 0;
	struct inode **files = files_in_dir(path, &nfiles);
	if (!files) {
		fprintf(stderr, "[debug] rmdir failed: memory allocation error (%s)\n", strerror(ENOMEM));
		errno = ENOMEM;
		return -ENOMEM;
	}
	free(files);
	
	if (nfiles > 0) {
		fprintf(stderr, "[debug] rmdir failed: directory not empty (%s)\n", strerror(ENOTEMPTY));
		errno = ENOTEMPTY;
		return -ENOTEMPTY;
	}
	super.bitmap_inodes[index] = FREE;
	memset(inode, 0, sizeof(struct inode));
	return 0;
}

// Inicializa el sistema de archivos cuando no existe el archivo fs.fisopfs en
// el cual se guardan los datos del sistema de archivos. Crea dicho archivo e
// inicializa el superbloque y el directorio raiz.

static int fisopfs_init(struct fuse_conn_info *conn, struct fuse_config *cfg) {
    // Inicializar el superbloque
    memset(&super, 0, sizeof(struct super_block));

    // Crear el directorio raíz
    struct inode root_inode = {
        .type = DIR,
        .mode = __S_IFDIR | 0755,  // Permisos de directorio estándar
        .size = 0,
        .uid = getuid(),  // ID de usuario actual
        .gid = getgid(),  // ID de grupo actual
        .last_access = time(NULL),
        .last_modification = time(NULL),
        .creation_time = time(NULL),
        .path = ROOT_PATH,
        .content = "",
        .directory_path = ""
    };

    // Usar next_free_inode_index para encontrar un inodo libre para la raíz
    int free_inode_index = next_free_inode_index(ROOT_PATH);

    // Verificar si se encontró un inodo libre
    if (free_inode_index < 0) {
        // Si next_free_inode_index devuelve un error (negativo)
        fprintf(stderr, "Error al inicializar el sistema de archivos\n");
        return free_inode_index;  // Devolver el código de error
    }

    // Guardar el inodo raíz en el índice libre
    super.inodes[free_inode_index] = root_inode;
    super.bitmap_inodes[free_inode_index] = OCCUPIED;

    // Intentar guardar el superbloque en el archivo de filesystem
    FILE *fs_file_ptr = fopen(fs_file, "wb");
    if (fs_file_ptr == NULL) {
        perror("Error al crear el archivo de filesystem");
        return -errno;
    }

    // Escribir el superbloque completo en el archivo
    size_t written = fwrite(&super, sizeof(struct super_block), 1, fs_file_ptr);
    fclose(fs_file_ptr);

    if (written != 1) {
        perror("Error al escribir el superbloque");
        return -EIO;
    }
    
    return 0;
}

void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisop_destroy\n");
	FILE *file = fopen(fs_file, "w");
	//si falla el file open, no deberia dejar que se haga flush y write
	if (!file) {
		fprintf(stderr,
		        "[debug] Error saving filesystem: %s\n",
		        strerror(errno));
		return;
	}
	//agrego verificaciones a estos metodos
	size_t w = fwrite(&super, sizeof(super), 1, file);
	if (w!=1){
		fprintf(stderr, "[debug] Error escribiendo en archivo '%s': %s\n",fs_file, strerror(errno));
		fclose(file);
		return;
	}
	size_t f = fflush(file);
	if (f!=0){
		fprintf(stderr,"[debug] Error flush '%s': %s\n",fs_file, strerror(errno));
		fclose(file);
		return;
	}
	fclose(file);
}

/* Truncate or extend the given file so that it is precisely size bytes long. See truncate(2) for details. This call is required for read/write filesystems, because recreating a file will first truncate it. */
static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s\n", path);
	if (size > MAX_CONTENT) {
		fprintf(stderr, "[debug] Error truncate: %s\n", strerror(errno));
		errno = EINVAL;
		return -EINVAL;
	}

	struct inode* inode = find_inode(path);
	if (!inode) {
		fprintf(stderr, "[debug] Error read: %s\n", strerror(errno));
		errno = ENOENT;
		return -ENOENT;
	}

	inode->size = size;
	inode->last_modification = time(NULL);
	return 0;
}

static void
fisopfs_flush(const char *path, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_flush(%s)\n", path);
	return fisopfs_destroy(NULL);
}

static struct fuse_operations operations = {
	.getattr = fisopfs_getattr,
	.readdir = fisopfs_readdir,
	.read = fisopfs_read,
	.mkdir = fisopfs_mkdir,
	.rmdir = fisopfs_rmdir,
	.unlink = fisopfs_unlink,
	.init = fisopfs_init,
	.write = fisopfs_write,
	.destroy = fisopfs_destroy,
	.create = fisopfs_create,
	.utimens = fisopfs_utimens,
	.truncate = fisopfs_truncate,
};

int
main(int argc, char *argv[])
{
	// El ultimo argumento es el path del archivo del fs, si es que se pasa
	if (strcmp(argv[1], "-f") == 0) {
		if (argc == 4) {
			strcpy(fs_file, argv[3]);
			argv[3] = NULL;
			argc--;
		}
	} else {
		if (argc == 3) {
			strcpy(fs_file, argv[2]);
			argv[2] = NULL;
			argc--;
		}
	}

	return fuse_main(argc, argv, &operations, NULL);
}