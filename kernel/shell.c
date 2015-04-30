#include <kernel.h>

WINDOW shell_wnd = {0, 9, 61, 16, 0, 0, 0xDC};
WINDOW pac_wnd = {61, 9, 61, 16, 0, 0, 0xDC};
WINDOW train_wnd = {35, 0, 40, 8, 0, 0, ' '};

#define MAX_CMD_LENGTH 32
char current_cmd[MAX_CMD_LENGTH];
int cmd_size = 0;

/*Command codes*/
#define SHELL_CMD_EMPTY   0
#define SHELL_CMD_HELP    1
#define SHELL_CMD_CLEAR   2
#define SHELL_CMD_PS      3
#define SHELL_CMD_TRAIN   4
#define SHELL_CMD_TSTOP   5
#define SHELL_CMD_TSTART  6
#define SHELL_CMD_TPOS    7
#define SHELL_CMD_TDIR    8
#define SHELL_CMD_PAC     9
#define SHELL_CMD_ABOUT   10

#define SHELL_CMD_INVALID 255

const char* cmd_table[] = {
	"help", "clear", "ps", "train", "tstop", 
	"tgo", "tpos","tdir", "pac", "about"
};

/********************* utils functions ***********************/

void print_helpers(WINDOW* wnd) {
	wprintf(wnd, "Available tos shell commands: \n");
	wprintf(wnd, "help  : show help of commands\n");
	wprintf(wnd, "clear : clear shell window\n");
	wprintf(wnd, "ps    : print all processes\n");
	wprintf(wnd, "train : start train application\n");
	wprintf(wnd, "tstop : stop train\n");
	wprintf(wnd, "tgo   : start train\n");
	wprintf(wnd, "tpos  : change positions of switches\n");
	wprintf(wnd, "tdir  : toggle direction of the train\n");
	wprintf(wnd, "pac   : start pacman game\n");
	wprintf(wnd, "about : show about\n");
}

void print_head(WINDOW* wnd) {
	
	wprintf(wnd, "  _____ ___  ___             |\n"); 
	wprintf(wnd, " |_   _/ _ \\/ __|            |\n");
	wprintf(wnd, "   | || (_) \\__ \\            |\n");
	wprintf(wnd, "   |_| \\___/|___/            |\n");
	wprintf(wnd, "                             |\n");

	wprintf(wnd, "Welcome to TOS shell window! |\nType 'help' to get helps.    |\n");       
	wprintf(wnd, "------------------------------\n");                 
}

/*********************** commands *****************************/

void cmd_empty() {
}

void cmd_help() {
	print_helpers(&shell_wnd);
}

void cmd_clear() {
	clear_window(&shell_wnd);
}

void cmd_ps() {
	print_all_processes(&shell_wnd);
}

void cmd_train() {
	init_train(&train_wnd);
}

void cmd_tstop() {
	train_command("L20S0");
}

void cmd_tgo() {
	train_command("L20S5");
}

void cmd_tpos() {
	wprintf(&shell_wnd, "not implemented yet\n");
}

void cmd_tdir() {
	train_command("L20D");
}

void cmd_pac() {
	init_pacman(&pac_wnd, 2);
}

void cmd_about() {
	wprintf(&shell_wnd, "This program is a term project of CSC720 Advanced Operating System class at San Francisco State University.\n");
}

void cmd_invalid() {
	wprintf(&shell_wnd, "command not found!\n");
}

/** function pointer table **/
typedef void (*cmd_func)();
cmd_func cmd_functions[] = {
	cmd_empty, cmd_help, cmd_clear, 
	cmd_ps, cmd_train, cmd_tstop, cmd_tgo, 
	cmd_tpos, cmd_tdir, cmd_pac, cmd_about
};

/********************** shell codes ***************************/

BOOL compare_cmd(const char* cmd) {
	const char* pos = cmd;
	int len = 0;
	while(pos && len < cmd_size) {
		if(current_cmd[len++] != *pos++) return FALSE;
	}
	return len == cmd_size && !*pos;
}

int get_cmd_code() {
	if (0 == cmd_size) {
		return SHELL_CMD_EMPTY;
	}
	int table_size = sizeof(cmd_table) / sizeof(char*);
	int i = 1;
	for(; i <= table_size; ++i) {
		if(compare_cmd(cmd_table[i-1])) return i;
	}
	return SHELL_CMD_INVALID;
}

void execute_cmd() {
	wprintf(&shell_wnd, "\n");
	int code = get_cmd_code();
	if(SHELL_CMD_INVALID == code) 
		cmd_invalid();
	else {
		cmd_functions[code]();
	}
}

void shell_process(PROCESS self, PARAM param) {
    char ch;
    Keyb_Message msg;

    wprintf(&shell_wnd, "> ");

	while (1) {
		msg.key_buffer = &ch; 
		send(keyb_port, &msg); 
	
		if (ch == 8) { /* backspace */
			if(cmd_size > 0) { 
				--cmd_size;
				wprintf(&shell_wnd, "%c", ch);
			}
		} else if(ch == 13) { /* return */
			/* execute command */
			execute_cmd();
			cmd_size = 0;
			wprintf(&shell_wnd, "> ");
		} else if(cmd_size < MAX_CMD_LENGTH) {
			current_cmd[cmd_size++] = ch;
			wprintf(&shell_wnd, "%c", ch);
		}
		
	}
}

void init_shell() {
	clear_window(kernel_window);
	print_head(kernel_window);
    create_process(shell_process, 5, 0, "Shell Process");
    resign();
}