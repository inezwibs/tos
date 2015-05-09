#include <kernel.h>

/* Window definitions */
WINDOW shell_wnd = {0, 9, 61, 16, 0, 0, 0xDC};
WINDOW pac_wnd = {61, 9, 61, 16, 0, 0, 0xDC};
WINDOW train_wnd = {35, 0, 40, 8, 0, 0, ' '};

/* Command buffer */
#define MAX_CMD_LENGTH 32
char current_cmd[MAX_CMD_LENGTH];

/* Command parameter buffer */
#define MAX_CMD_PARAM_LENGTH 5
char cmd_param[MAX_CMD_PARAM_LENGTH];

/* Length of the current command */
int cmd_size = 0;

/* Length of the current command parameter */
int cmd_param_size = 0;

extern BOOL train_running;

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

/* Commands */
const char* cmd_table[] = {
	"help", "clear", "ps", "train", "tstop", 
	"tgo", "ts","tdir", "pac", "about"
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
	wprintf(wnd, "ts    : get the status of a contact\n");
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

	wprintf(wnd, "Welcome to TOS shell window! |\n");
	wprintf(wnd, "Type 'help' to get helps.    |\n");       
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
	if (!train_running) {
		init_train(&train_wnd);
	} else {
        wprintf(&shell_wnd, "Sorry, the train process is running.\n");
	}
}

void cmd_tstop() {
	train_command("L20S0");
}

void cmd_tgo() {
	train_command("L20S5");
}

void cmd_ts() {
	if(cmd_param_size != 1 && cmd_param_size != 2) {
		wprintf(&shell_wnd, "Invalid parameter!\n");
		return;
	}
	int nb = 0;
	nb = cmd_param[0] - '0';
	if (cmd_param_size == 2) {
		nb = nb * 10 + cmd_param[1] - '0';
	}
	if(nb > 15 || nb <= 0) {
		wprintf(&shell_wnd, "Invalid parameter!\n");
		return;
	}
	BOOL ret = probe(nb);
	wprintf(&shell_wnd, "%s %d.\n", ret ? "On contact" : "Not on contact", nb);
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
	cmd_ts, cmd_tdir, cmd_pac, cmd_about
};

/********************** shell codes ***************************/
/* Compare the "cmd" with current command buffer */
BOOL compare_cmd(const char* cmd) {
	const char* pos = cmd;
	int len = 0;
	int cmd_len = k_strlen(cmd);
	while(*pos && len < cmd_size) {
		if(current_cmd[len++] != *pos++) {
			return FALSE;
		}
	}
	return *pos == '\0' ? current_cmd[len] == ' ' || len == cmd_size : 
		len == cmd_len;
}

/* Get the code of the current command buffer */
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

/* Process the shell parameter */
void process_param() {
	cmd_param_size = 0;
	int i = 0;
	BOOL start_param = FALSE;
	for(; i < cmd_size; ++i) {
		if(!start_param && current_cmd[i] == ' ') {
			start_param = TRUE;
		} else if(start_param) {
			if(current_cmd[i] != ' ') {
				cmd_param[cmd_param_size++] = current_cmd[i];
			} else {				
				break;
			}
		}
	}
	cmd_param[cmd_param_size] = '\0';
}

/* Execute command saved in command buffer */
void execute_cmd() {
	wprintf(&shell_wnd, "\n");
	int code = get_cmd_code();
	process_param();
	if(SHELL_CMD_INVALID == code) {
		cmd_invalid();
	} else {
		cmd_functions[code]();
	}
}

/* Entry function of shell process */
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

/* Initialize the TOS shell */
void init_shell() {
	clear_window(kernel_window);
	print_head(kernel_window);
    create_process(shell_process, 5, 0, "Shell Process");
    resign();
}