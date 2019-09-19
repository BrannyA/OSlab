#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void write_bootblock(FILE *image, FILE *bbfile, Elf32_Phdr *Phdr);
Elf32_Phdr *read_exec_file(FILE *opfile);
uint8_t count_kernel_sectors(Elf32_Phdr *Phdr);
void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz);

Elf32_Phdr *read_exec_file(FILE *opfile)
{
	Elf32_Ehdr ehdr;
	fseek(opfile, 0, SEEK_SET);
	fread(&ehdr, sizeof(char), sizeof(Elf32_Ehdr), opfile);

	Elf32_Phdr *phdr = (Elf32_Phdr *)malloc(sizeof(Elf32_Phdr));
	fseek(opfile, ehdr.e_phoff, SEEK_SET);
	fread(phdr, sizeof(Elf32_Phdr), 1, opfile);

	return phdr;
}

uint8_t count_kernel_sectors(Elf32_Phdr *Phdr)
{
	uint8_t num = (Phdr->p_memsz - 1) / 512 + 1;
	return num;
}

void write_bootblock(FILE *image, FILE *file, Elf32_Phdr *phdr)
{
	char buf[512];
	int bootblock_size = phdr->p_filesz;

	fseek(file, phdr->p_offset, SEEK_SET);
	fread(buf, sizeof(char), bootblock_size, file);
	fwrite(buf, sizeof(char), bootblock_size, image);

	fseek(image, 0x1fe, SEEK_SET);
	fputc(0x55, image);
	fputc(0xaa, image);
	return;
}

void write_kernel(FILE *image, FILE *knfile, Elf32_Phdr *Phdr, int kernelsz)
{
	char buf[512];
	int kernel_size = Phdr->p_filesz;

	fseek(knfile, Phdr->p_offset, SEEK_SET);
	fseek(image, 512, SEEK_SET);
	fread(buf, sizeof(char), kernel_size, knfile);
	fwrite(buf, sizeof(char), kernel_size, image);
	return;
}

void record_kernel_sectors(FILE *image, uint8_t kernelsz)
{
	fseek(image, 0x1fd, SEEK_SET);
	fwrite(&kernelsz, sizeof(uint8_t), 1, image);
	fseek(image, -0x1fd, 0);
	return;
}

void extent_opt(Elf32_Phdr *Phdr_bb, Elf32_Phdr *Phdr_k, int kernelsz)
{
	printf("bootblock image\n");
	printf("sectors: %u\n", 1);
	printf("segment file offset: 0x%x\n", Phdr_bb->p_offset);
	printf("virtural address: 0x%x\n", Phdr_bb->p_vaddr);
	printf("segment size in file: 0x%x\n", Phdr_bb->p_filesz);
	printf("segment size in memory: 0x%x\n", Phdr_bb->p_memsz);
	printf("padding up to 0x%x\n", 512);

	printf("kernel image\n");
	printf("sectors: %u\n", kernelsz);
	printf("segment file offset: 0x%x\n", Phdr_k->p_offset);
	printf("virtural address: 0x%x\n", Phdr_k->p_vaddr);
	printf("segment size in file: 0x%x\n", Phdr_k->p_filesz);
	printf("segment size in memory: 0x%x\n", Phdr_k->p_memsz);
	printf("padding up to 0x%x\n", kernelsz * 512);
	return;
}

int main(int argc, char *argv[])
{
	FILE *image = fopen("image", "wb");

	FILE *bootblock = fopen("bootblock", "rb");
	FILE *kernel = fopen("kernel", "rb");
	Elf32_Phdr *bootblock_phdr = read_exec_file(bootblock);
	write_bootblock(image, bootblock, bootblock_phdr);
	Elf32_Phdr *kernel_phdr = read_exec_file(kernel);
	int kernelsz = count_kernel_sectors(kernel_phdr);
	record_kernel_sectors(image, kernelsz);

	write_kernel(image, kernel, kernel_phdr, kernelsz);

	if (argc > 1 && strcmp(argv[1], "--extended") == 0)
		extent_opt(bootblock_phdr, kernel_phdr, kernelsz);

	record_kernel_sectors(image, kernelsz); //55aa

	fclose(bootblock);
	fclose(kernel);
	fclose(image);
	return;
}
