
#include <kernel.h>

WINDOW* output_wnd = NULL;
PORT train_port;
BOOL train_running = FALSE;

#define CMD_SLEEP_TIME 10
#define CONFIG_4_LAST_SLEEP 420
#define CHECK_ZAMBONI_SLEEP 50

#define TRAIN_CONFIG_1 0x0001
#define TRAIN_CONFIG_2 0x0010
#define TRAIN_CONFIG_3 0x0100
#define TRAIN_CONFIG_4 0x1000

BOOL has_zamboni = FALSE;
BOOL zamboni_dir_left = TRUE;
unsigned short current_config = 0;

BOOL train_command(const char* cmd) {
	int len = k_strlen(cmd);
	if (len >= 15) return FALSE;

	/*maximum size 15*/
	char out_buf[15];
	char in_buf[3];

	COM_Message msg;
	msg.output_buffer = out_buf;
	msg.input_buffer = in_buf;
	msg.len_input_buffer = cmd[0] == 'C' ? 3 : 0;

	k_memcpy(out_buf, cmd, len);
	out_buf[len] = '\015';
	out_buf[len+1] = '\0';

	sleep(CMD_SLEEP_TIME);
	send(com_port, &msg);

	if (cmd[0] == 'C') {
		return msg.input_buffer[1] == '1';
	} else {
		return FALSE;
	}
}

void wait_until_on_contact(int nb, BOOL on) {  
	while (42) {
		if(probe(nb) == on) return;
   	}
}

void clean_buffer() {
    train_command("R");
}

void change_switch(int number, char color) {
	if (number > 9 || number <= 0) {
		wprintf(output_wnd, "Can't change switch, bad number!\n");
		return;
	}
	char buf[4];
	buf[0] = 'M';
	buf[1] = '0' + number;
	buf[2] = color;
	buf[3] = '\0';
	train_command(buf);
}

BOOL probe(int number) {
	if (number > 16 || number <= 0) {
		wprintf(output_wnd, "Can't probe, bad number!\n");
		return;
	}

	char probe_msg[4];
	probe_msg[0] = 'C';

	if (number >= 10) {
		probe_msg[1] = '1';
		probe_msg[2] = '0' + number % 10;
		probe_msg[3] = '\0';
	} else {
		probe_msg[1] = '0' + number;
		probe_msg[2] = '\0';
	}

	train_command("R");	
    return train_command(probe_msg);
}

int check_config() {
	wprintf(output_wnd, "Checking the configuration...\n");

	sleep(CHECK_ZAMBONI_SLEEP);
	if(probe(3)) {
		has_zamboni = TRUE;
		zamboni_dir_left = FALSE;
	} else {
		if(probe(6)) {
			has_zamboni = TRUE;
			zamboni_dir_left = TRUE;
		} else {
			has_zamboni = FALSE;
		}
	}

	if(probe(8)) {
		/* Config 1 or 2*/
		current_config = TRAIN_CONFIG_1 | TRAIN_CONFIG_2;
		if (has_zamboni) {
			wprintf(output_wnd, "Configuration %s", zamboni_dir_left?"1":"2");		
		} else {
			wprintf(output_wnd, "Configuration 1 or 2");
		}
	} else if(probe(11)) {
		/* Config 3 */
		current_config = TRAIN_CONFIG_3;
		wprintf(output_wnd, "Configuration 3");
	} else {
		/* Config 4 */
		current_config = TRAIN_CONFIG_4;
		wprintf(output_wnd, "Configuration 4");
	}

	wprintf(output_wnd, " %s.\n", has_zamboni ? 
		"with Zamboni" : "without Zamboni");

	if (!has_zamboni) {
		if(current_config & TRAIN_CONFIG_3) {
			run_config_3();
		} else if(current_config & TRAIN_CONFIG_4) {
			run_config_4();
		} else if(current_config & (TRAIN_CONFIG_1 | TRAIN_CONFIG_2)) {
			run_config_1_or_2();
		}
	} else {
		zamboni_preprocess();

		if(current_config & (TRAIN_CONFIG_1 | TRAIN_CONFIG_2)) {
			run_config_1_or_2();
		} else if(current_config & TRAIN_CONFIG_3) {
			run_config_3_zam();
		} else {
			run_config_4_zam();
		}
	}
}

