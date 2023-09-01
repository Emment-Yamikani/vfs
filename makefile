cc=gcc
ld=ld
ar=ar
as=as

cflags+= -O2 -g -lgcc -pthread -std=gnu17 -Wall -Werror -Wextra

cppflags+=

ldflags+=

vfs_flags:=$(cflags) $(cppflags) -Iinclude

#directory having iso-image recipe(contents)
bin_dir=bin

linked_objs+=\
dentry.o\
inode.o\
main.o\
mount.o\
vfs.o\

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