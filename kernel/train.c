
#include <kernel.h>

WINDOW* output_wnd = NULL;

BOOL train_command(const char* cmd) {
	int len = k_strlen(cmd);
	if (len >= 15) return FALSE;

	char out_buf[15]; /*maximum size 15*/
	char in_buf[3];

	COM_Message msg;
	msg.output_buffer = out_buf;
	msg.input_buffer = in_buf;
	msg.len_input_buffer = cmd[0] == 'C' ? 3 : 0;

	k_memcpy(out_buf, cmd, len);
	out_buf[len] = '\015';
	out_buf[len+1] = '\0';

	send(com_port, &msg);

	if (cmd[0] == 'C') {
		return msg.input_buffer[1] == '1';
	} else {
		return FALSE;
	}
}

void clean_buffer() {
    train_command("R");
}

BOOL probe(int number) {
	if (number > 15 || number <= 0) {
		wprintf(output_wnd, "Can't probe, bad number!\n");
		return;
	}

	char probe_msg[5];
	if (number >= 10) {
		probe_msg[0] = 'C';
		probe_msg[1] = '1';
		probe_msg[2] = '0' + number % 10;
		probe_msg[3] = '\015';
		probe_msg[4] = '\0';
	} else {
		probe_msg[0] = 'C';
		probe_msg[1] = '0' + number;
		probe_msg[2] = '\015';
		probe_msg[3] = '\0';
	}

    return train_command(probe_msg);
}

/***************************
  Run the train application
****************************/

void train_process(PROCESS self, PARAM param) {
	clear_window(output_wnd);
	
	train_command("L20S5");
	
	wprintf(output_wnd, "Train process initialized.");	

	while (1) {
	    //sleep(10);
	    //send_train_msg("L20S5\015");
	    //wprintf(output_wnd, "come here");
	}
}

void init_train(WINDOW* wnd) {
	output_wnd = wnd;	
	create_process(train_process, 3, 0, "Train Process");
	return;
}
