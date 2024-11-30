#define FUSE_USE_VERSION 30
#define MAX_CONTENT 1024
#define MAX_DIRECTORY_SIZE 1024
#define MAX_INODES 1024
#define MAX_PATH 256
#define ROOT_PATH "/"

#include <fuse.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

char filedisk[MAX_PATH] = "fs.fisopfs";

enum inode_type { INODE_FILE, INODE_DIR };
enum bitmap_state { FREE, OCCUPIED };

struct inode {
	enum inode_type type;
	mode_t mode;
	size_t size;
	uid_t uid;
	gid_t gid;
	time_t last_access;
	time_t last_modification;
	time_t creation_time;
	char path[MAX_PATH];
	char content[MAX_CONTENT];
	char directory_path[MAX_PATH];
};

struct superblock {
    struct inode inodes[MAX_INODES];
    enum bitmap_state bitmap_inodes[MAX_INODES];
};

struct superblock super = {};

/* ========================================
 *               AUXILIARES
 * ========================================
 */

const char* 
get_filename_from_path(const char* path) 
{
    if (!path || *path == '\0') {
        return NULL;
    }

    const char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        return path;
    }

    return last_slash + 1;
}

void
get_parent_path_from_path(char *parent_path)
{
	char *last_slash = strrchr(parent_path, '/');

	if (last_slash != NULL)
		*last_slash = '\0';
	else
		parent_path[0] = '\0';
}

int 
next_free_inode_index(const char *path)
{
    for (int i = 0; i < MAX_INODES; i++) {
        if (strcmp(super.inodes[i].path, path) == 0) {
			errno = EEXIST;
            return -EEXIST;
        }
        if (super.bitmap_inodes[i] == FREE) {
            return i;
        }
    }

	errno = ENOSPC;
    return -ENOSPC;
}

