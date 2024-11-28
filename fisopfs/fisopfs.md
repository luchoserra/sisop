# fisopfs-fs

Lugar para respuestas en prosa y documentación del TP.

# ASPECTOS DE DISEÑO

## Descripción General

Este documento detalla los aspectos principales del diseño del sistema de archivos, abarcando:

- Las estructuras en memoria utilizadas para almacenar archivos, directorios y metadatos.
- Los métodos de acceso a archivos dado un path.
- Las estructuras auxiliares empleadas.
- El formato de serialización del sistema de archivos en disco.

---

## Estructuras en Memoria

### Estructuras Principales

1. **Superbloque (`superblock`)**
   - Contiene:
     - Un arreglo de inodos (`inodes`).
     - Un bitmap de inodos que indica si están libres (`FREE`) u ocupados (`OCCUPIED`).

2. **Inodo (`inode`)**
   - Representa un archivo o directorio y contiene los siguientes campos:
     - `type`: Tipo del inodo (`file` o `directory`).
     - `mode`: Permisos del archivo/directorio.
     - `size`: Tamaño del contenido asociado.
     - `uid`: ID del usuario propietario.
     - `gid`: ID del grupo propietario.
     - `last_access`: Tiempo del último acceso.
     - `last_modification`: Tiempo de la última modificación.
     - `creation_time`: Tiempo de creación.
     - `path`: Ruta asociada al inodo.
     - `content`: Contenido almacenado.
     - `directory_path`: Ruta del directorio padre.

### Estructuras Auxiliares

- **Bitmap de Inodos**
  - Un arreglo que indica si un inodo está libre (`FREE`) u ocupado (`OCCUPIED`).

---

## Cómo el Sistema Encuentra un Archivo Específico

La búsqueda de un archivo dado un path se realiza mediante la función `get_inode_index`. Esta función:

1. Verifica que el path no sea nulo ni vacío.
2. Si el path es la raíz (`"/"`), retorna el índice `0`.
3. Obtiene el nombre del archivo desde el path.
4. Busca coincidencias recorriendo los inodos activos en el bitmap.

### El Formato de Serialización del Sistema de Archivos en Disco

La serialización del sistema de archivos en disco se realiza mediante el siguiente comando:

```bash
./fisopfs -f /prueba archivo_de_alocacion
```

## PRUEBAS

### fisopfs_read:

```
/prueba$ mkdir hola
/prueba$ touch hola/hola.txt
/prueba$ echo "Hola!" > hola/hola.txt
/prueba$ cat hola/hola.txt
Hola!
/prueba$ echo "Chau!" >> hola/hola.txt
/prueba$ cat hola/hola.txt
Hola!
Chau!
/prueba$ cat chau.txt
cat: chau.txt: No existe el archivo o el directorio
/prueba$ cat
^C
/prueba$ cd ..
$ cd prueba
/prueba$ mkdir hola
/prueba$ touch hola/hola.txt
/prueba$ echo "Hola!" > hola/hola.txt
/prueba$ cat hola/hola.txt
Hola!
/prueba$ echo "Chau!" >> hola/hola.txt
/prueba$ cat hola/hola.txt
Hola!
Chau!
/prueba$ cat chau.txt
cat: chau.txt: No existe el archivo o el directorio
/prueba$ cat hola
cat: hola: Es un directorio
/prueba$ head -n 1 hola/hola.txt
Hola!
/prueba$ head -n 1 hola/chau.txt
head: no se puede abrir 'hola/chau.txt' para lectura: No existe el archivo o el directorio
/prueba$ head -n 1 hola
head: error al leer 'hola': Es un directorio

```
![imagen read](./capturas/read.png)

### fisopfs_truncate:

```
/prueba$ mkdir hola
/prueba$ touch hola.txt
/prueba$ echo "Hola! ¿Cómo estas?" > hola.txt
/prueba$ ls -lh hola.txt
-rw-r--r-- 1 usuario usuario 21 nov 28 00:07 hola.txt
/prueba$ truncate -s 10 hola.txt
/prueba$ ls -lh hola.txt
-rw-r--r-- 1 usuario usuario 10 nov 28 00:07 hola.txt
/prueba$ ls -lh chau.txt
ls: no se puede acceder a 'chau.txt': No existe el archivo o el directorio
```
![imagen truncate](./capturas/truncate.png)

### fisops_create

```
/prueba$ touch hola.txt
/prueba$ ls
hola.txt
/prueba$ touch
touch: falta un archivo como argumento
Pruebe 'touch --help' para más información.
```
![imagen create](./capturas/create.png)

### fisops_mkdir

```
/prueba$ mkdir directorio
/prueba$ ls
directorio
/prueba$ mkdir
mkdir: falta un operando
Pruebe 'mkdir --help' para más información.
```
![imagen create](./capturas/prueba_mkdir.png)

