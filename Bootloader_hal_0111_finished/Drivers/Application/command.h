#ifndef __COMMMAND_H
#define __COMMMAND_H



struct command {
  char *name;        // 命令的名字，比如 "help"
  char *short_help;  // 短帮助
  char *long_help;   // 长帮助
  
  // 【关键】这是一个函数指针！
  // 它指向一个返回值为 int，参数为 (int, char**) 的函数
  int (*function)(int argc, char **argv); 
};

struct command ** get_cmds(void);
struct command *find_cmd(char *name);


#endif