int 
new_inode(const char *path, mode_t mode, enum inode_type type)
{
	if (strlen(path) - 1 > MAX_CONTENT) {
        errno = ENAMETOOLONG;
        return -ENAMETOOLONG;
    }
    
    const char *filename = get_filename_from_path(path);
    if (!filename) {
        return -1;
    }

    int free_inode_index = next_free_inode_index(filename);
    if (free_inode_index < 0) {
        return free_inode_index;
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
    strcpy(new_inode.path, filename);

    if (type == INODE_FILE) {
        char parent_path[MAX_PATH];
        memcpy(parent_path, path + 1, strlen(path) - 1);
        parent_path[strlen(path) - 1] = '\0';

        get_parent_path_from_path(parent_path);

        if (strlen(parent_path) == 0) {
            strcpy(parent_path, ROOT_PATH);
        }

        strcpy(new_inode.directory_path, parent_path);

    } else {
        strcpy(new_inode.directory_path, ROOT_PATH);
    }

    super.inodes[free_inode_index] = new_inode;
    super.bitmap_inodes[free_inode_index] = OCCUPIED;

    return 0;
}

int
get_inode_index(const char *path)
{
    if (!path || strcmp(path, "") == 0) {
        return -1;
    }

    if (strcmp(path, ROOT_PATH) == 0) {
        return 0;
    }

    const char* filename = get_filename_from_path(path);
    if (!filename) {
        return -1;
    }

    for (int i = 0; i < MAX_INODES; i++) {
        if (super.bitmap_inodes[i] == 1) {
            if (strcmp(filename, super.inodes[i].path) == 0) {
                return i;
            }
        }
    }

    return -1;
}

static int is_child_inode(const struct inode *dir_inode, const struct inode *child_inode) {
    return strcmp(child_inode->directory_path, dir_inode->path) == 0;
}

struct inode **
files_in_dir(const char *path_dir, int *nfiles)
{
	int tope = 0;
	struct inode **files = malloc(MAX_INODES * sizeof(struct inode *));
	const char *filename = get_filename_from_path(path_dir);

	for (int i = 0; i < MAX_INODES; i++) {
		if (strcmp(super.inodes[i].directory_path,
		           filename) == 0) {
			files[tope++] = &super.inodes[i];
		}
	}

	*nfiles = tope;
	return files;
}

/* ========================================
 *             IMPLEMENTACIÓN
 * ========================================
 */

/* Return file attributes. The "stat" structure is described in detail in the stat(2) manual page. For the given pathname, this should fill in the elements of the "stat" structure. If a field is meaningless or semi-meaningless (e.g., st_ino) then it should be set to 0 or given a "reasonable" value. This call is pretty much required for a usable filesystem. */
static int
fisopfs_getattr(const char *path, struct stat *st)
{
	printf("[debug] fisopfs_getattr - path: %s\n", path);

	int inode_index = get_inode_index(path);
	if (inode_index == -1) {
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode inode = super.inodes[inode_index];

	st->st_dev = 0;
	st->st_ino = inode_index;
	st->st_uid = inode.uid;
	st->st_mode = inode.mode;
	st->st_atime = inode.last_access;
	st->st_mtime = inode.last_modification;
	st->st_ctime = inode.creation_time;
	st->st_size = inode.size;
	st->st_gid = inode.gid;
	st->st_nlink = 2;
	st->st_mode = __S_IFDIR | 0755;
	if (inode.type == INODE_FILE) {
		st->st_mode = __S_IFREG | 0644;
		st->st_nlink = 1;
	}

	return 0;
}

/* Return one or more directory entries (struct dirent) to the caller. This is one of the most complex FUSE functions. It is related to, but not identical to, the readdir(2) and getdents(2) system calls, and the readdir(3) library function. Because of its complexity, it is described separately below. Required for essentially any filesystem, since it's what makes ls and a whole bunch of other things work. */
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
	
	int inode_index = get_inode_index(path);
	if (inode_index == -1) {
		errno = ENOENT;
		return -ENOENT;
	}

	struct inode dir_inode = super.inodes[inode_index];

	if (dir_inode.type != INODE_DIR) {
		errno = -ENOTDIR;
		return -ENOTDIR;
	}
	dir_inode.last_access = time(NULL);

	for (int i = 1; i < MAX_INODES; i++) {
		if (super.bitmap_inodes[i] == OCCUPIED &&
            is_child_inode(&dir_inode, &super.inodes[i])) {
				filler(buffer, super.inodes[i].path, NULL, 0);
			
		}
	}

	return 0;
}

/* Read size bytes from the given file into the buffer buf, beginning offset bytes into the file. See read(2) for full details. Returns the number of bytes transferred, or 0 if offset was at or beyond the end of the file. Required for any sensible filesystem.*/
static int
fisopfs_read(const char* path, char *buf, size_t size, off_t offset, struct fuse_file_info* fi){
	printf("[debug] fisopfs_read - path: %s, offset: %lu, size: %lu\n", path, offset, size);

	if (offset < 0 || size < 0) {
		errno = EINVAL;
		return -EINVAL;
	}

	int inode_index = get_inode_index(path);
	if (inode_index == -1) {
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode *inode = &super.inodes[inode_index];

	if (inode->type == INODE_DIR) {
		errno = EISDIR;
		return -EISDIR;
	}

	if (offset >= inode->size) {
        return 0;
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
fisopfs_create(const char *path, mode_t mode, struct fuse_file_info *fi)
{
	printf("[debug] fisopfs_touch - path: %s\n", path);

	return new_inode(path, mode, INODE_FILE);
}

/* Create a directory with the given name. The directory permissions are encoded in mode. See mkdir(2) for details. This function is needed for any reasonable read/write filesystem. */
static int
fisopfs_mkdir(const char *path, mode_t mode)
{
	printf("[debug] fisopfs_mkdir - path: %s\n", path);

	return new_inode(path, mode, INODE_DIR);
}

/* Remove the given directory. This should succeed only if the directory is empty (except for "." and ".."). See rmdir(2) for details. */
static int
fisopfs_rmdir(const char *path)
{
	printf("[debug] fisopfs_rmdir - path: %s\n", path);

	int inode_index = get_inode_index(path);
	if (inode_index < 0) {
        errno = ENOENT;
		return -ENOENT;
    }
	struct inode *inode = &super.inodes[inode_index];
	
	if (inode->type != INODE_DIR) {
		errno = ENOTDIR;
		return -ENOTDIR;
	}
	
	int nfiles = 0;
	struct inode **files = files_in_dir(path, &nfiles);
	if (!files) {
		errno = ENOMEM;
		return -ENOMEM;
	}
	free(files);
	
	if (nfiles > 0) {
		errno = ENOTEMPTY;
		return -ENOTEMPTY;
	}
	super.bitmap_inodes[inode_index] = FREE;
	memset(inode, 0, sizeof(struct inode));
	return 0;
}

/* Remove (delete) the given file, symbolic link, hard link, or special node. Note that if you support hard links, unlink only deletes the data when the last hard link is removed. See unlink(2) for details. */
static int
fisopfs_unlink(const char *path)
{
	printf("[debug] fisopfs_unlink(%s)\n", path);
	
	int inode_index = get_inode_index(path);
	if (inode_index == -1) {
		errno = ENOENT;
		return -ENOENT;
	}

	struct inode *inode = &super.inodes[inode_index];

	if (inode->type == INODE_FILE) {
		errno = EISDIR;
		return -EISDIR;
	}

	memset(inode, 0, sizeof(struct inode));
	super.bitmap_inodes[inode_index] = FREE;

	return 0;
}

/* Initialize the filesystem. This function can often be left unimplemented, but it can be a handy way to perform one-time setup such as allocating variable-sized data structures or initializing a new filesystem. The fuse_conn_info structure gives information about what features are supported by FUSE, and can be used to request certain capabilities (see below for more information). The return value of this function is available to all file operations in the private_data field of fuse_context. It is also passed as a parameter to the destroy() method. (Note: see the warning under Other Options below, regarding relative pathnames.) */
void *
fisopfs_init(struct fuse_conn_info *conn)
{
	printf("[debug] fisop_init\n");

	FILE *file = fopen(filedisk, "r");
	if (!file) {
		memset(super.inodes, 0, sizeof(super.inodes));
		memset(super.bitmap_inodes, 0, sizeof(super.bitmap_inodes));
		struct inode *root_dir = &super.inodes[0];
		root_dir->type = INODE_DIR;
		root_dir->mode = __S_IFDIR | 0755;
		root_dir->size = MAX_DIRECTORY_SIZE;
		root_dir->uid = 1717;
		root_dir->gid = getgid();
		root_dir->last_access = time(NULL);
		root_dir->last_modification = time(NULL);
		root_dir->creation_time = time(NULL);
		strcpy(root_dir->path, ROOT_PATH);
		memset(root_dir->content, 0, sizeof(root_dir->content));
		strcpy(root_dir->directory_path, "");
		super.bitmap_inodes[0] = OCCUPIED;
	} else {
		int i = fread(&super, sizeof(super), 1, file);
		if (i != 1) {
			return NULL;
		}
		fclose(file);
	}
	return 0;
}

/* As for read above, except that it can't return 0. */
int
fisopfs_write(const char *path,const char *data,size_t size_data,off_t offset,struct fuse_file_info *fuse_info)
{
	printf("[debug] fisops_write (%s) \n", path);
	
	if (offset + size_data > MAX_CONTENT) {
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
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode *inode = &super.inodes[inode_index];
	
	if (inode->size < offset) {
		errno = EINVAL;
		return -EINVAL;
	}

	if (inode->type != INODE_FILE) {
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

/* Called when the filesystem exits. The private_data comes from the return value of init. */
void
fisopfs_destroy(void *private_data)
{
	printf("[debug] fisop_destroy\n");
	
	FILE *file = fopen(filedisk, "w");

	if (!file) {
		return;
	}

	size_t w = fwrite(&super, sizeof(super), 1, file);
	if (w!=1){
		fclose(file);
		return;
	}
	size_t f = fflush(file);
	if (f!=0){
		fclose(file);
		return;
	}
	fclose(file);
}

/* Update the last access time of the given object from ts[0] and the last modification time from ts[1]. Both time specifications are given to nanosecond resolution, but your filesystem doesn't have to be that precise; see utimensat(2) for full details. Note that the time specifications are allowed to have certain special values; however, I don't know if FUSE functions have to support them. This function isn't necessary but is nice to have in a fully functional filesystem. */
int
fisopfs_utimens(const char *path, const struct timespec tv[2])
{
	printf("[debug] fisop_utimens\n");

	int inode_index = get_inode_index(path);
	if (inode_index == -1) {
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode *inode = &super.inodes[inode_index];

	inode->last_access = tv[0].tv_sec;
	inode->last_modification = tv[1].tv_sec;
	return 0;
}

/* Truncate or extend the given file so that it is precisely size bytes long. See truncate(2) for details. This call is required for read/write filesystems, because recreating a file will first truncate it. */
static int
fisopfs_truncate(const char *path, off_t size)
{
	printf("[debug] fisopfs_truncate - path: %s\n", path);

	if (size > MAX_CONTENT) {
		errno = EINVAL;
		return -EINVAL;
	}

	int inode_index = get_inode_index(path);
	if (inode_index == -1) {
		errno = ENOENT;
		return -ENOENT;
	}
	struct inode *inode = &super.inodes[inode_index];

	inode->size = size;
	inode->last_modification = time(NULL);
	return 0;
}

/* Called on each close so that the filesystem has a chance to report delayed errors. Important: there may be more than one flush call for each open. Note: There is no guarantee that flush will ever be called at all! */
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
	.create = fisopfs_create,
	.mkdir = fisopfs_mkdir,
	.rmdir = fisopfs_rmdir,
	.unlink = fisopfs_unlink,
	.init = fisopfs_init,
	.write = fisopfs_write,
	.destroy = fisopfs_destroy,
	.utimens = fisopfs_utimens,
	.truncate = fisopfs_truncate,
};

int
main(int argc, char *argv[])
{
	if (strcmp(argv[1], "-f") == 0) {
		if (argc == 4) {
			strcpy(filedisk, argv[3]);
			argv[3] = NULL;
			argc--;
		}
	} else {
		if (argc == 3) {
			strcpy(filedisk, argv[2]);
			argv[2] = NULL;
			argc--;
		}
	}

	return fuse_main(argc, argv, &operations, NULL);
}