### fisops_destroy
```
Primero, montar sobre la terminal como se indico previamente. Luego se puede modificar, crear o eliminar archivos con comandos como cat,touch,echo.
Ejemplo, estando sobre el directorio prueba: 'touch mnt/test_destroy.txt' 'touch mnt/test_destroy.txt' o 'cat mnt/test_destroy' entre otros. L
uego se debe desmontar siguiendo el comando 'sudo umount prueba'
```

### fisops_getattr

```
/prueba$ mkdir ruta
/prueba$ touch ruta/archivo.txt
/prueba$stat ruta/rachivo.txt
stat ruta/rachivo.txt
stat: cannot statx 'ruta/rachivo.txt': No existe el archivo o el directorio
/prueba$ stat ruta/archivo.txt
Fichero: ruta/archivo.txt
  Tamaño: 0         	Bloques: 0          Bloque E/S: 4096   fichero regular vacío
Dispositivo: 804h/2052d	Nodo-i: 4238502     Enlaces: 1
Acceso: (0664/-rw-rw-r--)  Uid: ( 1000/   lucia)   Gid: ( 1000/   lucia)
Acceso: 2024-11-27 11:48:46.047617470 -0300
Modificación: 2024-11-27 11:48:46.047617470 -0300
      Cambio: 2024-11-27 11:48:46.047617470 -0300
    Creación: 2024-11-27 11:48:46.047617470 -0300

```

![imagen getattr](./capturas/getattr.png)

### fisops_rmdir
Prueba simple para ver que se elimina el directorio:
```
/prueba$ mkdir testdir
/prueba$ rmdir testdir
/prueba$ ls
/prueba$ 
```

![imagen prueba_simple_rmdir](./capturas/prueba_simple_rmdir.png)

Prueba para ver que sucede si no existe el directorio:
```
/prueba$ rmdir /noneexistent
rmdir: fallo al borrar '/noneexistent': No existe el archivo o el directorio 
```

![imagen rmdir_error](./capturas/rmdir_error.png)

Prueba donde se intenta eliminar un archivo y no un directorio:
```
![imagen 1](./capturas/getattr.png)
/prueba$ touch file
/prueba$ rmdir file
rmdir: fallo al borrar 'file': No es un directorio
```

![imagen rmdir_eliminar_arch](./capturas/rmdir_eliminar_arch.png)

Prueba donde se intenta eliminar un directorio no vacio:
```
/prueba$ mkdir noneemptydir
/prueba$ touch noneemptydir/file.txt
/prueba$ rmdir noneemptydir
rmdir: fallo al borrar 'noneemptydir': El directorio no esta vacio
```

![imagen rmdir_dir_no_vacio](./capturas/rmdir_dir_no_vacio.png)

### fisops_write
Prueba simple donde se intenta escribir un archivo
```
/prueba$ echo 'Este es un test' > testfile.txt
/prueba$ cat testfile.txt
Este es un test
```

![imagen write_test_simple](./capturas/write_test_simple.png)

Luego intentamos escribir en un archivo ya creado mas cosas en el directorio, al principio y al final:
```
/prueba$ echo 'test' > testfile.txt
/prueba$ echo "Hola Mundo" > testfile.txt
/prueba$ cat testfile.txt
Hola Mundo
test
/prueba$ echo "Hola!" >> testfile.txt
/prueba$cat testfile.txt
Hola Mundo
test
Hola!
```

![imagen write_archivo_ya_creado](./capturas/write_archivo_ya_creado.png)

En esta prueba se intenta escribir un archivo en otro directorio:
```
/prueba$ mkdir new
/prueba$ echo "hola de nuevo" > /new/test.txt
bash: /new/test.txt: NO existe el archivo o el directorio
/prueba$ echo "hola de nuevo" > new/test.txt
/prueba$ cat new/test.txt
hola de nuevo
```

![imagen write_directorio](./capturas/write_directorio.png)

En esta prueba se intenta escribir en algo que no es un archivo:
```
/prueba$ echo "Este es un texto" > testdir/
bash: testdir/: Es un directorio
```
![imagen write_no_archivo](./capturas/write_no_archivo.png)

### fisopfs_readdir
![imagen readdir](./capturas/readdir.png)


### fisopfs_unlink
![imagen unlink](./capturas/unlink.png)

### fisops_init

```
/fisopfs$ cd prueba/
/fisopfs/prueba$ mkdir a
/fisopfs/prueba$ ls
a
/fisopfs/prueba$ cd ..
/fisopfs$ cd prueba/
/fisopfs/prueba$ ls
a
```
![imagen init](./capturas/prueba_init.png)