void zamboni_preprocess() {
	change_switch(1, 'G');
	change_switch(8, 'G');
	change_switch(5, 'G');
	change_switch(4, 'G');
	change_switch(7, 'R');
	change_switch(2, 'R');

	wait_until_on_contact(zamboni_dir_left ? 13 : 14, TRUE);

	change_switch(9, 'R');
	change_switch(1, 'R');
	change_switch(8, 'R');
}

void run_config_1_or_2() {
	change_switch(6, 'R');
	change_switch(5, 'R');
	change_switch(4, 'R');
	change_switch(3, 'G');
	train_command("L20S5");

	wait_until_on_contact(1, TRUE);
	train_command("L20S0");
	train_command("L20D");
	train_command("L20S5");

	wait_until_on_contact(8, TRUE);
	train_command("L20S0");

	wprintf(output_wnd, "We are done!\n");	
}

void run_config_3() {
	change_switch(3, 'R');
	change_switch(4, 'R');
	change_switch(5, 'R');
	change_switch(6, 'G');
	change_switch(7, 'G');
	train_command("L20S5");

	wait_until_on_contact(12, TRUE);
	train_command("L20S0");
	train_command("L20D");

	change_switch(7, 'R');
	change_switch(8, 'R');
	train_command("L20S5");

	wait_until_on_contact(13, TRUE);
	train_command("L20S0");
	train_command("L20D");

	change_switch(8, 'G');
	train_command("L20S5");

	wait_until_on_contact(5, TRUE);
	train_command("L20S0");

	wprintf(output_wnd, "We are done!\n");	
}

void run_config_3_zam() {
	change_switch(3, 'R');
	change_switch(4, 'R');
	change_switch(5, 'R');
	change_switch(6, 'G');
	
	train_command("L20S5");

	wait_until_on_contact(14, TRUE);
	change_switch(1, 'G');

	wait_until_on_contact(12, TRUE);
	sleep(100);
	train_command("L20S0");
	train_command("L20D");

	change_switch(7, 'R');
	change_switch(8, 'R');
	train_command("L20S5");

	wait_until_on_contact(13, TRUE);
	train_command("L20S0");
	train_command("L20D");

	change_switch(8, 'G');
	train_command("L20S5");

	wait_until_on_contact(5, TRUE);
	train_command("L20S0");

	change_switch(4, 'G');

	wprintf(output_wnd, "We are done!\n");	
}

void run_config_4() {
	change_switch(3, 'R');
	change_switch(4, 'R');
	change_switch(5, 'G');
	change_switch(8, 'G');
	change_switch(9, 'G');
	train_command("L20S5");

	wait_until_on_contact(14, TRUE);
	sleep(CONFIG_4_LAST_SLEEP);

	train_command("L20S0");
	train_command("L20D");
	train_command("L20S5");

	wait_until_on_contact(7, TRUE);
	train_command("L20S0");

	change_switch(5, 'R');
	change_switch(6, 'R');

	train_command("L20D");
	train_command("L20S5");

	wait_until_on_contact(8, TRUE);
	sleep(50);
	train_command("L20S0");

	wprintf(output_wnd, "We are done!\n");	
}

void run_config_4_zam() {
	change_switch(3, 'R');
	train_command("L20S5");

	wait_until_on_contact(6, TRUE);
	train_command("L20S0");
	train_command("L20D");
	train_command("L20S5");

	change_switch(4, 'G');
	change_switch(1, 'G');
	change_switch(8, 'G');

	wait_until_on_contact(10, TRUE);
	train_command("L20S0");
	train_command("L20D");
	train_command("L20S5");
	change_switch(9, 'G');

	wait_until_on_contact(14, TRUE);
	sleep(CONFIG_4_LAST_SLEEP);
	train_command("L20S0");
	train_command("L20D");
	train_command("L20S5");

	wait_until_on_contact(10, TRUE);
	wait_until_on_contact(7, TRUE);
	train_command("L20S0");

	change_switch(5, 'R');
	change_switch(6, 'R');

	train_command("L20D");
	train_command("L20S5");

	wait_until_on_contact(8, TRUE);
	sleep(50);
	train_command("L20S0");

	wprintf(output_wnd, "We are done!\n");	
}

/***************************
  Run the train application
****************************/

void train_process(PROCESS self, PARAM param) {
	train_running = TRUE;
	clear_window(output_wnd);	

	wprintf(output_wnd, "Train process initialized.\n");	

	check_config();

	train_running = FALSE;
	remove_ready_queue(active_proc);
    resign();
}

void init_train(WINDOW* wnd) {
	output_wnd = wnd;	
	train_port = create_process(train_process, 3, 0, "Train Process");
	return;
}
