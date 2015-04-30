
#include <kernel.h>

WINDOW* output_wnd = NULL;

#define CMD_SLEEP_TIME 20

#define TRAIN_CONFIG_1 0x0001
#define TRAIN_CONFIG_2 0x0010
#define TRAIN_CONFIG_3 0x0100
#define TRAIN_CONFIG_4 0x1000

BOOL has_zamboni = FALSE;
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
		sleep(8);
		if(probe(nb) == on) {
      		return;
      	}
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
	if (number > 15 || number <= 0) {
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
	has_zamboni = probe(4);

	if(probe(8)) {
		/* Config 1 or 2*/
		current_config = TRAIN_CONFIG_1 | TRAIN_CONFIG_2;
		wprintf(output_wnd, "Configuration 1 or 2");
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
	}
}

void run_config_1_z() {

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

void run_config_4() {
	change_switch(3, 'R');
	change_switch(4, 'R');
	change_switch(5, 'G');
	change_switch(8, 'G');
	change_switch(9, 'G');
	train_command("L20S5");

	wait_until_on_contact(14, TRUE);
	sleep(350);

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

/***************************
  Run the train application
****************************/

void train_process(PROCESS self, PARAM param) {
	clear_window(output_wnd);	

	wprintf(output_wnd, "Train process initialized.\n");	

	check_config();

	while (1) {
	    sleep(10);
	    //send_train_msg("L20S5\015");
	    //wprintf(output_wnd, "come here");
	}
}

void init_train(WINDOW* wnd) {
	output_wnd = wnd;	
	create_process(train_process, 3, 0, "Train Process");
	return;
}
