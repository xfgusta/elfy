#include "elf.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <libelf.h>
#include <gelf.h>

#define ELFY_VERSION "0.1.0"

#define TAB      "    "
#define C_RED    "\033[31m"
#define C_GREEN  "\033[32m"
#define C_YELLOW "\033[33m"
#define C_END    "\033[0m"

#define print_error(...) fprintf(stderr, "elfy: " __VA_ARGS__);

int file_header_opt,
    program_headers_opt,
    section_headers_opt,
    dynamic_section_opt,
    symtab_opt,
    dynamic_symtab_opt,
    color_opt,
    help_opt,
    version_opt,
    all_opt;

const struct option long_opts[] = {
    {"file-header",     no_argument, &file_header_opt,     1},
    {"program-headers", no_argument, &program_headers_opt, 1},
    {"section-headers", no_argument, &section_headers_opt, 1},
    {"dynamic",         no_argument, &dynamic_section_opt, 1},
    {"symtab",          no_argument, &symtab_opt,          1},
    {"dyn-syms",        no_argument, &dynamic_symtab_opt,  1},
    {"all",             no_argument, &all_opt,             1},
    {"color",           no_argument, &color_opt,           1},
    {"help",            no_argument, &help_opt,            1},
    {"version",         no_argument, &version_opt,         1},
    {0,                 0,           0,                    0}
};

// length of the longest field name (used by print_field)
int field_max_len = 0;

// add color to the title when needed
void print_title(char *title, ...) {
    va_list args;
    va_start(args, title);

    if(color_opt)
        printf("%s", C_YELLOW);

    vprintf(title, args);

    if(color_opt)
        printf("%s", C_END);

    putchar('\n');

    va_end(args);
}

// print field name and its value
// add spaces between name and value based on field_max_len
// don't print value if value is NULL
void print_field(const char *field, char *value, ...) {
    int len = strlen(field);

    if(color_opt)
        printf(C_RED "%s" C_END TAB "%*s", field, field_max_len - len, "");
    else
        printf("%s" TAB "%*s", field, field_max_len - len, "");

    if(value) {
        va_list args;
        va_start(args, value);

        if(color_opt)
            printf("%s", C_GREEN);

        vprintf(value, args);

        if(color_opt)
            printf("%s", C_END);

        putchar('\n');

        va_end(args);
    }
}

// print field value and its info inside parentheses
// e.g.: ET_DYN (shared object file)
void print_field_info(char *value, char *info) {
    if(color_opt)
        printf(C_GREEN "%s" C_END " (%s)\n", value, info);
    else
        printf("%s (%s)\n", value, info);
}

