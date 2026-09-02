/***************************************************************************************
* Copyright (c) 2014-2024 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm();

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
// Log("Exercise: Please remove me in the source code and compile NEMU again.");
// assert(0);
}

#ifndef CONFIG_TARGET_AM
  #include <getopt.h>
  #ifdef CONFIG_FTRACE
    #include <elf.h>      
  #endif

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static char *elf_file = NULL;
static int difftest_port = 1234;

#ifdef CONFIG_FTRACE
  typedef struct {
    word_t value;
    word_t size;
    const char *name;
  } FuncSymbol;
  static FuncSymbol *funcs = NULL;
  static int func_cnt = 0;
  static int func_depth = 0;

  static void init_ftrace(const char *elf_file){
    if (elf_file == NULL) {
      Log("ftrace: No ELF file is given. .");
      return;
    }
    FILE *fp = fopen(elf_file, "rb");
    Assert(fp, "ftrace can not open '%s'", elf_file);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    
    uint8_t *elf_buf = malloc(size);
    assert(elf_buf);
    ret = fread(elf_buf, size, 1, fp);
    assert(ret == 1);
    fclose(fp);

    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)elf_buf;
    Assert(memcmp(ehdr->e_ident, ELFMAG, SELFMAG) == 0, "ftrace:%s is not a valid ELF file", elf_file);
    Elf32_Shdr *shdr = (Elf32_Shdr *)(elf_buf + ehdr->e_shoff);
    Elf32_Shdr *symtab = NULL;
    Elf32_Shdr *strtab = NULL;
    for (int i = 0; i < ehdr->e_shnum; i ++) {
      if (shdr[i].sh_type == SHT_SYMTAB) {
        symtab = &shdr[i];
        strtab = &shdr[symtab->sh_link];
        break;
      }
    }
    Assert(symtab != NULL, "ftrace: no symbol table in '%s'", elf_file);

    Elf32_Sym *sym = (Elf32_Sym *)(elf_buf + symtab->sh_offset);
    int symtab_cnt = symtab->sh_size / sizeof(Elf32_Sym);
    const char *strtab_base = (const char *)(elf_buf + strtab->sh_offset);
    funcs = malloc(symtab_cnt * sizeof(FuncSymbol));
    for (int i = 0; i < symtab_cnt; i ++) {
      if (ELF32_ST_TYPE(sym[i].st_info) == STT_FUNC) {
        funcs[func_cnt].value = sym[i].st_value;
        funcs[func_cnt].size  = sym[i].st_size;
        funcs[func_cnt].name  = strtab_base + sym[i].st_name;
        func_cnt ++;
      }
    }
    Log("ftrace: loaded %d function symbols from %s", func_cnt, elf_file);
  }
  
  static const char *find_func(word_t addr, word_t *value) {
    for (int i = 0; i < func_cnt; i ++) {
      if (addr >= funcs[i].value && addr < funcs[i].value + funcs[i].size) {
        *value = funcs[i].value;
        return funcs[i].name;
      }
    }
    return NULL;
  }
  void ftrace_call(vaddr_t pc, vaddr_t target) {
    word_t value = target;
    const char *name = find_func(target, &value);
    if (name == NULL) name = "???";
    log_write(FMT_WORD ": %*scall [%s@" FMT_WORD "]\n", pc, func_depth * 2, "", name, value);
    func_depth ++;
  }
  void ftrace_ret(vaddr_t pc) {
    if (func_depth > 0) func_depth --;
    word_t value = pc;
    const char *name = find_func(pc, &value);
    if (name == NULL) name = "???";
    log_write(FMT_WORD ": %*sret  [%s]\n", pc, func_depth * 2, "", name);
  }
#endif


static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  assert(ret == 1);

  fclose(fp); 
  return size;
}

static int parse_args(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"elf"      , required_argument, NULL, 'e'},
    {"port"     , required_argument, NULL, 'p'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:e:p:h:", table, NULL)) != -1) {
    switch (o) {
      case 'b': sdb_set_batch_mode(); break;
      case 'e': elf_file = optarg; break;
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 1: img_file = optarg; return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-e,--elf=ELF_FILE        ELF file for ftrace\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Initialize ftrace. */
  IFDEF(CONFIG_FTRACE, init_ftrace(elf_file));

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger. */
  init_sdb();

  IFDEF(CONFIG_ITRACE, init_disasm());

  /* Display welcome message. */
  welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
