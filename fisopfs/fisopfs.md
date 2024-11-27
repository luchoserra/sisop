# fisop-fs

Lugar para respuestas en prosa y documentación del TP.

### fisopfs_read:

```
/prueba$ mkdir hola
/prueba$ touch hola/hola.txt
/prueba$ echo "Hola!" > hola/hola.txt
/prueba$ echo "Chau!" >> hola/hola.txt
/prueba$ cat hola/hola.txt
Hola!
Chau!
/prueba$ head -n 1 hola/hola.txt
Hola!
```

### fisopfs_truncate:

```
/prueba$ touch hola.txt
/prueba$ echo "Hola! ¿Cómo estas?" > hola.txt
/prueba$ ls -lh hola.txt
-rw-r--r-- 1 usuario usuario 21 nov 26 00:52 hola.txt
/prueba$ truncate -s 10 hola.txt
/prueba$ ls -lh hola.txt
-rw-r--r-- 1 usuario usuario 10 nov 26 00:53 hola.txt
```

### fisops_create

```
/prueba$ touch hola.txt
/prueba$ ls
hola.txt
```

### fisops_destroy

Primero, montar sobre la terminal como se indico previamente. Luego se puede modificar, crear o eliminar archivos con comandos como cat,touch,echo. Ejemplo, estando sobre el directorio prueba: 'touch mnt/test_destroy.txt' 'touch mnt/test_destroy.txt' o 'cat mnt/test_destroy' entre otros. Luego se debe desmontar siguiendo el comando 'sudo umount prueba'


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

### fisops_rmdir
Prueba simple para ver que se elimina el directorio:
```
/prueba$ mkdir testdir
/prueba$ rmdir testdir
/prueba$ ls
/prueba$ 
```
Prueba para veri



### fisops_write

### fisopfs_readdir

### fisopfs_unlink