# FAT32_FILESYSTEM

Program that allows you to store informations in a file


HOW TO INSTALL 

Go to the 'sources' directory ('cd sources'), compile with 'make' command and run it with './FS_main' command.
If 'disk' file does not exist it will be created, otherwise the program uses it to store your files and everytime, if you want, you can delete 'disk' file and the program creates a newone. Pay attention, if you delete the 'disk' file, all the files will be lost.



HOW TO USE

By deafult, a 'root' directoty will be created, and to do everything you want, you have only to write the corresponding command that you see below:

- open existing file: 'open', the program will ask you the name of the file to open and the opening modality, put '0' to read the file, '1' to write or '2' to read and write.
If you want to change the modality opening you have to close and open it again.

- close opened file: 'close', the program will ask you the name of the file to close.

- create a file: 'createfile', the program will ask you the name of the file to create.
The file will be opened with modality '1'.

- erase a file: 'erasefile', the program will ask you the name of the file to delete;

- check opened files: 'openedfiles', the program will print the name of the opened file.

- create a directory: 'mkdir', the program will ask you the name.

- erase a directory: 'erasedir', the program will ask you the name of the directoy to erase. All the files and directory inside will be erased.

- change the current directory: 'cd', the program will ask you the name of the directory, if you want to return to parent directory put '..'.

- listing files and directories of the current directory: 'ls', the program will print the files/directories names.

- seek the position of an opened file: 'seek', the program will ask you the name of the file, the offset and from, put 'start' if you mean the offset from the beginnig of the file, 'current' to advance the position of offset from the current or 'end' to seek the position at offset from the end of the file.

- write in an opened file: 'write', the program will ask you the name of the file and if it opened with modality '1' or '2', the program will write the content put.

- read from an opened file: 'read', the program will ask you the name of the file to read and the size in bytes.

- quit program: 'quit', all opened files will be closed if there are.









