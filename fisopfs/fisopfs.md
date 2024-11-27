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