// display the elf file header (option -h)
void show_file_header(Elf *elf) {
    GElf_Ehdr ehdr;

    print_title("File Header\n");

    // strlen("EI_ABIVERSION")
    field_max_len = 13;

    // get the elf file header
    if(!gelf_getehdr(elf, &ehdr)) {
        print_error("gelf_getehdr() failed: %s", elf_errmsg(-1));
        exit(EXIT_FAILURE);
    }

    print_title("Elf_Ehdr");

    // magic number and other info
    print_field("e_ident", NULL);

    if(color_opt)
        printf("%s", C_GREEN);

    for(int i = 0; i < EI_NIDENT; i++) {
        if(i + 1 != EI_NIDENT)
            printf("%2.2x ", ehdr.e_ident[i]);
        else
            printf("%2.2x\n", ehdr.e_ident[i]);
    }

    if(color_opt)
        printf("%s", C_END);

    // object file type
    print_field("e_type", NULL);
    switch(ehdr.e_type) {
        case ET_NONE:
            print_field_info("ET_NONE", "unknown type");
            break;
        case ET_REL:
            print_field_info("ET_REL", "relocatable file");
            break;
        case ET_EXEC:
            print_field_info("ET_EXEC", "executable file");
            break;
        case ET_DYN:
            print_field_info("ET_DYN", "shared object file");
            break;
        case ET_CORE:
            print_field_info("ET_CORE", "core file");
            break;
        default:
            if(color_opt)
                printf(C_GREEN "%#x" C_END, ehdr.e_type);
            else
                printf("%#x", ehdr.e_type);

            if((ehdr.e_type >= ET_LOOS) && (ehdr.e_type <= ET_HIOS))
                puts(" (os-specific)");
            else if(ehdr.e_type >= ET_LOPROC)
                puts(" (processor-specific)");
            else
                puts(" (unknown)");
    }

    // architecture
    print_field("e_machine", NULL);
    switch(ehdr.e_machine) {
        case EM_NONE:
            print_field_info("EM_NONE", "no machine");
            break;
        case EM_M32:
            print_field_info("EM_M32", "AT&T WE 32100");
            break;
        case EM_SPARC:
            print_field_info("EM_SPARC", "SUN SPARC");
            break;
        case EM_386:
            print_field_info("EM_386", "Intel 80386");
            break;
        case EM_68K:
            print_field_info("EM_68K", "Motorola m68k family");
            break;
        case EM_88K:
            print_field_info("EM_88K", "Motorola m88k family");
            break;
        case EM_IAMCU:
            print_field_info("EM_IAMCU", "Intel MCU");
            break;
        case EM_860:
            print_field_info("EM_860", "Intel 80860");
            break;
        case EM_MIPS:
            print_field_info("EM_MIPS", "MIPS R3000 big-endian");
            break;
        case EM_S370:
            print_field_info("EM_S370", "IBM System/370");
            break;
        case EM_MIPS_RS3_LE:
            print_field_info("EM_MIPS_RS3_LE", "MIPS R3000 little-endian");
            break;
        case EM_PARISC:
            print_field_info("EM_PARISC", "HPPA");
            break;
        case EM_VPP500:
            print_field_info("EM_VPP500", "Fujitsu VPP500");
            break;
        case EM_SPARC32PLUS:
            print_field_info("EM_SPARC32PLUS", "Sun's \"v8plus\"");
            break;
        case EM_960:
            print_field_info("EM_960", "Intel 80960");
            break;
        case EM_PPC:
            print_field_info("EM_PPC", "PowerPC");
            break;
        case EM_PPC64:
            print_field_info("EM_PPC64", "PowerPC 64-bit");
            break;
        case EM_S390:
            print_field_info("EM_S390", "IBM S390");
            break;
        case EM_SPU:
            print_field_info("EM_SPU", "IBM SPU/SPC");
            break;
        case EM_V800:
            print_field_info("EM_V800", "NEC V800 series");
            break;
        case EM_FR20:
            print_field_info("EM_FR20", "Fujitsu FR20");
            break;
        case EM_RH32:
            print_field_info("EM_RH32", "TRW RH-32");
            break;
        case EM_RCE:
            print_field_info("EM_RCE", "Motorola RCE");
            break;
        case EM_ARM:
            print_field_info("EM_ARM", "ARM");
            break;
        case EM_FAKE_ALPHA:
            print_field_info("EM_FAKE_ALPHA", "Digital Alpha");
            break;
        case EM_SH:
            print_field_info("EM_SH", "Hitachi SH");
            break;
        case EM_SPARCV9:
            print_field_info("EM_SPARCV9", "SPARC v9 64-bit");
            break;
        case EM_TRICORE:
            print_field_info("EM_TRICORE", "Siemens Tricore");
            break;
        case EM_ARC:
            print_field_info("EM_ARC", "Argonaut RISC Core");
            break;
        case EM_H8_300:
            print_field_info("EM_H8_300", "Hitachi H8/300");
            break;
        case EM_H8_300H:
            print_field_info("EM_H8_300H", "Hitachi H8/300H");
            break;
        case EM_H8S:
            print_field_info("EM_H8S", "Hitachi H8S");
            break;
        case EM_H8_500:
            print_field_info("EM_H8_500", "Hitachi H8/500");
            break;
        case EM_IA_64:
            print_field_info("EM_IA_64", "Intel Merced");
            break;
        case EM_MIPS_X:
            print_field_info("EM_MIPS_X", "Stanford MIPS-X");
            break;
        case EM_COLDFIRE:
            print_field_info("EM_COLDFIRE", "Motorola Coldfire");
            break;
        case EM_68HC12:
            print_field_info("EM_68HC12", "Motorola M68HC12");
            break;
        case EM_MMA:
            print_field_info("EM_MMA", "Fujitsu MMA Multimedia Accelerator");
            break;
        case EM_PCP:
            print_field_info("EM_PCP", "Siemens PCP");
            break;
        case EM_NCPU:
            print_field_info("EM_NCPU", "Sony nCPU embeeded RISC");
            break;
        case EM_NDR1:
            print_field_info("EM_NDR1", "Denso NDR1 microprocessor");
            break;
        case EM_STARCORE:
            print_field_info("EM_STARCORE", "Motorola Start*Core processor");
            break;
        case EM_ME16:
            print_field_info("EM_ME16", "Toyota ME16 processor");
            break;
        case EM_ST100:
            print_field_info("EM_ST100", "STMicroelectronic ST100 processor");
            break;
        case EM_TINYJ:
            print_field_info("EM_TINYJ", "Advanced Logic Corp. Tinyj emb.fam");
            break;
        case EM_X86_64:
            print_field_info("EM_X86_64", "AMD x86-64 architecture");
            break;
        case EM_PDSP:
            print_field_info("EM_PDSP", "Sony DSP Processor");
            break;
        case EM_PDP10:
            print_field_info("EM_PDP10", "Digital PDP-10");
            break;
        case EM_PDP11:
            print_field_info("EM_PDP11", "Digital PDP-11");
            break;
        case EM_FX66:
            print_field_info("EM_FX66", "Siemens FX66 microcontroller");
            break;
        case EM_ST9PLUS:
            print_field_info("EM_ST9PLUS", "STMicroelectronics ST9+ 8/16 mc");
            break;
        case EM_ST7:
            print_field_info("EM_ST7", "STmicroelectronics ST7 8 bit mc");
            break;
        case EM_68HC16:
            print_field_info("EM_68HC16", "Motorola MC68HC16 microcontroller");
            break;
        case EM_68HC11:
            print_field_info("EM_68HC11", "Motorola MC68HC11 microcontroller");
            break;
        case EM_68HC08:
            print_field_info("EM_68HC08", "Motorola MC68HC08 microcontroller");
            break;
        case EM_68HC05:
            print_field_info("EM_68HC05", "Motorola MC68HC05 microcontroller");
            break;
        case EM_SVX:
            print_field_info("EM_SVX", "Silicon Graphics SVx");
            break;
        case EM_ST19:
            print_field_info("EM_ST19", "STMicroelectronics ST19 8 bit mc");
            break;
        case EM_VAX:
            print_field_info("EM_VAX", "Digital VAX");
            break;
        case EM_CRIS:
            print_field_info("EM_CRIS", "Axis Communications 32-bit emb.proc");
            break;
        case EM_JAVELIN:
            print_field_info("EM_JAVELIN", "Infineon Technologies 32-bit emb.proc");
            break;
        case EM_FIREPATH:
            print_field_info("EM_FIREPATH", "Element 14 64-bit DSP Processor");
            break;
        case EM_ZSP:
            print_field_info("EM_ZSP", "LSI Logic 16-bit DSP Processor");
            break;
        case EM_MMIX:
            print_field_info("EM_MMIX", "Donald Knuth's educational 64-bit proc");
            break;
        case EM_HUANY:
            print_field_info("EM_HUANY", "Harvard University machine-independent object files");
            break;
        case EM_PRISM:
            print_field_info("EM_PRISM", "SiTera Prism");
            break;
        case EM_AVR:
            print_field_info("EM_AVR", "Atmel AVR 8-bit microcontroller");
            break;
        case EM_FR30:
            print_field_info("EM_FR30", "Fujitsu FR30");
            break;
        case EM_D10V:
            print_field_info("EM_D10V", "Mitsubishi D10V");
            break;
        case EM_D30V:
            print_field_info("EM_D30V", "Mitsubishi D30V");
            break;
        case EM_V850:
            print_field_info("EM_V850", "NEC v850");
            break;
        case EM_M32R:
            print_field_info("EM_M32R", "Mitsubishi M32R");
            break;
        case EM_MN10300:
            print_field_info("EM_MN10300", "Matsushita MN10300");
            break;
        case EM_MN10200:
            print_field_info("EM_MN10200", "Matsushita MN10200");
            break;
        case EM_PJ:
            print_field_info("EM_PJ", "picoJava");
            break;
        case EM_OPENRISC:
            print_field_info("EM_OPENRISC", "OpenRISC 32-bit embedded processor");
            break;
        case EM_ARC_COMPACT:
            print_field_info("EM_ARC_COMPACT", "ARC International ARCompact");
            break;
        case EM_XTENSA:
            print_field_info("EM_XTENSA", "Tensilica Xtensa Architecture");
            break;
        case EM_VIDEOCORE:
            print_field_info("EM_VIDEOCORE", "Alphamosaic VideoCore");
            break;
        case EM_TMM_GPP:
            print_field_info("EM_TMM_GPP", "Thompson Multimedia General Purpose Proc");
            break;
        case EM_NS32K:
            print_field_info("EM_NS32K", "National Semi. 32000");
            break;
        case EM_TPC:
            print_field_info("EM_TPC", "Tenor Network TPC");
            break;
        case EM_SNP1K:
            print_field_info("EM_SNP1K", "Trebia SNP 1000");
            break;
        case EM_ST200:
            print_field_info("EM_ST200", "STMicroelectronics ST200");
            break;
        case EM_IP2K:
            print_field_info("EM_IP2K", "Ubicom IP2xxx");
            break;
        case EM_MAX:
            print_field_info("EM_MAX", "MAX processor");
            break;
        case EM_CR:
            print_field_info("EM_CR", "National Semi. CompactRISC");
            break;
        case EM_F2MC16:
            print_field_info("EM_F2MC16", "Fujitsu F2MC16");
            break;
        case EM_MSP430:
            print_field_info("EM_MSP430", "Texas Instruments msp430");
            break;
        case EM_BLACKFIN:
            print_field_info("EM_BLACKFIN", "Analog Devices Blackfin DSP");
            break;
        case EM_SE_C33:
            print_field_info("EM_SE_C33", "Seiko Epson S1C33 family");
            break;
        case EM_SEP:
            print_field_info("EM_SEP", "Sharp embedded microprocessor");
            break;
        case EM_ARCA:
            print_field_info("EM_ARCA", "Arca RISC");
            break;
        case EM_UNICORE:
            print_field_info("EM_UNICORE", "PKU-Unity & MPRC Peking Uni. mc series");
            break;
        case EM_EXCESS:
            print_field_info("EM_EXCESS", "eXcess configurable cpu");
            break;
        case EM_DXP:
            print_field_info("EM_DXP", "Icera Semi. Deep Execution Processor");
            break;
        case EM_ALTERA_NIOS2:
            print_field_info("EM_ALTERA_NIOS2", "Altera Nios II");
            break;
        case EM_CRX:
            print_field_info("EM_CRX", "National Semi. CompactRISC CRX");
            break;
        case EM_XGATE:
            print_field_info("EM_XGATE", "Motorola XGATE");
            break;
        case EM_C166:
            print_field_info("EM_C166", "Infineon C16x/XC16x");
            break;
        case EM_M16C:
            print_field_info("EM_M16C", "Renesas M16C");
            break;
        case EM_DSPIC30F:
            print_field_info("EM_DSPIC30F", "Microchip Technology dsPIC30F");
            break;
        case EM_CE:
            print_field_info("EM_CE", "Freescale Communication Engine RISC");
            break;
        case EM_M32C:
            print_field_info("EM_M32C", "Renesas M32C");
            break;
        case EM_TSK3000:
            print_field_info("EM_TSK3000", "Altium TSK3000");
            break;
        case EM_RS08:
            print_field_info("EM_RS08", "Freescale RS08");
            break;
        case EM_SHARC:
            print_field_info("EM_SHARC", "Analog Devices SHARC family");
            break;
        case EM_ECOG2:
            print_field_info("EM_ECOG2", "Cyan Technology eCOG2");
            break;
        case EM_SCORE7:
            print_field_info("EM_SCORE7", "Sunplus S+core7 RISC");
            break;
        case EM_DSP24:
            print_field_info("EM_DSP24", "New Japan Radio (NJR) 24-bit DSP");
            break;
        case EM_VIDEOCORE3:
            print_field_info("EM_VIDEOCORE3", "Broadcom VideoCore III");
            break;
        case EM_LATTICEMICO32:
            print_field_info("EM_LATTICEMICO32", "RISC for Lattice FPGA");
            break;
        case EM_SE_C17:
            print_field_info("EM_SE_C17", "Seiko Epson C17");
            break;
        case EM_TI_C6000:
            print_field_info("EM_TI_C6000", "Texas Instruments TMS320C6000 DSP");
            break;
        case EM_TI_C2000:
            print_field_info("EM_TI_C2000", "Texas Instruments TMS320C2000 DSP");
            break;
        case EM_TI_C5500:
            print_field_info("EM_TI_C5500", "Texas Instruments TMS320C55x DSP");
            break;
        case EM_TI_ARP32:
            print_field_info("EM_TI_ARP32", "Texas Instruments App. Specific RISC");
            break;
        case EM_TI_PRU:
            print_field_info("EM_TI_PRU", "Texas Instruments Prog. Realtime Unit");
            break;
        case EM_MMDSP_PLUS:
            print_field_info("EM_MMDSP_PLUS", "STMicroelectronics 64bit VLIW DSP");
            break;
        case EM_CYPRESS_M8C:
            print_field_info("EM_CYPRESS_M8C", "Cypress M8C");
            break;
        case EM_R32C:
            print_field_info("EM_R32C", "Renesas R32C");
            break;
        case EM_TRIMEDIA:
            print_field_info("EM_TRIMEDIA", "NXP Semi. TriMedia");
            break;
        case EM_QDSP6:
            print_field_info("EM_QDSP6", "QUALCOMM DSP6");
            break;
        case EM_8051:
            print_field_info("EM_8051", "Intel 8051 and variants");
            break;
        case EM_STXP7X:
            print_field_info("EM_STXP7X", "STMicroelectronics STxP7x");
            break;
        case EM_NDS32:
            print_field_info("EM_NDS32", "Andes Tech. compact code emb. RISC");
            break;
        case EM_ECOG1X:
            print_field_info("EM_ECOG1X", "Cyan Technology eCOG1X");
            break;
        case EM_MAXQ30:
            print_field_info("EM_MAXQ30", "Dallas Semi. MAXQ30 mc");
            break;
        case EM_XIMO16:
            print_field_info("EM_XIMO16", "New Japan Radio (NJR) 16-bit DSP");
            break;
        case EM_MANIK:
            print_field_info("EM_MANIK", "M2000 Reconfigurable RISC");
            break;
        case EM_CRAYNV2:
            print_field_info("EM_CRAYNV2", "Cray NV2 vector architecture");
            break;
        case EM_RX:
            print_field_info("EM_RX", "Renesas RX");
            break;
        case EM_METAG:
            print_field_info("EM_METAG", "Imagination Tech. META");
            break;
        case EM_MCST_ELBRUS:
            print_field_info("EM_MCST_ELBRUS", "MCST Elbrus");
            break;
        case EM_ECOG16:
            print_field_info("EM_ECOG16", "Cyan Technology eCOG16");
            break;
        case EM_CR16:
            print_field_info("EM_CR16", "National Semi. CompactRISC CR16");
            break;
        case EM_ETPU:
            print_field_info("EM_ETPU", "Freescale Extended Time Processing Unit");
            break;
        case EM_SLE9X:
            print_field_info("EM_SLE9X", "Infineon Tech. SLE9X");
            break;
        case EM_L10M:
            print_field_info("EM_L10M", "Intel L10M");
            break;
        case EM_K10M:
            print_field_info("EM_K10M", "Intel K10M");
            break;
        case EM_AARCH64:
            print_field_info("EM_AARCH64", "ARM AARCH64");
            break;
        case EM_AVR32:
            print_field_info("EM_AVR32", "Amtel 32-bit microprocessor");
            break;
        case EM_STM8:
            print_field_info("EM_STM8", "STMicroelectronics STM8");
            break;
        case EM_TILE64:
            print_field_info("EM_TILE64", "Tilera TILE64");
            break;
        case EM_TILEPRO:
            print_field_info("EM_TILEPRO", "Tilera TILEPro");
            break;
        case EM_MICROBLAZE:
            print_field_info("EM_MICROBLAZE", "Xilinx MicroBlaze");
            break;
        case EM_CUDA:
            print_field_info("EM_CUDA", "NVIDIA CUDA");
            break;
        case EM_TILEGX:
            print_field_info("EM_TILEGX", "Tilera TILE-Gx");
            break;
        case EM_CLOUDSHIELD:
            print_field_info("EM_CLOUDSHIELD", "CloudShield");
            break;
        case EM_COREA_1ST:
            print_field_info("EM_COREA_1ST", "KIPO-KAIST Core-A 1st gen");
            break;
        case EM_COREA_2ND:
            print_field_info("EM_COREA_2ND", "KIPO-KAIST Core-A 2nd gen");
            break;
        case EM_ARCV2:
            print_field_info("EM_ARCV2", "Synopsys ARCv2 ISA");
            break;
        case EM_OPEN8:
            print_field_info("EM_OPEN8", "Open8 RISC");
            break;
        case EM_RL78:
            print_field_info("EM_RL78", "Renesas RL78");
            break;
        case EM_VIDEOCORE5:
            print_field_info("EM_VIDEOCORE5", "Broadcom VideoCore V");
            break;
        case EM_78KOR:
            print_field_info("EM_78KOR", "Renesas 78KOR");
            break;
        case EM_56800EX:
            print_field_info("EM_56800EX", "Freescale 56800EX DSC");
            break;
        case EM_BA1:
            print_field_info("EM_BA1", "Beyond BA1");
            break;
        case EM_BA2:
            print_field_info("EM_BA2", "Beyond BA2");
            break;
        case EM_XCORE:
            print_field_info("EM_XCORE", "XMOS xCORE");
            break;
        case EM_MCHP_PIC:
            print_field_info("EM_MCHP_PIC", "Microchip 8-bit PIC(r)");
            break;
        case EM_INTELGT:
            print_field_info("EM_INTELGT", "Intel Graphics Technology");
            break;
        case EM_KM32:
            print_field_info("EM_KM32", "KM211 KM32");
            break;
        case EM_KMX32:
            print_field_info("EM_KMX32", "KM211 KMX32");
            break;
        case EM_EMX16:
            print_field_info("EM_EMX16", "KM211 KMX16");
            break;
        case EM_EMX8:
            print_field_info("EM_EMX8", "KM211 KMX8");
            break;
        case EM_KVARC:
            print_field_info("EM_KVARC", "KM211 KVARC");
            break;
        case EM_CDP:
            print_field_info("EM_CDP", "Paneve CDP");
            break;
        case EM_COGE:
            print_field_info("EM_COGE", "Cognitive Smart Memory Processor");
            break;
        case EM_COOL:
            print_field_info("EM_COOL", "Bluechip CoolEngine");
            break;
        case EM_NORC:
            print_field_info("EM_NORC", "Nanoradio Optimized RISC");
            break;
        case EM_CSR_KALIMBA:
            print_field_info("EM_CSR_KALIMBA", "CSR Kalimba");
            break;
        case EM_Z80:
            print_field_info("EM_Z80", "Zilog Z80");
            break;
        case EM_VISIUM:
            print_field_info("EM_VISIUM", "Controls and Data Services VISIUMcore");
            break;
        case EM_FT32:
            print_field_info("EM_FT32", "FTDI Chip FT32");
            break;
        case EM_MOXIE:
            print_field_info("EM_MOXIE", "Moxie processor");
            break;
        case EM_AMDGPU:
            print_field_info("EM_AMDGPU", "AMD GPU");
            break;
        case EM_RISCV:
            print_field_info("EM_RISCV", "RISC-V");
            break;
        case EM_BPF:
            print_field_info("EM_BPF", "Linux BPF -- in-kernel virtual machine");
            break;
        case EM_CSKY:
            print_field_info("EM_CSKY", "C-SKY");
            break;
        case EM_ALPHA:
            print_field_info("EM_ALPHA", "Alpha");
            break;
        default:
            if(color_opt)
                printf(C_GREEN "%#x" C_END, ehdr.e_machine);
            else
                printf("%#x", ehdr.e_machine);

            puts(" (unknown)");
    }

    // object file version
    print_field("e_version", "%x", ehdr.e_version);

    // entry point virtual address
    print_field("e_entry", "%#lx", ehdr.e_entry);

    // program header table file offset
    print_field("e_phoff", "%#lx", ehdr.e_phoff);

    // section header table file offset
    print_field("e_shoff", "%#lx", ehdr.e_shoff);

    // processor-specific flags
    print_field("e_flags", "%#x", ehdr.e_flags);

    // ELF header size in bytes
    print_field("e_ehsize", "%d", ehdr.e_ehsize);

    // program header table entry size
    print_field("e_phentsize", "%d", ehdr.e_phentsize);

    // program header table entry count
    print_field("e_phnum", NULL);

    if(color_opt)
        printf("%s", C_GREEN);

    // handle when phnum is too large to fit into e_phnum
    if(ehdr.e_phnum == PN_XNUM) {
        Elf_Scn *section = NULL;
        GElf_Shdr shdr;

        section = elf_getscn(elf, 0);
        if(!section) {
            print_error("elf_getscn() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }

        if(!gelf_getshdr(section, &shdr)) {
            print_error("gelf_getshdr() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }

        printf("%d\n", shdr.sh_info);
    } else
        printf("%d\n", ehdr.e_phnum);

    if(color_opt)
        printf("%s", C_END);

    // section header table entry size
    print_field("e_shentsize", "%d", ehdr.e_shentsize);

    // section header table entry count
    print_field("e_shnum", "%d", ehdr.e_shnum);

    // section header string table index
    print_field("e_shstrndx", "%d", ehdr.e_shstrndx);

    putchar('\n');

    // display the e_ident array in details
    print_title("Elf_Ehdr.e_ident");

    // file identification byte 0..3 index
    print_field("EI_MAG0", "%#x", ehdr.e_ident[EI_MAG0]);
    print_field("EI_MAG1", "%c", ehdr.e_ident[EI_MAG1]);
    print_field("EI_MAG2", "%c", ehdr.e_ident[EI_MAG2]);
    print_field("EI_MAG3", "%c", ehdr.e_ident[EI_MAG3]);

    // file class byte index
    print_field("EI_CLASS", NULL);
    switch(ehdr.e_ident[EI_CLASS]) {
        case ELFCLASSNONE:
            print_field_info("ELFCLASSNONE", "invalid class");
            break;
        case ELFCLASS32:
            print_field_info("ELFCLASS32", "32-bit object");
            break;
        case ELFCLASS64:
            print_field_info("ELFCLASS64", "64-bit object");
            break;
        default:
            if(color_opt)
                printf(C_GREEN "%#x" C_END, ehdr.e_ident[EI_CLASS]);
            else
                printf("%#x", ehdr.e_ident[EI_CLASS]);

            puts(" (unknown)");
    }

    // data encoding byte index
    print_field("EI_DATA", NULL);
    switch(ehdr.e_ident[EI_DATA]) {
        case ELFDATANONE:
            print_field_info("ELFDATANONE", "invalid data encoding");
            break;
        case ELFDATA2LSB:
            print_field_info("ELFDATA2LSB", "2's complement, little endian");
            break;
        case ELFDATA2MSB:
            print_field_info("ELFDATA2MSB", "2's complement, big endian");
            break;
        default:
            if(color_opt)
                printf(C_GREEN "%#x" C_END, ehdr.e_ident[EI_DATA]);
            else
                printf("%#x", ehdr.e_ident[EI_DATA]);

            puts(" (unknown)");
    }

    // file version byte index
    print_field("EI_VERSION", NULL);
    switch(ehdr.e_ident[EI_VERSION]) {
        case EV_NONE:
            print_field_info("EV_NONE", "invalid ELF version");
            break;
        case EV_CURRENT:
            print_field_info("EV_CURRENT", "current version");
            break;
        default:
            if(color_opt)
                printf(C_GREEN "%#x" C_END, ehdr.e_ident[EI_VERSION]);
            else
                printf("%#x", ehdr.e_ident[EI_VERSION]);

            puts(" (unknown)");
    }

    // OS ABI identification
    print_field("EI_OSABI", NULL);
    switch(ehdr.e_ident[EI_OSABI]) {
        case ELFOSABI_SYSV:
            print_field_info("ELFOSABI_SYSV", "UNIX System V");
            break;
        case ELFOSABI_HPUX:
            print_field_info("ELFOSABI_HPUX", "HP-UX");
            break;
        case ELFOSABI_NETBSD:
            print_field_info("ELFOSABI_NETBSD", "NetBSD");
            break;
        case ELFOSABI_GNU:
            print_field_info("ELFOSABI_GNU", "object uses GNU ELF extensions");
            break;
        case ELFOSABI_SOLARIS:
            print_field_info("ELFOSABI_SOLARIS", "Sun Solaris");
            break;
        case ELFOSABI_AIX:
            print_field_info("ELFOSABI_AIX", "IBM AIX");
            break;
        case ELFOSABI_IRIX:
            print_field_info("ELFOSABI_IRIX", "SGI Irix");
            break;
        case ELFOSABI_FREEBSD:
            print_field_info("ELFOSABI_FREEBSD", "FreeBSD");
            break;
        case ELFOSABI_TRU64:
            print_field_info("ELFOSABI_TRU64", "Compaq TRU64 UNIX");
            break;
        case ELFOSABI_MODESTO:
            print_field_info("ELFOSABI_MODESTO", "Novell Modesto");
            break;
        case ELFOSABI_OPENBSD:
            print_field_info("ELFOSABI_OPENBSD", "OpenBSD");
            break;
        case ELFOSABI_ARM_AEABI:
            print_field_info("ELFOSABI_ARM_AEABI", "ARM EABI");
            break;
        case ELFOSABI_ARM:
            print_field_info("ELFOSABI_ARM", "ARM");
            break;
        case ELFOSABI_STANDALONE:
            print_field_info("ELFOSABI_STANDALONE", "standalone (embedded) application");
            break;
        default:
            if(color_opt)
                printf(C_GREEN "%#x" C_END, ehdr.e_ident[EI_OSABI]);
            else
                printf("%#x", ehdr.e_ident[EI_OSABI]);

            puts(" (unknown)");
    }

    // ABI version
    print_field("EI_ABIVERSION", "%#x", ehdr.e_ident[EI_ABIVERSION]);

    // byte index of padding bytes
    print_field("EI_PAD", "%#x", ehdr.e_ident[EI_PAD]);
}

// display the program headers (option -p)
void show_program_headers(Elf *elf) {
    size_t num;

    print_title("Program Headers\n");

    // strlen("p_filesz")
    field_max_len = 8;

    // get the number of program headers
    if(elf_getphdrnum(elf, &num) != 0) {
        print_error("elf_getphdrnum() failed: %s", elf_errmsg(-1));
        exit(EXIT_FAILURE);
    }

    for(size_t i = 0; i < num; i++) {
        GElf_Phdr phdr;

        // get the program header
        if(!gelf_getphdr(elf, i, &phdr)) {
            print_error("gelf_getphdr() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }

        print_title("Elf_Phdr %zu", i);

        // segment type
        print_field("p_type", NULL);
        switch(phdr.p_type) {
            case PT_LOAD:
                print_field_info("PT_LOAD", "loadable program segment");
                break;
            case PT_DYNAMIC:
                print_field_info("PT_DYNAMIC", "dynamic linking information");
                break;
            case PT_INTERP:
                print_field_info("PT_INTERP", "program interpreter");
                break;
            case PT_NOTE:
                print_field_info("PT_NOTE", "auxiliary information");
                break;
            case PT_SHLIB:
                print_field_info("PT_SHLIB", "reserved");
                break;
            case PT_PHDR:
                print_field_info("PT_PHDR", "entry for the header table itself");
                break;
            case PT_TLS:
                print_field_info("PT_TLS", "thread-local storage segment");
                break;
            case PT_GNU_EH_FRAME:
                print_field_info("PT_GNU_EH_FRAME", "GCC .eh_frame_hdr segment");
                break;
            case PT_GNU_STACK:
                print_field_info("PT_GNU_STACK", "indicates stack executability");
                break;
            case PT_GNU_RELRO:
                print_field_info("PT_GNU_RELRO", "read-only after relocation");
                break;
            case PT_GNU_PROPERTY:
                print_field_info("PT_GNU_PROPERTY", "GNU property");
                break;
            default:
                if(color_opt)
                    printf(C_GREEN "%#x" C_END, phdr.p_type);
                else
                    printf("%#x", phdr.p_type);

                if((phdr.p_type >= PT_LOOS) && (phdr.p_type <= PT_HIOS))
                    puts(" (os-specific)");
                else if(phdr.p_type >= PT_LOPROC)
                    puts(" (processor-specific)");
                else
                    puts(" (unknown)");
        }

        // segment flags
        print_field("p_flags", NULL);
        switch(phdr.p_flags) {
            case PF_R:
                print_field_info("PF_R", "segment is readable");
                break;
            case PF_W:
                print_field_info("PF_W", "segment is writable");
                break;
            case PF_X:
                print_field_info("PF_X", "segment is executable");
                break;
            case PF_R | PF_W:
                print_field_info("PF_R | PF_W", "segment is readable and writable");
                break;
            case PF_R | PF_X:
                print_field_info("PF_R | PF_X", "segment is readable and executable");
                break;
            case PF_W | PF_X:
                print_field_info("PF_W | PF_X", "segment is writable and executable");
                break;
            case PF_R | PF_W | PF_X:
                print_field_info("PF_R | PF_W | PF_X", "segment is readable, writable and executable");
                break;
            default:
                if(color_opt)
                    printf(C_GREEN "%#x" C_END, phdr.p_flags);
                else
                    printf("%#x", phdr.p_flags);

                if(phdr.p_flags & PF_MASKOS)
                    puts(" (os-specific)");
                else if(phdr.p_flags & PF_MASKPROC)
                    puts(" (processor-specific)");
                else
                    puts(" (unknown)");
        }

        // segment file offset
        print_field("p_offset", "%#lx", phdr.p_offset);

        // segment virtual address
        print_field("p_vaddr", "%#lx", phdr.p_vaddr);

        // segment physical address
        print_field("p_paddr", "%#lx", phdr.p_paddr);

        // segment size in file
        print_field("p_filesz", "%#lx", phdr.p_filesz);

        // segment size in memory
        print_field("p_memsz", "%#lx", phdr.p_memsz);

        // segment alignment
        print_field("p_align", "%#lx", phdr.p_align);

        if(i + 1 != num)
            putchar('\n');
    }
}

// display the section headers (option -s)
void show_section_headers(Elf *elf) {
    size_t num;
    size_t shstrndx;

    print_title("Section Headers\n");

    // strlen("sh_addralign")
    field_max_len = 12;

    // get the number of section headers
    if(elf_getshdrnum(elf, &num) != 0) {
        print_error("elf_getshdrnum() failed: %s", elf_errmsg(-1));
        exit(EXIT_FAILURE);
    }

    // get the section index of the strtab
    if(elf_getshdrstrndx(elf, &shstrndx) != 0) {
        print_error("elf_getshdrstrndx() failed: %s", elf_errmsg(-1));
        exit(EXIT_FAILURE);
    }

    for(size_t i = 0; i < num; i++) {
        Elf_Scn *section = NULL;
        GElf_Shdr shdr;
        char *name = NULL;

        // get the section
        section = elf_getscn(elf, i);
        if(!section) {
            print_error("elf_getscn() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }

        // get the section header
        if(!gelf_getshdr(section, &shdr)) {
            print_error("gelf_getshdr() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }

        // get the section name from strtab
        name = elf_strptr(elf, shstrndx, shdr.sh_name);
        if(!name) {
            print_error("elf_strptr() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }

        print_title("Elf_Shdr %zu", i);

        // section name
        print_field("sh_name", NULL);
        if(color_opt)
            printf(C_GREEN "%d" C_END, shdr.sh_name);
        else
            printf("%d", shdr.sh_name);

        if(*name == '\0')
            putchar('\n');
        else
            printf(" (%s)\n", name);

        // section type
        print_field("sh_type", NULL);
        switch(shdr.sh_type) {
            case SHT_NULL:
                print_field_info("SHT_NULL", "section header table entry unused");
                break;
            case SHT_PROGBITS:
                print_field_info("SHT_PROGBITS", "program data");
                break;
            case SHT_SYMTAB:
                print_field_info("SHT_SYMTAB", "symbol table");
                break;
            case SHT_STRTAB:
                print_field_info("SHT_STRTAB", "string table");
                break;
            case SHT_RELA:
                print_field_info("SHT_RELA", "relocation entries with addends");
                break;
            case SHT_HASH:
                print_field_info("SHT_HASH", "symbol hash table");
                break;
            case SHT_DYNAMIC:
                print_field_info("SHT_DYNAMIC", "dynamic linking information");
                break;
            case SHT_NOTE:
                print_field_info("SHT_NOTE", "notes");
                break;
            case SHT_NOBITS:
                print_field_info("SHT_NOBITS", "program space with no data (bss)");
                break;
            case SHT_REL:
                print_field_info("SHT_REL", "relocation entries, no addends");
                break;
            case SHT_SHLIB:
                print_field_info("SHT_SHLIB", "reserved");
                break;
            case SHT_DYNSYM:
                print_field_info("SHT_DYNSYM", "dynamic linker symbol table");
                break;
            case SHT_INIT_ARRAY:
                print_field_info("SHT_INIT_ARRAY", "array of constructors");
                break;
            case SHT_FINI_ARRAY:
                print_field_info("SHT_FINI_ARRAY", "array of destructors");
                break;
            case SHT_PREINIT_ARRAY:
                print_field_info("SHT_PREINIT_ARRAY", "array of pre-constructors");
                break;
            case SHT_GROUP:
                print_field_info("SHT_GROUP", "section group");
                break;
            case SHT_SYMTAB_SHNDX:
                print_field_info("SHT_SYMTAB_SHNDX", "extended section indices");
                break;
            case SHT_GNU_ATTRIBUTES:
                print_field_info("SHT_GNU_ATTRIBUTES", "object attributes");
                break;
            case SHT_GNU_HASH:
                print_field_info("SHT_GNU_HASH", "GNU-style hash table");
                break;
            case SHT_GNU_LIBLIST:
                print_field_info("SHT_GNU_LIBLIST", "prelink library list");
                break;
            case SHT_CHECKSUM:
                print_field_info("SHT_CHECKSUM", "checksum for DSO content");
                break;
            case SHT_GNU_verdef:
                print_field_info("SHT_GNU_verdef", "version definition section");
                break;
            case SHT_GNU_verneed:
                print_field_info("SHT_GNU_verneed", "version needs section");
                break;
            case SHT_GNU_versym:
                print_field_info("SHT_GNU_versym", "version symbol table");
                break;
            default:
                if(color_opt)
                    printf(C_GREEN "%#x" C_END, shdr.sh_type);
                else
                    printf("%#x", shdr.sh_type);

                if((shdr.sh_type >= SHT_LOPROC) && (shdr.sh_type <= SHT_HIPROC))
                    puts(" (processor-specific)");
                else if((shdr.sh_type >= SHT_LOOS) && (shdr.sh_type <= SHT_HIOS))
                    puts(" (OS-specific)");
                else if((shdr.sh_type >= SHT_LOUSER) && (shdr.sh_type <= SHT_HIUSER))
                    puts(" (application-specific)");
                else
                    puts(" (unknown)");
        }

        // section flags
        print_field("sh_flags", NULL);
        {
            int first = 1;
            unsigned long flags = shdr.sh_flags;

            if(color_opt)
                printf("%s", C_GREEN);

            if(flags == 0)
                printf("%#lx", flags);

            while(flags) {
                unsigned long flag;

                flag = flags & -flags;
                flags &= ~flag;

                if(first)
                    first = 0;
                else
                    printf(" | ");

                switch(flag) {
                    // writable
                    case SHF_WRITE:
                        printf("SHF_WRITE");
                        break;
                    // occupies memory during execution
                    case SHF_ALLOC:
                        printf("SHF_ALLOC");
                        break;
                    // executable
                    case SHF_EXECINSTR:
                        printf("SHF_EXECINSTR");
                        break;
                    // might be merged
                    case SHF_MERGE:
                        printf("SHF_MERGE");
                        break;
                    // contains nul-terminated strings
                    case SHF_STRINGS:
                        printf("SHF_STRINGS");
                        break;
                    // `sh_info' contains SHT index
                    case SHF_INFO_LINK:
                        printf("SHF_INFO_LINK");
                        break;
                    // preserve order after combining
                    case SHF_LINK_ORDER:
                        printf("SHF_LINK_ORDER");
                        break;
                    // non-standard OS specific handling required
                    case SHF_OS_NONCONFORMING:
                        printf("SHF_OS_NONCONFORMING");
                        break;
                    // section is member of a group
                    case SHF_GROUP:
                        printf("SHF_GROUP");
                        break;
                    // section hold thread-local data
                    case SHF_TLS:
                        printf("SHF_TLS");
                        break;
                    // special ordering requirement
                    case SHF_ORDERED:
                        printf("SHF_ORDERED");
                        break;
                    // section is excluded unless referenced or allocated
                    case SHF_EXCLUDE:
                        printf("SHF_EXCLUDE");
                        break;
                    // section with compressed data
                    case SHF_COMPRESSED:
                        printf("SHF_COMPRESSED");
                        break;
                    // not to be GCed by linker
                    case SHF_GNU_RETAIN:
                        printf("SHF_GNU_RETAIN");
                        break;
                    default:
                        // the flag is unknown
                        printf("%#lx", flag);
                }
            }

            if(color_opt)
                printf("%s", C_END);

            putchar('\n');
        }

        // section virtual addr at execution
        print_field("sh_addr", "%#lx", shdr.sh_addr);

        // section file offset
        print_field("sh_offset", "%#lx", shdr.sh_offset);

        // section size in bytes
        print_field("sh_size", "%#lx", shdr.sh_size);

        // link to another section
        print_field("sh_link", "%#x", shdr.sh_link);

        // additional section information
        print_field("sh_info", "%#x", shdr.sh_info);

        // section alignment
        print_field("sh_addralign", "%#lx", shdr.sh_addralign);

        // entry size if section holds table
        print_field("sh_entsize", "%#lx", shdr.sh_entsize);

        if(i + 1 != num)
            putchar('\n');
    }
}

// display the dynamic section (option -d)
void show_dynamic_section(Elf *elf) {
    Elf_Scn *section = NULL;
    size_t sh_entsize;

    print_title("Dynamic Section\n");

    // strlen("d_val")
    field_max_len = 5;

    sh_entsize = gelf_fsize(elf, ELF_T_DYN, 1, EV_CURRENT);

    while((section = elf_nextscn(elf, section))) {
        GElf_Shdr shdr;
        Elf_Data *data = NULL;
        size_t num = 0;

        // get the section header
        if(!gelf_getshdr(section, &shdr)) {
            print_error("gelf_getshdr() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }

        // if it's not a dynamic section, skip the section
        if(shdr.sh_type != SHT_DYNAMIC)
            continue;

        num = shdr.sh_size / sh_entsize;

        // get data from section
        data = elf_getdata(section, data);
        if(!data) {
            print_error("elf_getdata() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }

        for(size_t i = 0; i < num; i++) {
            GElf_Dyn dyn;
            char *name = NULL;

            // get information from the dynamic table 
            if(!gelf_getdyn(data, i, &dyn)) {
                print_error("gelf_getdyn() failed: %s", elf_errmsg(-1));
                exit(EXIT_FAILURE);
            }

            print_title("Elf_Dyn %zu", i);

            // dynamic entry type
            print_field("d_tag", NULL);
            switch(dyn.d_tag) {
                case DT_NULL:
                    print_field_info("DT_NULL", "marks end of dynamic section");
                    break;
                case DT_NEEDED:
                    print_field_info("DT_NEEDED", "name of needed library");
                    break;
                case DT_PLTRELSZ:
                    print_field_info("DT_PLTRELSZ", "size in bytes of PLT relocs");
                    break;
                case DT_PLTGOT:
                    print_field_info("DT_PLTGOT", "processor defined value");
                    break;
                case DT_HASH:
                    print_field_info("DT_HASH", "address of symbol hash table");
                    break;
                case DT_STRTAB:
                    print_field_info("DT_STRTAB", "address of string table");
                    break;
                case DT_SYMTAB:
                    print_field_info("DT_SYMTAB", "address of symbol table");
                    break;
                case DT_RELA:
                    print_field_info("DT_RELA", "address of Rela relocs");
                    break;
                case DT_RELASZ:
                    print_field_info("DT_RELASZ", "total size of Rela relocs");
                    break;
                case DT_RELAENT:
                    print_field_info("DT_RELAENT", "size of one Rela reloc");
                    break;
                case DT_STRSZ:
                    print_field_info("DT_STRSZ", "size of string table");
                    break;
                case DT_SYMENT:
                    print_field_info("DT_SYMENT", "size of one symbol table entry");
                    break;
                case DT_INIT:
                    print_field_info("DT_INIT", "address of init function");
                    break;
                case DT_FINI:
                    print_field_info("DT_FINI", "address of termination function");
                    break;
                case DT_SONAME:
                    print_field_info("DT_SONAME", "name of shared object");
                    break;
                case DT_RPATH:
                    print_field_info("DT_RPATH", "library search path (deprecated)");
                    break;
                case DT_SYMBOLIC:
                    print_field_info("DT_SYMBOLIC", "start symbol search here");
                    break;
                case DT_REL:
                    print_field_info("DT_REL", "address of Rel relocs");
                    break;
                case DT_RELSZ:
                    print_field_info("DT_RELSZ", "total size of Rel relocs");
                    break;
                case DT_RELENT:
                    print_field_info("DT_RELENT", "size of one Rel reloc");
                    break;
                case DT_PLTREL:
                    print_field_info("DT_PLTREL", "type of reloc in PLT");
                    break;
                case DT_DEBUG:
                    print_field_info("DT_DEBUG", "for debugging; unspecified");
                    break;
                case DT_TEXTREL:
                    print_field_info("DT_TEXTREL", "Reloc might modify .text");
                    break;
                case DT_JMPREL:
                    print_field_info("DT_JMPREL", "address of PLT relocs");
                    break;
                case DT_BIND_NOW:
                    print_field_info("DT_BIND_NOW", "process relocations of object");
                    break;
                case DT_INIT_ARRAY:
                    print_field_info("DT_INIT_ARRAY", "array with addresses of init fct");
                    break;
                case DT_FINI_ARRAY:
                    print_field_info("DT_FINI_ARRAY", "array with addresses of fini fct");
                    break;
                case DT_INIT_ARRAYSZ:
                    print_field_info("DT_INIT_ARRAYSZ", "size in bytes of DT_INIT_ARRAY");
                    break;
                case DT_FINI_ARRAYSZ:
                    print_field_info("DT_FINI_ARRAYSZ", "size in bytes of DT_FINI_ARRAY");
                    break;
                case DT_RUNPATH:
                    print_field_info("DT_RUNPATH", "library search path");
                    break;
                case DT_FLAGS:
                    print_field_info("DT_FLAGS", "flags for the object being loaded");
                    break;
                case DT_PREINIT_ARRAY:
                    print_field_info("DT_PREINIT_ARRAY", "array with addresses of preinit fct");
                    break;
                case DT_PREINIT_ARRAYSZ:
                    print_field_info("DT_PREINIT_ARRAYSZ", "size in bytes of DT_PREINIT_ARRAY");
                    break;
                case DT_SYMTAB_SHNDX:
                    print_field_info("DT_SYMTAB_SHNDX", "address of SYMTAB_SHNDX section");
                    break;
                case DT_CHECKSUM:
                    if(color_opt)
                        printf(C_GREEN "DT_CHECKSUM" C_END "\n");
                    else
                        puts("DT_CHECKSUM");
                    break;
                case DT_PLTPADSZ:
                    if(color_opt)
                        printf(C_GREEN "DT_PLTPADSZ" C_END "\n");
                    else
                        puts("DT_PLTPADSZ");
                    break;
                case DT_MOVEENT:
                    print_field_info("DT_MOVEENT", "size in bytes of DT_MOVETAB");
                    break;
                case DT_MOVESZ:
                    print_field_info("DT_MOVESZ", "total size of DT_MOVETAB");
                    break;
                case DT_VERSYM:
                    if(color_opt)
                        printf(C_GREEN "DT_VERSYM" C_END "\n");
                    else
                        puts("DT_VERSYM");
                    break;
                case DT_TLSDESC_GOT:
                    if(color_opt)
                        printf(C_GREEN "DT_TLSDESC_GOT" C_END "\n");
                    else
                        puts("DT_TLSDESC_GOT");
                    break;
                case DT_TLSDESC_PLT:
                    if(color_opt)
                        printf(C_GREEN "DT_TLSDESC_PLT" C_END "\n");
                    else
                        puts("DT_TLSDESC_PLT");
                    break;
                case DT_RELACOUNT:
                    print_field_info("DT_RELACOUNT", "Rela reloc count");
                    break;
                case DT_RELCOUNT:
                    print_field_info("DT_RELCOUNT", "Rel reloc count");
                    break;
                case DT_GNU_PRELINKED:
                    print_field_info("DT_GNU_PRELINKED", "prelinking timestamp");
                    break;
                case DT_GNU_CONFLICTSZ:
                    print_field_info("DT_GNU_CONFLICTSZ", "size of conflict section");
                    break;
                case DT_GNU_LIBLISTSZ:
                    print_field_info("DT_GNU_LIBLISTSZ", "size of library list");
                    break;
                case DT_FEATURE_1:
                    print_field_info("DT_FEATURE_1", "feature selection (DTF_*)");
                    break;
                case DT_SYMINSZ:
                    print_field_info("DT_SYMINSZ", "size of syminfo table (in bytes)");
                    break;
                case DT_SYMINENT:
                    print_field_info("DT_SYMINENT", "entry size of syminfo");
                    break;
                case DT_GNU_HASH:
                    print_field_info("DT_GNU_HASH", "GNU-style hash table");
                    break;
                case DT_GNU_CONFLICT:
                    print_field_info("DT_GNU_CONFLICT", "start of conflict section");
                    break;
                case DT_GNU_LIBLIST:
                    print_field_info("DT_GNU_LIBLIST", "library list");
                    break;
                case DT_CONFIG:
                    print_field_info("DT_CONFIG", "configuration information");
                    break;
                case DT_DEPAUDIT:
                    print_field_info("DT_DEPAUDIT", "dependency auditing");
                    break;
                case DT_AUDIT:
                    print_field_info("DT_AUDIT", "object auditing");
                    break;
                case DT_PLTPAD:
                    print_field_info("DT_PLTPAD", "PLT padding");
                    break;
                case DT_MOVETAB:
                    print_field_info("DT_MOVETAB", "address of move table");
                    break;
                case DT_SYMINFO:
                    print_field_info("DT_SYMINFO", "address of syminfo table");
                    break;
                case DT_FLAGS_1:
                    print_field_info("DT_FLAGS_1", "state flags");
                    break;
                case DT_VERDEF:
                    print_field_info("DT_VERDEF", "address of version definition");
                    break;
                case DT_VERDEFNUM:
                    print_field_info("DT_VERDEFNUM", "number of version definitions");
                    break;
                case DT_VERNEED:
                    print_field_info("DT_VERNEED", "address of table with needed versions");
                    break;
                case DT_VERNEEDNUM:
                    print_field_info("DT_VERNEEDNUM", "number of needed versions");
                    break;
                case DT_AUXILIARY:
                    print_field_info("DT_AUXILIARY", "shared object to load before self");
                    break;
                case DT_FILTER:
                    print_field_info("DT_FILTER", "shared object to get values from");
                    break;
                default:
                    if(color_opt)
                        printf(C_GREEN "%#lx" C_END, dyn.d_tag);
                    else
                        printf("%#lx", dyn.d_tag);

                    if((dyn.d_tag >= DT_LOPROC) && (dyn.d_tag <= DT_HIPROC))
                        puts(" (processor-specific)");
                    else if((dyn.d_tag >= DT_LOOS) && (dyn.d_tag <= DT_HIOS))
                        puts(" (OS-specific)");
                    else
                        puts(" (unknown)");
            }

            // integer value
            print_field("d_val", NULL);
            switch(dyn.d_tag) {
                // print library name
                case DT_NEEDED:
                case DT_SONAME:
                    if(color_opt)
                        printf(C_GREEN "%#lx" C_END, dyn.d_un.d_val);
                    else
                        printf("%#lx", dyn.d_un.d_val);

                    if(dyn.d_tag == DT_NEEDED || dyn.d_tag == DT_SONAME) {
                        name = elf_strptr(elf, shdr.sh_link, dyn.d_un.d_val);
                        if(!name || *name == '\0')
                            putchar('\n');
                        else
                            printf(" (%s)\n", name);
                    } else
                        putchar('\n');

                    break;
                // print size/count/version/number of ...
                case DT_PLTRELSZ:
                case DT_RELASZ:
                case DT_RELAENT:
                case DT_STRSZ:
                case DT_SYMENT:
                case DT_RELSZ:
                case DT_RELENT:
                case DT_INIT_ARRAYSZ:
                case DT_FINI_ARRAYSZ:
                case DT_PREINIT_ARRAYSZ:
                case DT_MOVEENT:
                case DT_MOVESZ:
                case DT_RELACOUNT:
                case DT_RELCOUNT:
                case DT_GNU_CONFLICTSZ:
                case DT_GNU_LIBLISTSZ:
                case DT_SYMINSZ:
                case DT_SYMINENT:
                case DT_VERDEFNUM:
                case DT_VERNEEDNUM:
                    if(color_opt)
                        printf(C_GREEN "%ld\n" C_END, dyn.d_un.d_val);
                    else
                        printf("%ld\n", dyn.d_un.d_val);
                    break;
                // parse flags
                case DT_FLAGS:
                    {
                        int first = 1;
                        unsigned long flags = dyn.d_un.d_val;

                        if(color_opt)
                            printf("%s", C_GREEN);

                        if(flags == 0)
                            printf("%#lx", flags);

                        while(flags) {
                            unsigned long flag;

                            flag = flags & -flags;
                            flags &= ~flag;

                            if(first)
                                first = 0;
                            else
                                printf(" | ");

                            switch(flag) {
                                // object may use DF_ORIGIN
                                case DF_ORIGIN:
                                    printf("DF_ORIGIN");
                                    break;
                                // symbol resolutions starts here
                                case DF_SYMBOLIC:
                                    printf("DF_SYMBOLIC");
                                    break;
                                // object contains text relocations
                                case DF_TEXTREL:
                                    printf("DF_TEXTREL");
                                    break;
                                // no lazy binding for this object
                                case DF_BIND_NOW:
                                    printf("DF_BIND_NOW");
                                    break;
                                // module uses the static TLS model
                                case DF_STATIC_TLS:
                                    printf("DF_STATIC_TLS");
                                    break;
                                default:
                                    // the flag is unknown
                                    printf("%#lx", flag);
                            }
                        }

                        if(color_opt)
                            printf("%s", C_END);

                        putchar('\n');
                    }
                    break;
                // parse feature_1
                case DT_FEATURE_1:
                    {
                        int first = 1;
                        unsigned long flags = dyn.d_un.d_val;

                        if(color_opt)
                            printf("%s", C_GREEN);

                        if(flags == 0)
                            printf("%#lx", flags);

                        while(flags) {
                            unsigned long flag;

                            flag = flags & -flags;
                            flags &= ~flag;

                            if(first)
                                first = 0;
                            else
                                printf(" | ");

                            switch(flag) {
                                case DTF_1_PARINIT:
                                    printf("DTF_1_PARINIT");
                                    break;
                                case DTF_1_CONFEXP:
                                    printf("DTF_1_CONFEXP");
                                    break;
                                default:
                                    // the flag is unknown
                                    printf("%#lx", flag);
                            }
                        }

                        if(color_opt)
                            printf("%s", C_END);

                        putchar('\n');
                    }
                    break;
                // parse flags_1
                case DT_FLAGS_1:
                    {
                        int first = 1;
                        unsigned long flags = dyn.d_un.d_val;

                        if(color_opt)
                            printf("%s", C_GREEN);

                        if(flags == 0)
                            printf("%#lx", flags);

                        while(flags) {
                            unsigned long flag;

                            flag = flags & -flags;
                            flags &= ~flag;

                            if(first)
                                first = 0;
                            else
                                printf(" | ");

                            switch(flag) {
                                // set RTLD_NOW for this object
                                case DF_1_NOW:
                                    printf("DF_1_NOW");
                                    break;
                                // set RTLD_GLOBAL for this object
                                case DF_1_GLOBAL:
                                    printf("DF_1_GLOBAL");
                                    break;
                                // set RTLD_GROUP for this object
                                case DF_1_GROUP:
                                    printf("DF_1_GROUP");
                                    break;
                                // set RTLD_NODELETE for this object
                                case DF_1_NODELETE:
                                    printf("DF_1_NODELETE");
                                    break;
                                // trigger filtee loading at runtime
                                case DF_1_LOADFLTR:
                                    printf("DF_1_LOADFLTR");
                                    break;
                                // set RTLD_INITFIRST for this object
                                case DF_1_INITFIRST:
                                    printf("DF_1_INITFIRST");
                                    break;
                                // set RTLD_NOOPEN for this object
                                case DF_1_NOOPEN:
                                    printf("DF_1_NOOPEN");
                                    break;
                                // $ORIGIN must be handled
                                case DF_1_ORIGIN:
                                    printf("DF_1_ORIGIN");
                                    break;
                                // direct binding enabled
                                case DF_1_DIRECT:
                                    printf("DF_1_DIRECT");
                                    break;
                                case DF_1_TRANS:
                                    printf("DF_1_TRANS");
                                    break;
                                // object is used to interpose
                                case DF_1_INTERPOSE:
                                    printf("DF_1_INTERPOSE");
                                    break;
                                // ignore default lib search path
                                case DF_1_NODEFLIB:
                                    printf("DF_1_NODEFLIB");
                                    break;
                                // object can't be dldump'ed
                                case DF_1_NODUMP:
                                    printf("DF_1_NODUMP");
                                    break;
                                // configuration alternative created
                                case DF_1_CONFALT:
                                    printf("DF_1_CONFALT");
                                    break;
                                // filtee terminates filters search
                                case DF_1_ENDFILTEE:
                                    printf("DF_1_ENDFILTEE");
                                    break;
                                // disp reloc applied at build time
                                case DF_1_DISPRELDNE:
                                    printf("DF_1_DISPRELDNE");
                                    break;
                                // disp reloc applied at run-time
                                case DF_1_DISPRELPND:
                                    printf("DF_1_DISPRELPND");
                                    break;
                                // object has no-direct binding
                                case DF_1_NODIRECT:
                                    printf("DF_1_NODIRECT");
                                    break;
                                case DF_1_IGNMULDEF:
                                    printf("DF_1_IGNMULDEF");
                                    break;
                                case DF_1_NOKSYMS:
                                    printf("DF_1_NOKSYMS");
                                    break;
                                case DF_1_NOHDR:
                                    printf("DF_1_NOHDR");
                                    break;
                                // object is modified after built
                                case DF_1_EDITED:
                                    printf("DF_1_EDITED");
                                    break;
                                case DF_1_NORELOC:
                                    printf("DF_1_NORELOC");
                                    break;
                                // object has individual interposers
                                case DF_1_SYMINTPOSE:
                                    printf("DF_1_SYMINTPOSE");
                                    break;
                                // global auditing required
                                case DF_1_GLOBAUDIT:
                                    printf("DF_1_GLOBAUDIT");
                                    break;
                                // singleton symbols are used
                                case DF_1_SINGLETON:
                                    printf("DF_1_SINGLETON");
                                    break;
                                case DF_1_STUB:
                                    printf("DF_1_STUB");
                                    break;
                                case DF_1_PIE:
                                    printf("DF_1_PIE");
                                    break;
                                case DF_1_KMOD:
                                    printf("DF_1_KMOD");
                                    break;
                                case DF_1_WEAKFILTER:
                                    printf("DF_1_WEAKFILTER");
                                    break;
                                case DF_1_NOCOMMON:
                                    printf("DF_1_NOCOMMON");
                                    break;
                                default:
                                    // the flag is unknown
                                    printf("%#lx", flag);
                            }
                        }

                        if(color_opt)
                            printf("%s", C_END);

                        putchar('\n');
                    }
                    break;
                default:
                    if(color_opt)
                        printf(C_GREEN "%#lx\n" C_END, dyn.d_un.d_val);
                    else
                        printf("%#lx\n", dyn.d_un.d_val);
            }

            // stop when it's the end of the dynamic section
            if(dyn.d_tag == DT_NULL)
                break;

            putchar('\n');
        }

    }
}

// display the symbol table (option --symtab)
void show_symtab(Elf *elf) {
    Elf_Scn *section = NULL;
    size_t shstrndx;

    print_title("Symbol Table\n");

    // strlen("st_shndx")
    field_max_len = 8;

    // get the section index of the strtab
    if(elf_getshdrstrndx(elf, &shstrndx) != 0) {
        print_error("elf_getshdrstrndx() failed: %s", elf_errmsg(-1));
        exit(EXIT_FAILURE);
    }

    while((section = elf_nextscn(elf, section))) {
        GElf_Shdr shdr;
        Elf_Data *data = NULL;
        size_t num = 0;

        // get the section header
        if(!gelf_getshdr(section, &shdr)) {
            print_error("gelf_getshdr() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }
        // if it's not the symbol table section, skip the section
        if(shdr.sh_type != SHT_SYMTAB)
            continue;

        // get data from section
        data = elf_getdata(section, data);
        if(!data) {
            print_error("elf_getdata() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }

        num = shdr.sh_size / gelf_fsize(elf, ELF_T_SYM, 1, data->d_version);

        for(size_t i = 0; i < num; i++) {
            GElf_Sym sym;
            char *name = NULL;

            // get information from the dynamic symbol table
            if(!gelf_getsym(data, i, &sym)) {
                print_error("gelf_getsym() failed: %s", elf_errmsg(-1));
                exit(EXIT_FAILURE);
            }

            print_title("Elf_Sym %zu", i);

            // symbol name
            print_field("st_name", NULL);

            if(color_opt)
                printf(C_GREEN "%d" C_END, sym.st_name);
            else
                printf("%d", sym.st_name);

            name = elf_strptr(elf, shdr.sh_link, sym.st_name);
            if(!name || *name == '\0')
                putchar('\n');
            else
                printf(" (%s)\n", name);

            // symbol type and binding
            print_field("st_info", NULL);
            if(color_opt)
                printf(C_GREEN "%#x" C_END, sym.st_info);
            else
                printf("%#x", sym.st_info);

            printf(" (");

            // parse symbol type
            switch(GELF_ST_TYPE(sym.st_info)) {
                case STT_NOTYPE:
                    printf("STT_NOTYPE");
                    break;
                case STT_OBJECT:
                    printf("STT_OBJECT");
                    break;
                case STT_FUNC:
                    printf("STT_FUNC");
                    break;
                case STT_SECTION:
                    printf("STT_SECTION");
                    break;
                case STT_FILE:
                    printf("STT_FILE");
                    break;
                case STT_COMMON:
                    printf("STT_COMMON");
                    break;
                case STT_TLS:
                    printf("STT_TLS");
                    break;
                default:
                    printf("%#x", GELF_ST_TYPE(sym.st_info));

                    if((sym.st_info >= STT_LOPROC) && (sym.st_info <= STT_HIPROC))
                        printf(" processor-specific");
                    else if((sym.st_info >= STT_LOOS) && (sym.st_info <= STT_HIOS))
                        printf(" OS-specific");
                    else
                        printf(" unknown");
            }

            printf(", ");

            // parse symbol binding
            switch(GELF_ST_BIND(sym.st_info)) {
                case STB_LOCAL:
                    printf("STB_LOCAL");
                    break;
                case STB_GLOBAL:
                    printf("STB_GLOBAL");
                    break;
                case STB_WEAK:
                    printf("STB_WEAK");
                    break;
                default:
                    printf("%#x", GELF_ST_BIND(sym.st_info));

                    if((sym.st_info >= STB_LOPROC) && (sym.st_info <= STB_HIPROC))
                        printf(" processor-specific");
                    else if((sym.st_info >= STB_LOOS) && (sym.st_info <= STB_HIOS))
                        printf(" OS-specific");
                    else
                        printf(" unknown");
            }

            puts(")");

            // symbol visibility
            print_field("st_other", NULL);
            switch(GELF_ST_VISIBILITY(sym.st_other)) {
                case STV_DEFAULT:
                    print_field_info("STV_DEFAULT", "default symbol visibility rules");
                    break;
                case STV_INTERNAL:
                    print_field_info("STV_INTERNAL", "processor specific hidden class");
                    break;
                case STV_HIDDEN:
                    print_field_info("STV_HIDDEN", "sym unavailable in other modules");
                    break;
                case STV_PROTECTED:
                    print_field_info("STV_PROTECTED", "not preemptible, not exported");
                    break;
                default:
                    if(color_opt)
                        printf(C_GREEN "%#x" C_END, GELF_ST_VISIBILITY(sym.st_other));
                    else
                        printf("%#x", GELF_ST_VISIBILITY(sym.st_other));

                    puts(" (unknown)");
            }

            // section index
            print_field("st_shndx", NULL);
            // parse special section indices
            switch(sym.st_shndx) {
                case SHN_UNDEF:
                    print_field_info("SHN_UNDEF", "undefined section");
                    break;
                case SHN_BEFORE:
                    print_field_info("SHN_BEFORE", "order section before all others (Solaris)");
                    break;
                case SHN_AFTER:
                    print_field_info("SHN_AFTER", "order section after all others (Solaris)");
                    break;
                case SHN_ABS:
                    print_field_info("SHN_ABS", "associated symbol is absolute");
                    break;
                case SHN_COMMON:
                    print_field_info("SHN_COMMON", "associated symbol is common");
                    break;
                case SHN_XINDEX:
                    print_field_info("SHN_XINDEX", "index is in extra table");
                    break;
                default:
                    if(color_opt)
                        printf(C_GREEN "%d" C_END, sym.st_shndx);
                    else
                        printf("%d", sym.st_shndx);

                    if((sym.st_shndx >= SHN_LOPROC) && (sym.st_shndx <= SHN_HIPROC))
                        puts(" (processor-specific)");
                    else if((sym.st_shndx >= SHN_LOOS) && (sym.st_shndx <= SHN_HIOS))
                        puts(" (OS-specific)");
                    else if(sym.st_shndx >= SHN_LORESERVE)
                        puts(" (reserved indices)");
                    else {
                        Elf_Scn *section;
                        GElf_Shdr shdr;
                        char *name = NULL;

                        // get the section
                        section = elf_getscn(elf, sym.st_shndx);
                        if(!section) {
                            print_error("elf_getscn() failed: %s",
                                        elf_errmsg(-1));
                            exit(EXIT_FAILURE);
                        }

                        // get the section header
                        if(!gelf_getshdr(section, &shdr)) {
                            print_error("gelf_getshdr() failed: %s",
                                        elf_errmsg(-1));
                            exit(EXIT_FAILURE);
                        }

                        name = elf_strptr(elf, shstrndx, shdr.sh_name);
                        if(!name || *name == '\0')
                            putchar('\n');
                        else
                            printf(" (%s)\n", name);
                    }
            }

            // symbol value
            print_field("st_value", "%#lx", sym.st_value);

            // symbol size
            print_field("st_size", "%ld", sym.st_size);

            if(i + 1 != num)
                putchar('\n');
        }
    }
}

// display the dynamic symbol table (option --dyn-syms)
void show_dynamic_symtab(Elf *elf) {
    Elf_Scn *section = NULL;

    print_title("Dynamic Symbol Table\n");

    // strlen("st_shndx")
    field_max_len = 8;

    while((section = elf_nextscn(elf, section))) {
        GElf_Shdr shdr;
        Elf_Data *data = NULL;
        size_t num = 0;

        // get the section header
        if(!gelf_getshdr(section, &shdr)) {
            print_error("gelf_getshdr() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }

        // if it's not the dynamic symbol table section, skip the section
        if(shdr.sh_type != SHT_DYNSYM)
            continue;

        // get data from section
        data = elf_getdata(section, data);
        if(!data) {
            print_error("elf_getdata() failed: %s", elf_errmsg(-1));
            exit(EXIT_FAILURE);
        }

        num = shdr.sh_size / gelf_fsize(elf, ELF_T_SYM, 1, data->d_version);

        for(size_t i = 0; i < num; i++) {
            GElf_Sym sym;
            char *name = NULL;

            // get information from the dynamic symbol table
            if(!gelf_getsym(data, i, &sym)) {
                print_error("gelf_getsym() failed: %s", elf_errmsg(-1));
                exit(EXIT_FAILURE);
            }

            print_title("Elf_Sym %zu", i);

            // symbol name
            print_field("st_name", NULL);

            if(color_opt)
                printf(C_GREEN "%d" C_END, sym.st_name);
            else
                printf("%d", sym.st_name);

            name = elf_strptr(elf, shdr.sh_link, sym.st_name);
            if(!name || *name == '\0')
                putchar('\n');
            else
                printf(" (%s)\n", name);

            // symbol type and binding
            print_field("st_info", NULL);
            if(color_opt)
                printf(C_GREEN "%#x" C_END, sym.st_info);
            else
                printf("%#x", sym.st_info);

            printf(" (");

            // parse symbol type
            switch(GELF_ST_TYPE(sym.st_info)) {
                case STT_NOTYPE:
                    printf("STT_NOTYPE");
                    break;
                case STT_OBJECT:
                    printf("STT_OBJECT");
                    break;
                case STT_FUNC:
                    printf("STT_FUNC");
                    break;
                case STT_SECTION:
                    printf("STT_SECTION");
                    break;
                case STT_FILE:
                    printf("STT_FILE");
                    break;
                case STT_COMMON:
                    printf("STT_COMMON");
                    break;
                case STT_TLS:
                    printf("STT_TLS");
                    break;
                default:
                    printf("%#x", GELF_ST_TYPE(sym.st_info));

                    if((sym.st_info >= STT_LOPROC) && (sym.st_info <= STT_HIPROC))
                        printf(" processor-specific");
                    else if((sym.st_info >= STT_LOOS) && (sym.st_info <= STT_HIOS))
                        printf(" OS-specific");
                    else
                        printf(" unknown");
            }

            printf(", ");

            // parse symbol binding
            switch(GELF_ST_BIND(sym.st_info)) {
                case STB_LOCAL:
                    printf("STB_LOCAL");
                    break;
                case STB_GLOBAL:
                    printf("STB_GLOBAL");
                    break;
                case STB_WEAK:
                    printf("STB_WEAK");
                    break;
                default:
                    printf("%#x", GELF_ST_BIND(sym.st_info));

                    if((sym.st_info >= STB_LOPROC) && (sym.st_info <= STB_HIPROC))
                        printf(" processor-specific");
                    else if((sym.st_info >= STB_LOOS) && (sym.st_info <= STB_HIOS))
                        printf(" OS-specific");
                    else
                        printf(" unknown");
            }

            puts(")");

            // symbol visibility
            print_field("st_other", NULL);
            switch(GELF_ST_VISIBILITY(sym.st_other)) {
                case STV_DEFAULT:
                    print_field_info("STV_DEFAULT", "default symbol visibility rules");
                    break;
                case STV_INTERNAL:
                    print_field_info("STV_INTERNAL", "processor specific hidden class");
                    break;
                case STV_HIDDEN:
                    print_field_info("STV_HIDDEN", "sym unavailable in other modules");
                    break;
                case STV_PROTECTED:
                    print_field_info("STV_PROTECTED", "not preemptible, not exported");
                    break;
                default:
                    if(color_opt)
                        printf(C_GREEN "%#x" C_END, GELF_ST_VISIBILITY(sym.st_other));
                    else
                        printf("%#x", GELF_ST_VISIBILITY(sym.st_other));

                    puts(" (unknown)");
            }

            // section index
            print_field("st_shndx", NULL);
            // parse special section indices
            switch(sym.st_shndx) {
                case SHN_UNDEF:
                    print_field_info("SHN_UNDEF", "undefined section");
                    break;
                case SHN_BEFORE:
                    print_field_info("SHN_BEFORE", "order section before all others (Solaris)");
                    break;
                case SHN_AFTER:
                    print_field_info("SHN_AFTER", "order section after all others (Solaris)");
                    break;
                case SHN_ABS:
                    print_field_info("SHN_ABS", "associated symbol is absolute");
                    break;
                case SHN_COMMON:
                    print_field_info("SHN_COMMON", "associated symbol is common");
                    break;
                case SHN_XINDEX:
                    print_field_info("SHN_XINDEX", "index is in extra table");
                    break;
                default:
                    if(color_opt)
                        printf(C_GREEN "%d" C_END, sym.st_shndx);
                    else
                        printf("%d", sym.st_shndx);

                    if((sym.st_shndx >= SHN_LOPROC) && (sym.st_shndx <= SHN_HIPROC))
                        puts(" (processor-specific)");
                    else if((sym.st_shndx >= SHN_LOOS) && (sym.st_shndx <= SHN_HIOS))
                        puts(" (OS-specific)");
                    else if(sym.st_shndx >= SHN_LORESERVE)
                        puts(" (reserved indices)");
                    else
                        puts(" (unknown)");
            }

            // symbol value
            print_field("st_value", "%#lx", sym.st_value);

            // symbol size
            print_field("st_size", "%ld", sym.st_size);

            if(i + 1 != num)
                putchar('\n');
        }
    }
}

// display the help message
void usage(FILE *stream) {
    fprintf(stream,
            "Usage: elfy [options] FILE\n\n"
            "Options:\n"
            "  -h, --file-header      display the ELF file header\n"
            "  -p, --program-headers  display the program headers\n"
            "  -s, --section-headers  display the section headers\n"
            "  -d, --dynamic          display the dynamic section\n"
            "  --symtab               display the symbol table\n"
            "  --dyn-syms             display the dynamic symbol table\n"
            "  -a, --all              equivalent to -h -p -s -d --symtab --dyn-syms\n"
            "  -c, --color            colored output\n"
            "  --help                 display this information\n"
            "  --version              display the version number of elfy\n\n"
            "Report bugs to <https://github.com/xfgusta/elfy/issues>\n");
}

int main(int argc, char **argv) {
    int opt;
    int opt_index = 0;
    char *filename;
    Elf *elf;
    int fd;
    int is_first = 1;

    while((opt = getopt_long(argc, argv, "hpsdac", long_opts,
                             &opt_index)) != -1) {
        switch(opt) {
            case 'h':
                file_header_opt = 1;
                break;
            case 'p':
                program_headers_opt = 1;
                break;
            case 's':
                section_headers_opt = 1;
                break;
            case 'd':
                dynamic_section_opt = 1;
                break;
            case 'a':
                all_opt = 1;
                break;
            case 'c':
                color_opt = 1;
                break;
            case '?':
                exit(EXIT_FAILURE);
            default:
                break;
        }
    }

    // none of the options were used
    if(!(file_header_opt || program_headers_opt || section_headers_opt ||
         dynamic_section_opt || symtab_opt || dynamic_symtab_opt || all_opt ||
         help_opt || version_opt)) {
        usage(stderr);
        exit(EXIT_FAILURE);
    }

    if(help_opt) {
        usage(stdout);
        exit(EXIT_SUCCESS);
    } else if(version_opt) {
        printf("%s\n", ELFY_VERSION);
        exit(EXIT_SUCCESS);
    }

    if(!argv[optind]) {
        print_error("ELF file missing\n");
        exit(EXIT_FAILURE);
    }

    filename = argv[optind];

    fd = open(filename, O_RDONLY);
    if(fd < 0) {
        print_error("Cannot open %s failed: %s\n", filename, strerror(errno));
        exit(EXIT_FAILURE);
    }

    if(elf_version(EV_CURRENT) == EV_NONE) {
        print_error("ELF library initialization failed: %s\n", elf_errmsg(-1));
        exit(EXIT_FAILURE);
    }

    // read the elf
    elf = elf_begin(fd, ELF_C_READ, NULL);
    if(!elf) {
        print_error("elf_begin() failed: %s\n", elf_errmsg(-1));
        exit(EXIT_FAILURE);
    }

    // check if the file is an ELF
    if(elf_kind(elf) != ELF_K_ELF) {
        print_error("%s is not an ELF object\n", filename);
        exit(EXIT_FAILURE);
    }

    if (all_opt) {
        show_file_header(elf);
        putchar('\n');

        show_program_headers(elf);
        putchar('\n');

        show_section_headers(elf);
        putchar('\n');

        show_dynamic_section(elf);
        putchar('\n');

        show_symtab(elf);
        putchar('\n');

        show_dynamic_symtab(elf);
    } else {
        if(file_header_opt) {
            if(!is_first)
                putchar('\n');

            show_file_header(elf);
            is_first = 0;
        }

        if(program_headers_opt) {
            if(!is_first)
                putchar('\n');

            show_program_headers(elf);
            is_first = 0;
        }

        if(section_headers_opt) {
            if(!is_first)
                putchar('\n');

            show_section_headers(elf);
            is_first = 0;
        }

        if(dynamic_section_opt) {
            if(!is_first)
                putchar('\n');

            show_dynamic_section(elf);
            is_first = 0;
        }

        if(symtab_opt) {
            if(!is_first)
                putchar('\n');

            show_symtab(elf);
            is_first = 0;
        }

        if(dynamic_symtab_opt) {
            if(!is_first)
                putchar('\n');

            show_dynamic_symtab(elf);
        }
    }

    elf_end(elf);
    close(fd);

    exit(EXIT_SUCCESS);
}
