
# FAT32_FILESYSTEM

**FAT32_FILESYSTEM** is a tool written in C language that allows the user to create, modify and delete files and directories according to the FAT32 filesystem.

## QUICK INSTALL GUIDE

1. Download the project on your local machine;
2. Move inside the *sources/* folder;
3. Use *make* to compile and launch the program by typing *./FS_main*:
	- If the "disk" file does not exist, it will be created;
	- If the "disk" file exists, it will be used as store for the current session;
	- Removing the "disk" file all the files within it will be completely lost.

## USER MANUAL

By default, when launched for the first time, the program creates a *root/* directory.
Below there is a complete list of all the commands supported by the **FAT32_FILESYSTEM** tool:

- To open an existing file, use **open** command. The program will ask the name of the file to open and the opening mode: use *0* to open the file in read mode, *1* for write mode or *2* for a combination of the two modes. If you want to change the opening mode the file must be closed first and then opened again;

- To close a file use the **close** command. The program will ask the name of the file which should be closed;

- To create a file use the **createfile** command. The program will wait for the name of the file to be created. At this point the file will be opened in write mode;

- To delete a file use the **erasefile** command. The program will ask the name of the file to delete;

- To get a list of the open files use the **openedfiles** command;

- To create a directory use the **mkdir** command. The program will ask for the name of the new directory.

- To remove a directory use the **erasedir** command. The program will ask you the name of the directoy to be removed. Be aware that anything inside the directory will be deleted as well;

- To change the current directory use the **cd** command. The program will ask you the name of the child directory and to go back to the parent directory use the '..' folder name;

- To get a list of all the files and directories inside the current folder use the **ls** command;

- To move the file offset of an open file use the **seek** command. The program will ask for three arguments: the *name* of the file, the *offset* and the *whence* (from where the offset should be repositioned). For the *whence* parameter, use 'start' to move the pointer of *offset* bytes from the beginnig of the file, 'current' to advance the position of *offset* from the current position or 'end' to seek the position at *offset* bytes from the end of the file;

- To write inside an open file use the **write** command. The program will ask for the name of the file. Be aware that the program won't write any data unless the file is open in write mode;

- To read data from an open file use the **read** command. The program will ask for the name of the file to and how many byte should be read;

- To quit and close the program use the **quit** command. By doing so, all the open files, if any, will be closed.









