Deathstar
=========

Halo PC Map Deprotection Library

  Deathstar is a deprotection library for deprotecting Halo PC/Mac and Halo Custom Edition maps. It is made to function as both a program for anyone's use and also as a library for projects using the portable C language. While it has functions for opening maps on its own, you can use your own functions.
  
#### Map Opening
  To open a map, you can use a file path or a buffer.
  
``` c
MapData exampleMapBuffer = openMapFromBuffer((void *)buffer, (uint32_t)length);
MapData exampleMapPath = openMapAtPath((char *)path);
```

#### Map Deprotection
  There are two methods used for deprotecting maps, which can be used together if needed. ZTeam Deprotection deobfuscates tag classes, and name deprotection deobfuscates tag names. Tag names cannot be recovered, if they were obfuscated.

``` c
MapData exampleMap = openMapFromBuffer((void *)buffer, (uint32_t)length);
MapData deprotectedVersion = zteam_deprotectMap(exampleMap);
free(exampleMap.buffer);  //The methods allocate a new buffer. Depending on what you are
                          //trying to do, you may want to free the original buffer.
```
