# Ginger OS Virtual FileSystem

This repository is intended to be a simple implementation of a hirachical virtual filesystem developed to the best of my abilities and as such does not guarantee any level of accuracy whatsoever with regards to the standards(e.g POSIX).

I have started this repo because i have noticed that there is not many sites on the web that teach VFS implementation thoroughly. I have been stuck in a loop on how to implement the best VFS i can for my operating system(s) (http://github.com/emment_yamikani/gingerOsx-x86_64.git or the 32bit version @ http://github.com/emment_yamikani/gingerOs.git). But most of the sites i have encountered so far are either lacking in one area or the other or fully non-unsable for a serious OS project.

Therefore, i have taken it upon myself to implement something with hopes that other hobbists out there who are facing the same challenges can draw some inspiration or maybe even adopt this software source code until such a time when they feel comfortable enough to deverge from the implementation designs herein.

Needless to say, this repository has code that is still far from being used in an operating system. But when the right time comes i'll endevour to make it known ;).
#
## Design
    This VFS is build around the Unix concept of everything (even a spinlock ??) is a file. This system has multiple data structures it uses to keep track of its operations. The following are the currently and heavily used structures:
        1. Dentry or directory entry: used to name an entry in a directory (can be a filesystem, or any file).
        2. Inode: In-memory representation of a file.
        3. fs_mount: represents a mounted filesystem.
        4: superblock: In-memory representation of a superblock.
        5: filesystem: In-memory representation of a filesystem, keeps all the necessary metadata about a filesystem.

    The struct filesystem is the structure responsible for holding all the necessary metadata about a named filesystem. 
    I is an in-memory representation of a filesystem and as such it is the struct that is responsible for keeping track of the activities of the named filesystem. Without it the Operating system has no proper way of handling filesystem specific details. Among the various structures defined in the struct filesystem, the struct fs_iops, struct fs_files are stored in this structure. with this, a filesystem has a way of providing filesystem specific operarations such as renaming, reading and writing to a file.
# TO BE CONTINUED... 
#
### File operations
#
```c
 int        truncate(int fd);
 int        getattr(int fd, void *buf);
 int        setattr(int fd, void *buf);
 int        open(const char *pathname);
 ssize_t    read(int fd, void *buf, size_t nbytes);
 ssize_t    write(int fd, void *buf, size_t nbytes);
 int        create(const char *pathname, mode_t mode);
 int        fcntl(int fd, int cmd, ... /*arguments*/);
 int        ioctl(int fd, int request, ... /*arguments*/);
 int        rename(const char *oldname, const char *newname);
 ssize_t    readdir(int dirfd, off_t off, struct dirent *dirent);
 int        perm();
 int        link();
 int        mknod();
 int        mount();
 int        mkdir();
 int        unlink();
 int        lookup();
 int        symlink();
```
#