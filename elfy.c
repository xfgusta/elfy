#include <stdio.h>
#include <stdbool.h>
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

#define ELFY_VERSION "elfy-v0.1.0"

#define TAB      "    "
#define C_RED    "\033[31m"
#define C_GREEN  "\033[32m"
#define C_YELLOW "\033[33m"
#define C_END    "\033[0m"

bool color_opt = false;

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

/* print field name and its value
   add spaces between name and value based on field_max_len
   don't print value if value is NULL
*/
void print_field(char *field, char *value, ...) {
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

/* print field value and its info inside parentheses
   e.g.: ET_DYN (shared object file)
*/
void print_field_info(char *value, char *info) {
    if(color_opt)
        printf(C_GREEN "%s" C_END " (%s)\n", value, info);
    else
        printf("%s (%s)\n", value, info);
}

// display the elf file header (option -h)
void show_file_header(Elf *elf) {
    GElf_Ehdr ehdr;

    // strlen("EI_ABIVERSION")
    field_max_len = 13;

    // get the elf file header
    if(!gelf_getehdr(elf, &ehdr)) {
        fprintf(stderr, "gelf_getehdr() failed: %s", elf_errmsg(-1));
        exit(1);
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
            print_field_info("ET_NONE", "no file type");
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
            fprintf(stderr, "elf_getscn() failed: %s", elf_errmsg(-1));
            exit(1);
        }

        if(!gelf_getshdr(section, &shdr)) {
            fprintf(stderr, "gelf_getshdr() failed: %s", elf_errmsg(-1));
            exit(1);
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
            print_field_info("ELFCLASS32", "32-bit objects");
            break;
        case ELFCLASS64:
            print_field_info("ELFCLASS64", "64-bit objects");
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
            print_field_info("ELFOSABI_GNU", "Object uses GNU ELF extensions");
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
            print_field_info("ELFOSABI_STANDALONE", "Standalone (embedded) application");
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

    // strlen("p_filesz")
    field_max_len = 8;

    // get the number of program headers
    if(elf_getphdrnum(elf, &num) != 0) {
        fprintf(stderr, "elf_getphdrnum() failed: %s", elf_errmsg(-1));
        exit(1);
    }

    for(size_t i = 0; i < num; i++) {
        GElf_Phdr phdr;

        // get the program header
        if(!gelf_getphdr(elf, i, &phdr)) {
            fprintf(stderr, "gelf_getphdr() failed: %s", elf_errmsg(-1));
            exit(1);
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

    // strlen("sh_addralign")
    field_max_len = 12;

    // get the number of section headers
    if(elf_getshdrnum(elf, &num) != 0) {
        fprintf(stderr, "elf_getshdrnum() failed: %s", elf_errmsg(-1));
        exit(1);
    }

    // get the section index of the strtab
    if(elf_getshdrstrndx(elf, &shstrndx) != 0) {
        fprintf(stderr, "elf_getshdrstrndx() failed: %s", elf_errmsg(-1));
        exit(1);
    }

    for(size_t i = 0; i < num; i++) {
        Elf_Scn *section = NULL;
        GElf_Shdr shdr;
        char *name = NULL;

        // get the section
        section = elf_getscn(elf, i);
        if(!section) {
            fprintf(stderr, "elf_getscn() failed: %s", elf_errmsg(-1));
            exit(1);
        }

        // get the section header
        if(!gelf_getshdr(section, &shdr)) {
            fprintf(stderr, "gelf_getshdr() failed: %s", elf_errmsg(-1));
            exit(1);
        }

        // get the section name from strtab
        name = elf_strptr(elf, shstrndx, shdr.sh_name);
        if(!name) {
            fprintf(stderr, "elf_strptr() failed: %s", elf_errmsg(-1));
            exit(1);
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
        print_field("sh_flags", "%#lx", shdr.sh_flags);

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

    // strlen("d_val")
    field_max_len = 5;

    sh_entsize = gelf_fsize(elf, ELF_T_DYN, 1, EV_CURRENT);

    while((section = elf_nextscn(elf, section))) {
        GElf_Shdr shdr;
        Elf_Data *data = NULL;
        size_t num = 0;

        // get the section header
        if(!gelf_getshdr(section, &shdr)) {
            fprintf(stderr, "gelf_getshdr() failed: %s", elf_errmsg(-1));
            exit(1);
        }

        // if it's not a dynamic section, skip the section
        if(shdr.sh_type != SHT_DYNAMIC)
            continue;

        num = shdr.sh_size / sh_entsize;

        // get data from section
        data = elf_getdata(section, data);
        if(!data) {
            fprintf(stderr, "elf_getdata() failed: %s", elf_errmsg(-1));
            exit(1);
        }

        for(size_t i = 0; i < num; i++) {
            GElf_Dyn dyn;
            char *name = NULL;

            // get information from the dynamic table 
            if(!gelf_getdyn(data, i, &dyn)) {
                fprintf(stderr, "gelf_getdyn() failed: %s", elf_errmsg(-1));
                exit(1);
            }

            print_title("Elf_Dyn %d", i);

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
            if(color_opt)
                printf(C_GREEN "%#lx" C_END, dyn.d_un.d_val);
            else
                printf("%#lx", dyn.d_un.d_val);

            // print library name
            if(dyn.d_tag == DT_NEEDED || dyn.d_tag == DT_SONAME) {
                name = elf_strptr(elf, shdr.sh_link, dyn.d_un.d_val);
                if(!name || *name == '\0')
                    putchar('\n');
                else
                    printf(" (%s)\n", name);
            } else
                putchar('\n');

            // stop when it's the end of the dynamic section
            if(dyn.d_tag == DT_NULL)
                break;

            putchar('\n');
        }

    }
}

// display the help message
void usage(FILE *stream) {
    fprintf(stream,
           "Usage: elfy [option] FILE\n\n"
           "Options:\n"
           "  -h        display the ELF file header\n"
           "  -p        display the program headers\n"
           "  -s        display the section headers\n"
           "  -d        display the dynamic section\n"
           "  -c        colored output\n"
           "  -H        display this information\n"
           "  -v        display the version number of elfy\n\n"
           "Report bugs to <https://github.com/xfgusta/elfy/issues>\n");
}

int main(int argc, char **argv) {
    int opt;
    bool file_header_opt = false;
    bool program_headers_opt = false;
    bool section_headers_opt = false;
    bool dynamic_section_opt = false;
    char *filename;
    Elf *elf;
    int fd;

    while((opt = getopt(argc, argv, "Hvhcpsd")) != -1) {
        switch(opt) {
            case 'H':
                usage(stdout);
                exit(0);
            case 'v':
                printf("%s\n", ELFY_VERSION);
                exit(0);
            case 'c':
                color_opt = true;
                break;
            case 'h':
                file_header_opt = true;
                break;
            case 'p':
                program_headers_opt = true;
                break;
            case 's':
                section_headers_opt = true;
                break;
            case 'd':
                dynamic_section_opt = true;
                break;
            case '?':
                exit(1);
            default:
                break;
        }
    }

    // none of the options were used
    if(!(file_header_opt || program_headers_opt || section_headers_opt ||
         dynamic_section_opt)){
        usage(stderr);
        exit(1);
    }

    if(!argv[optind]) {
        fprintf(stderr, "ELF file missing\n");
        exit(1);
    }

    filename = argv[optind];

    fd = open(filename, O_RDONLY);
    if(fd < 0) {
        fprintf(stderr, "Cannot open %s failed: %s\n", filename,
                strerror(errno));
        exit(1);
    }

    if(elf_version(EV_CURRENT) == EV_NONE) {
        fprintf(stderr, "ELF library initialization failed: %s\n",
                elf_errmsg(-1));
        exit(1);
    }

    // read the elf
    elf = elf_begin(fd, ELF_C_READ, NULL);
    if(!elf) {
        fprintf(stderr, "elf_begin() failed: %s\n", elf_errmsg(-1));
        exit(1);
    }

    // check if the file is an ELF
    if(elf_kind(elf) != ELF_K_ELF) {
        fprintf(stderr, "%s is not an ELF object\n", filename);
        exit(1);
    }

    if(file_header_opt)
        show_file_header(elf);
    else if(program_headers_opt)
        show_program_headers(elf);
    else if(section_headers_opt)
        show_section_headers(elf);
    else if(dynamic_section_opt)
        show_dynamic_section(elf);

    elf_end(elf);
    close(fd);

    exit(0);
}
