#include "loader.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h>

Elf32_Ehdr *ehdr = NULL; // ELF header
Elf32_Phdr *phdr = NULL; // Program headers
int fd = -1;             // File descriptor for ELF file
size_t page_size = 4096; // Page size in bytes (4KB)
int page_fault_count = 0;
int page_alloc_count = 0;
size_t internal_fragmentation = 0; // Track fragmentation

/*
 * Release memory and other cleanups
 */
void loader_cleanup() {
    if (ehdr) free(ehdr);
    if (phdr) free(phdr);
    if (fd >= 0) close(fd);
}

/*
 * Signal handler for segmentation faults (page faults)
 */
void handle_page_fault(int signum, siginfo_t *info, void *context) {
    void *fault_address = info->si_addr;
    page_fault_count++;

    // Find which segment caused the page fault
    for (int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD &&
            fault_address >= (void *)phdr[i].p_vaddr &&
            fault_address < (void *)(phdr[i].p_vaddr + phdr[i].p_memsz)) {

            // Calculate page boundary
            void *page_start = (void *)((uintptr_t)fault_address & ~(page_size - 1));
            size_t offset_in_segment = page_start - (void *)phdr[i].p_vaddr;
            size_t bytes_to_load = phdr[i].p_filesz > offset_in_segment
                                       ? phdr[i].p_filesz - offset_in_segment
                                       : 0;

            // Map memory page-by-page
            if (mmap(page_start, page_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0) == MAP_FAILED) {
                perror("Error mapping memory for page");
                exit(1);
            }
            page_alloc_count++;

            // Read segment contents into allocated memory if within file size
            if (bytes_to_load > 0) {
                lseek(fd, phdr[i].p_offset + offset_in_segment, SEEK_SET);
                read(fd, page_start, bytes_to_load > page_size ? page_size : bytes_to_load);
            }

            internal_fragmentation += page_size - (bytes_to_load % page_size);
            
            printf("Loading segment: %d\n", i);
            printf("Page starting point: %p\n", page_start);
            printf("Offset in segment: %zu bytes\n", offset_in_segment);
            printf("Bytes to load: %zu bytes\n", bytes_to_load);
            printf("Total Page Faults: %d\n", page_fault_count);
	    printf("Total Pages Allocated: %d\n", page_alloc_count);
            printf("Internal Fragmentation: %.2f KB\n\n", (double) internal_fragmentation / 1024);
            
            return;
        }
    }

    fprintf(stderr, "Segmentation fault at address %p: unhandled by SimpleSmartLoader\n", fault_address);
    exit(1);
}

/*
 * Load and run the ELF executable file
 */
void load_and_run_elf(char **exe) {
    // Open the ELF file
    fd = open(*exe, O_RDONLY);
    if (fd < 0) {
        perror("Error opening ELF file");
        exit(1);
    }

    // Allocate and read ELF header
    ehdr = malloc(sizeof(Elf32_Ehdr));
    if (!ehdr || read(fd, ehdr, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        perror("Error reading ELF header");
        exit(1);
    }

    // Validate ELF magic number
    if (ehdr->e_ident[0] != 0x7f || ehdr->e_ident[1] != 'E' || ehdr->e_ident[2] != 'L' || ehdr->e_ident[3] != 'F') {
        fprintf(stderr, "Invalid ELF file\n");
        exit(1);
    }

    // Allocate and read program headers
    phdr = malloc(ehdr->e_phentsize * ehdr->e_phnum);
    lseek(fd, ehdr->e_phoff, SEEK_SET);
    if (!phdr || read(fd, phdr, ehdr->e_phentsize * ehdr->e_phnum) != ehdr->e_phentsize * ehdr->e_phnum) {
        perror("Error reading program headers");
        exit(1);
    }

    // Register the signal handler for segmentation faults
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = handle_page_fault;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGSEGV, &sa, NULL) == -1) {
        perror("Error setting signal handler");
        exit(1);
    }

    // Attempt to execute from entry point
    void *entry_point = (void *)ehdr->e_entry;
    int (*_start)() = (int (*)())entry_point;

    printf("Attempting to start execution...\n");
    int result = _start();
    printf("User _start return value = %d\n", result);

    // Report results
    printf("Total page faults: %d\n", page_fault_count);
    printf("Total page allocations: %d\n", page_alloc_count);
    printf("Total internal fragmentation: %.2f KB\n", (double) internal_fragmentation / 1024);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ELF Executable>\n", argv[0]);
        exit(1);
    }

    load_and_run_elf(&argv[1]);
    loader_cleanup();
    return 0;
}
