cc=gcc
ld=ld
ar=ar
as=as

cflags+= -pthread -std=gnu2x -Wall -Werror -Wextra

cppflags+=

ldflags+=

vfs_flags:=$(cflags) $(cppflags) -Iinclude

#directory having iso-image recipe(contents)
bin_dir=bin
tmpfs_dir=tmpfs

linked_objs+=\
btree.o\
cmd.o\
dentry.o\
filesystem.o\
inode.o\
main.o\
mount.o\
path.o\
superblock.o\
vfs.o\
$(tmpfs_dir)/tmpfs.o

.PHONY: all clean

.SUFFIXES: .o .asm .s .c

.c.o:
	$(cc) $(vfs_flags) -MD -c $< -o $@

.s.o:
	$(cc) $(vfs_flags) -MD -c $< -o $@

.asm.o:
	nasm $< -f elf64 -o $@

all: vfs.elf run

vfs: vfs.elf

vfs.elf: $(linked_objs)
	$(cc) $(ldflags) $^ -o $(bin_dir)/$@

run:
	$(bin_dir)/vfs.elf

clean:
	rm $(linked_objs) $(linked_objs:.o=.d) $(bin_dir)/vfs.elf