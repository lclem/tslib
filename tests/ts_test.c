/*
 *  tslib/src/ts_test.c
 *
 *  Copyright (C) 2001 Russell King.
 *
 * This file is placed under the GPL.  Please see the file
 * COPYING for more details.
 *
 * SPDX-License-Identifier: GPL-2.0+
 *
 *
 * Basic test program for touchscreen library.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <ctype.h> 

#include "tslib.h"
#include "fbutils.h"
#include "testutils.h"

static int palette[] = {
	0x000000, 0xffe080, 0xffffff, 0xe0c0a0, 0x304050, 0x80b8c0, 0x00FF00, 0xFF0000
};
#define NR_COLORS (sizeof (palette) / sizeof (palette [0]))

#define NR_BUTTONS 6
static struct ts_button buttons[NR_BUTTONS];

#define NR_KEYBOARD_BUTTONS 72
static struct ts_button keyboardButtons[NR_KEYBOARD_BUTTONS];

// 0 = manage, 1 = scan
unsigned int program_mode = 0;

static void sig(int sig)
{
	close_framebuffer();
	fflush(stderr);
	printf("signal %d caught\n", sig);
	fflush(stdout);
	exit(1);
}

#define INPUT_TEXT_MAX_LEN 128
static char inputText[INPUT_TEXT_MAX_LEN];

unsigned int shift_on = 0;

void drawInputBox() {

	unsigned int x = 10;
	unsigned int y = 60;
	unsigned int width = xres - 20;
	unsigned int height = 40;
	
	// start from a black base
	fillrect(x, y, x + width, y + height, 0);
	rect(x, y, x + width, y + height, 3);
	put_string(x + 10, y + 10, inputText, 2);

}

void exec(const char* cmd) {
    char buffer[128];
    // string result = "";
    FILE* pipe = popen(cmd, "r");

    if (!pipe)
		perror("popen");
    
	int i = 0;
	
	while (fgets(buffer, sizeof buffer, pipe) != NULL) {
		printf(buffer);
	}

	pclose(pipe);
}

void exec_str(char *str, const char* cmd) {
    char buffer[128];
    // string result = "";
    FILE* pipe = popen(cmd, "r");

    if (!pipe)
		perror("popen");
    
	int i = 0;
	
	if (fgets(buffer, sizeof buffer, pipe) != NULL) {
		// printf(buffer);
		strcpy(str, buffer);
	}

	pclose(pipe);
}

#define NR_WPA_ENTRIES 32

struct wpa_entry {

	char ssid[128];
	char psk[128];

	int y;
	int selected;
};

struct wpa_entry wpa_entries[NR_WPA_ENTRIES];
int wpa_entries_len = 0;

static void put_wpa_entry(int i) {
	char buffer2[128];
	sprintf(buffer2, "%i. %s", i, wpa_entries[i].ssid);
	put_string(10, wpa_entries[i].y, buffer2, wpa_entries[i].selected);
}

void scan_wpa_entries() {
    char buffer[128];

    FILE* pipe = popen("bash wpa_list.sh", "r");

    if (!pipe)
		perror("popen");
    
	int i = 0;
	memset(&wpa_entries, 0, sizeof(wpa_entries));

	wpa_entries_len = 0;

	while (fgets(buffer, sizeof buffer, pipe) != NULL && i < NR_WPA_ENTRIES) {

		// remove last character
		int len=strlen(buffer);
		buffer[len-1]=0;
		
		int y = yres / 4 + 10 * i;

		// int pos;

		// for (pos = 0; pos < strlen(buffer) && buffer[pos] != ' '; ++pos)
		// 	;

		printf("ssid: %s\n", buffer);

		wpa_entries[i].y = y;
		strcpy(wpa_entries[i].ssid, buffer);
		wpa_entries[i].selected = 1;

		put_wpa_entry(i);
		i++;
	}

	wpa_entries_len = i;
	pclose(pipe);
}

#define NR_RESULTS 32

struct scan_result {

	char name[128];
	char ssid[128];
	int dbm;
	int y;
	int selected;

};

int results_len = 0;
struct scan_result results[NR_RESULTS];

void scan_results() {
    char buffer[128];

	exec("bash scan.sh");
	sleep(3);

    FILE* pipe = popen("bash scan_results.sh", "r");

    if (!pipe)
		perror("popen");
    
	int i = 0;
	memset(&results, 0, sizeof(results));

	results_len = 0;

	while (fgets(buffer, sizeof buffer, pipe) != NULL && i < NR_RESULTS) {

		// printf(buffer);

		// remove last character
		int len=strlen(buffer);
		buffer[len-1]=0;
		
		int x = 10; // xres / 2;
		int y = yres / 4 + 10 * i;

		// int pos;

		// for (pos = 0; pos < strlen(buffer) && buffer[pos] != ' '; ++pos)
		// 	;

		char line[128];  // where we will put a copy of the input
		char *dbm; // the "result"

		strcpy(line, buffer);

		dbm = strtok(line," ");
		printf("dbm: %s\n", dbm);

		int dbm_len = strlen(dbm);

		printf("dbm len: %i\n", dbm_len);

		char *name = buffer;
		name += dbm_len + 1; // skip the extra leading space

		printf("name: %s\n", name);

		results[i].y = y;
		results[i].dbm = atoi(dbm);
		strcpy(results[i].name, name);
		results[i].selected = 0;

		put_string(x, y, buffer, 1);

		i = i + 1;
	}

	results_len = i;
	pclose(pipe);
}

struct current_wifi {

	char name[128];
	int index;

} curr_wifi;

int bottom_x = 10;
int bottom_y;

void get_current_wifi() {
    static char buffer[128];
	static char buffer2[128];

    FILE* pipe = popen("bash current.sh", "r");

    if (!pipe)
		perror("popen");
    
	if (fgets(buffer, sizeof buffer, pipe) != NULL) {

		printf(buffer);

		// remove last character
		int len=strlen(buffer);
		buffer[len-1]=0;

		sprintf(buffer2, "Current wifi: %s", buffer);

		// TODO: parse properly
		strcpy(curr_wifi.name, buffer);
		curr_wifi.index = 0;

	}

	pclose(pipe);
}

#define RESULT_HEIGHT 10

int exec_ret(const char* cmd) {
	int ret = system(cmd);
	return ret;
}

static void put_wifi(int i) {
	char buffer [128];
	sprintf(buffer, "%i %s", results[i].dbm, results[i].name);

	int color_idx = results[i].selected;
	put_string(10, results[i].y, buffer, color_idx);
	// printf("idx %i, selected: %i \n", i, results[i].selected);
}

static void refresh_screen(void)
{
	unsigned int i;

	fillrect(0, 0, xres - 1, yres - 1, 0);
	// put_string_center(xres / 2, yres / 4, "Touchscreen example program", 1);
	// put_string_center(xres / 2, yres / 4 + 20, "Touch screen to move crosshair", 2);

	if (program_mode != 2) {
		for (i = 0; i < NR_BUTTONS; i++)
			button_draw(&buttons[i]);

		// scan mode
		if (program_mode == 1) {
			for (i = 0; i < results_len; i++) {
				put_wifi(i);
			}
		}
		// manage mode
		else if (program_mode == 0) {
			
		}
	}
	// keyboard mode
	else if (program_mode == 2) {

		printf("Drawing keyboard buttons...");

		for (i = 0; i < NR_KEYBOARD_BUTTONS; i++) {
			printf("%i (x = %i, y = %i) ", i, keyboardButtons[i].x, keyboardButtons[i].y);
			button_draw(&keyboardButtons[i]);
		}

		printf("done! \n");

		drawInputBox();

	}
}

void put_string_for(int x, int y, char *str, int color, int delay) {
	put_string(10, yres - 10, str, 1);
	sleep(5);
	// write it again with color black as to remove it
	put_string(10, yres - 10, str, 0);
}

void *wifi_updater(void *vargp)
{
	char now[128];
	memset(now, 0, sizeof(now));

	char ina219[128];
	memset(ina219, 0, sizeof(ina219));
	char battery[128];
	memset(battery, 0, sizeof(battery));

	int color = 1;

    while (1) {
		char str[128];
		memset(str, 0, sizeof(str));

		exec_str(str, "bash ./current_wifi.sh");
		printf("currently connected wifi: %s\n", str);

		time_t t = time(NULL);
		struct tm tm = *localtime(&t);

		exec_str(ina219, "python3 ./INA219.py");
		int percent = atoi(ina219 + 1);

		printf("ina219: %s, %i\n", ina219, percent);

		// skip the first character (+ or -)

		// put_string(xres - 160, yres - 10, now, 0);
		// sprintf(now, "%d-%02d-%02d %02d:%02d:%02d", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		// put_string(xres - 160, yres - 10, now, 1);

		put_string(xres - 120, yres - 10, now, 0);
		put_string(xres - 40, yres - 10, battery, 0);

		sprintf(now, "%02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);

		put_string(xres - 120, yres - 10, now, 1);

		sprintf(battery, "%s%%", ina219 + 1);

		if (ina219[0] == '+')
			// display the percentage in green when charging
			put_string(xres - 40, yres - 10, battery, 6);
		else {
			if (percent < 15)
				// display the percentage in RED if below 15
				put_string(xres - 40, yres - 10, battery, 7);
			else
				put_string(xres - 40, yres - 10, battery, 1);
		}

		if (strlen(str) > 0) {
			put_string(10, yres - 10, str, color);

			// print in green when online
			color = exec_ret("ping -c1 www.google.com > /dev/null 2>&1") == 0 ? 6 : 1;
			sleep(2);

			// write the last written string in black, as to remove it
			put_string(10, yres - 10, str, 0);
			sleep(2);
		}
		else {
			char *no_wifi = "No WiFi connected";
			put_string_for(10, yres - 10, no_wifi, 1, 5);
		}
	}
}

int keyboardMode() {



}

int main(int argc, char **argv)
{
	struct tsdev *ts;
	int x, y;
	unsigned int i, j;
	
	int quit_pressed = 0;
	unsigned int mode = 0;

	bottom_y = yres - 10;

	signal(SIGSEGV, sig);
	signal(SIGINT, sig);
	signal(SIGTERM, sig);

	ts = ts_setup(NULL, 0);
	if (!ts) {
		perror("ts_open");
		exit(1);
	}

	if (open_framebuffer()) {
		close_framebuffer();
		ts_close(ts);
		exit(1);
	}

	x = xres / 2;
	y = yres / 2;

	for (i = 0; i < NR_COLORS; i++)
		setcolor(i, palette[i]);

	/* Initialize buttons */
	memset(&buttons, 0, sizeof(buttons));
	int BASE = NR_BUTTONS;

	for (i = 0; i < NR_BUTTONS; i++) {
		buttons[i].w = xres / BASE - 1;
		buttons[i].h = 40;
		buttons[i].x = (i * xres) / BASE;
		buttons[i].y = 10;
		buttons[i].visible = 0;
	}

	buttons[0].text = "Scan"; // "Manage" or "Scan"
	buttons[1].text = "Connect";
	buttons[2].text = "Remove";
	buttons[3].text = "Psk";
	buttons[4].text = "Halt";
	buttons[5].text = "Reboot";

	buttons[0].visible = 1;
	buttons[4].visible = 1;
	buttons[5].visible = 1;

	memset(&keyboardButtons, 0, sizeof(keyboardButtons));

	for (i = 0; i < 2; i++) {
		keyboardButtons[i].w = xres / 3;
		keyboardButtons[i].h = 40;
		keyboardButtons[i].x = (2 * i * xres) / 3;
		keyboardButtons[i].y = 10;
		keyboardButtons[i].visible = 1;
	}

	keyboardButtons[0].text = "CANCEL";
	keyboardButtons[1].text = "SAVE";

	char *lowerchs[] = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "a", "s", "d", "f", "g", "h", "j", "k", "l", "z", "x", "c", "v", "b", "n", "m"};
	char *upperchs[] = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P", "A", "S", "D", "F", "G", "H", "J", "K", "L", "Z", "X", "C", "V", "B", "N", "M"};

	for (i = 2, j = 0; i < 28; i++, j++) {
		keyboardButtons[i].text = lowerchs[j];
	}

	int MAX_BUTTONS_PER_LINE = 10;
	for (i = 2, j = 0; i < 12; i++, j++) {
		keyboardButtons[i].w = xres / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].h = 20;
		keyboardButtons[i].x = (j * xres) / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].y = yres / 2;
		keyboardButtons[i].visible = 1;
	}

	for (i = 12, j = 0; i < 21; i++, j++) {
		keyboardButtons[i].w = xres / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].h = 20;
		keyboardButtons[i].x = 10 + (j * xres) / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].y = yres / 2 + 20;
		keyboardButtons[i].visible = 1;
	}

	for (i = 21, j = 0; i < 30; i++, j++) {
		keyboardButtons[i].w = xres / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].h = 20;
		keyboardButtons[i].x = 20 + (j * xres) / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].y = yres / 2 + 40;
		keyboardButtons[i].visible = 1;
	}

	keyboardButtons[28].text = ",";
	keyboardButtons[29].text = ".";

	for (i = 30, j = 0; i < 40; i++, j++) {
		keyboardButtons[i].w = xres / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].h = 20;
		keyboardButtons[i].x = (j * xres) / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].y = yres / 2 + 60;
		keyboardButtons[i].visible = 1;
	}

	keyboardButtons[30].text = "1";
	keyboardButtons[31].text = "2";
	keyboardButtons[32].text = "3";
	keyboardButtons[33].text = "4";
	keyboardButtons[34].text = "5";
	keyboardButtons[35].text = "6";
	keyboardButtons[36].text = "7";
	keyboardButtons[37].text = "8";
	keyboardButtons[38].text = "9";
	keyboardButtons[39].text = "0";

	for (i = 40, j = 0; i < 50; i++, j++) {
		keyboardButtons[i].w = xres / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].h = 20;
		keyboardButtons[i].x = 5 + (j * xres) / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].y = yres / 2 + 80;
		keyboardButtons[i].visible = 1;
	}

	char chpound[] = {163, 0}; // £

	keyboardButtons[40].text = "@";
	keyboardButtons[41].text = "#";
	keyboardButtons[42].text = chpound; 
	keyboardButtons[43].text = "_";
	keyboardButtons[44].text = "&";
	keyboardButtons[45].text = "-";
	keyboardButtons[46].text = "+";
	keyboardButtons[47].text = "(";
	keyboardButtons[48].text = ")";
	keyboardButtons[49].text = "/";

	for (i = 50, j = 0; i < 60; i++, j++) {
		keyboardButtons[i].w = xres / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].h = 20;
		keyboardButtons[i].x = 0 + (j * xres) / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].y = yres / 2 + 100;
		keyboardButtons[i].visible = 1;
	}

	keyboardButtons[50].text = "*";
	keyboardButtons[51].text = "\"";
	keyboardButtons[52].text = "'";
	keyboardButtons[53].text = ":";
	keyboardButtons[54].text = ";";
	keyboardButtons[55].text = "!";
	keyboardButtons[56].text = "?";
	keyboardButtons[57].text = "~";
	keyboardButtons[58].text = "^";
	keyboardButtons[59].text = "=";

	for (i = 60, j = 0; i < 70; i++, j++) {
		keyboardButtons[i].w = xres / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].h = 20;
		keyboardButtons[i].x = 5 + (j * xres) / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].y = yres / 2 + 120;
		keyboardButtons[i].visible = 1;
	}

	keyboardButtons[60].text = "{";
	keyboardButtons[61].text = "}";
	keyboardButtons[62].text = "\\";
	keyboardButtons[63].text = "%";
	keyboardButtons[64].text = "[";
	keyboardButtons[65].text = "]";
	keyboardButtons[66].text = " ";
	keyboardButtons[67].text = "<";
	keyboardButtons[68].text = ">";
	keyboardButtons[69].text = "`";

	for (i = 70, j = 0; i < 73; i++, j++) {
		keyboardButtons[i].w = 2 * xres / MAX_BUTTONS_PER_LINE;
		keyboardButtons[i].h = 20;
		keyboardButtons[i].y = yres / 2 - 20;
		keyboardButtons[i].visible = 1;
	}

	keyboardButtons[70].x = 10;
	#define SHIFT_TEXT "SHIFT"
	keyboardButtons[70].text = SHIFT_TEXT;

	keyboardButtons[71].x = xres - 2 * xres / MAX_BUTTONS_PER_LINE;
	keyboardButtons[71].text = "<-";

	memset(&inputText, 0, sizeof(inputText));

	program_mode = 0;
	refresh_screen();
	scan_wpa_entries();

	printf("starting main loop \n");

	// create the current wifi updater thread
	pthread_t thread_id;
    pthread_create(&thread_id, NULL, wifi_updater, NULL);

	int currently_selected_ssid = -1;

	while (1) {
		struct ts_sample samp;
		int ret;

		/* Show the cross */
		if ((mode & 15) != 1)
			put_cross(x, y, 2 | XORMODE);

		ret = ts_read(ts, &samp, 1);

		/* Hide it */
		if ((mode & 15) != 1)
			put_cross(x, y, 2 | XORMODE);

		if (ret < 0) {
			perror("ts_read");
			close_framebuffer();
			ts_close(ts);
			exit(1);
		}

		if (ret != 1)
			continue;

		if (program_mode == 2) {

			for (unsigned int i = 0; i < NR_KEYBOARD_BUTTONS; ++i) {
				if (button_handle(&keyboardButtons[i], samp.x, samp.y, samp.pressure)) {

					char * text = keyboardButtons[i].text;

					// cancel
					if (i == 0) {
						// go back to management mode
						program_mode = 0;
						refresh_screen();
						scan_wpa_entries();
					}
					// save
					else if (i == 1) {

						char psk[128];
						strcpy(psk, inputText);

						char *ssid = wpa_entries[currently_selected_ssid].ssid;
						
						char cmd[128];
						sprintf(cmd, "bash wifi_psk.sh \"%s\" \"%s\"", ssid, psk);

						printf("Change password for wifi %i: %s, new psk: %s\n", currently_selected_ssid, ssid, psk);
						int ret = exec_ret(cmd);
						
						if (ret != 0) {

							fillrect(xres / 2 - 40, yres / 2 - 20, xres / 2 + 40, yres / 2 + 20, 4);
							rect(xres / 2 - 40, yres / 2 - 20, xres / 2 + 40, yres / 2 + 20, 3);
							put_string_center(xres / 2, yres / 2, "ERROR!", 7);
							sleep(1);
							refresh_screen();

						}
						else {
							fillrect(xres / 2 - 40, yres / 2 - 20, xres / 2 + 40, yres / 2 + 20, 4);
							rect(xres / 2 - 40, yres / 2 - 20, xres / 2 + 40, yres / 2 + 20, 3);
							put_string_center(xres / 2, yres / 2, "OK", 6);
							sleep(1);
							program_mode = 0;
							refresh_screen();
							scan_wpa_entries();
						}

					}
					else if (strcmp(text, SHIFT_TEXT) == 0) {

						shift_on = 1 - shift_on;

						for (unsigned int k = 2, h = 0; k < 28; k++, h++) {
							keyboardButtons[k].text = shift_on ? upperchs[h] : lowerchs[h];
						}

						refresh_screen();

					}
					else {

						unsigned int len = strlen(inputText);

						printf("detected keyboard button %i, text: %s\n", i, text);

						if (strcmp(text, "<-") == 0) {
							if (len > 0) {
								printf("remove last character\n");

								// chop off last character
								inputText[len - 1] = 0;
							}
						}
						else if (len < INPUT_TEXT_MAX_LEN - 1) {
							char letter = keyboardButtons[i].text[0];

							printf("add character: %c\n", letter);
							inputText[len] = letter;
							inputText[len + 1] = 0;
						}

						printf("new input text: %s, of len: %i\n", inputText, strlen(inputText));
						drawInputBox();

					}

				}
			}
		}
		else {
			for (unsigned int i = 0; i < NR_BUTTONS; ++i) {
				if (button_handle(&buttons[i], samp.x, samp.y, samp.pressure)) {

					buttons[1].visible = 0;
					buttons[2].visible = 0;
					buttons[3].visible = 0;
					button_draw(&buttons[1]);
					button_draw(&buttons[2]);
					button_draw(&buttons[3]);

					printf("button id: %i\n", i);

					switch (i) {
					case 0:

						if (program_mode == 0) {
							program_mode = 1;
							buttons[0].text = "Manage";

							// make sure no result is displayed at first
							results_len = 0;
							//  scan
							refresh_screen();
							scan_results();
						}
						else {
							program_mode = 0;
							buttons[0].text = "Scan";
							refresh_screen();
							scan_wpa_entries();
						}

						break;
					case 1:

						// manage mode
						if (program_mode == 0 && currently_selected_ssid != -1) {

							// Connect to selected wifi network			
							
							char *ssid = wpa_entries[currently_selected_ssid].ssid;
							printf("Connect to wifi %i: %s\n", currently_selected_ssid, ssid);

							char cmd[128];
							sprintf(cmd, "bash wifi_add.sh \"%s\"", ssid);
							int ret = exec_ret(cmd);
							
							currently_selected_ssid = -1;

						}
						// scan mode
						else if (currently_selected_ssid != -1) {

							// Connect to selected wifi network			
							
							char *ssid = results[currently_selected_ssid].name;
							printf("Connect to wifi %i: %s\n", currently_selected_ssid, ssid);

							char cmd[128];
							sprintf(cmd, "bash wifi_add.sh \"%s\"", ssid);
							exec_ret(cmd);
							
							// now we select this wifi as the new current one
							// char cmd[128];
							// sprintf(cmd, "bash wifi_exists.sh %s", ssid);

							// int ret = exec_ret(cmd);
							// printf("ret: %i\n", ret);

							// // wifi does not exists in wpa_supplicant
							// if (ret == 0) {
							// 	char cmd[128];
							// 	char *psk = "default_psk";

							// 	sprintf(cmd, "bash wifi_add.sh %s %s", ssid, psk);
							// 	int ret = exec_ret(cmd);
							// }
							// // wifi exists in wpa_supplicant
							// else {
							// 	char cmd[128];
							// 	sprintf(cmd, "bash wifi_add.sh %s", ssid);
							// 	int ret = exec_ret(cmd);
							// }


							get_current_wifi();

							currently_selected_ssid = -1;

							printf("unselecting ssid\n");

						}

						break;
					case 2:
						// Remove

						if (program_mode == 0 && currently_selected_ssid != -1) {

							// remove selected wifi network from wpa_supplicant
							char *ssid = wpa_entries[currently_selected_ssid].ssid;
							printf("Remove wifi %i: %s\n", currently_selected_ssid, ssid);

							char cmd[128];
							sprintf(cmd, "bash wifi_remove.sh %i", currently_selected_ssid);
							exec_ret(cmd);
							
							currently_selected_ssid = -1;

							refresh_screen();
							scan_wpa_entries();

						}
						break;

					case 3:
						// Psk

						printf("PSK \n");

						if (program_mode == 0 && currently_selected_ssid != -1) {

							// change password for selected network
							char *ssid = wpa_entries[currently_selected_ssid].ssid;

							char cmd[128];
							sprintf(cmd, "bash get_psk.sh %i", currently_selected_ssid);

							char psk[128];
							exec_str(psk, cmd);
							psk[strlen(psk)-1]=0;
							
							printf("Change password for wifi %i: %s, old psk: %s\n", currently_selected_ssid, ssid, psk);

							// currently_selected_ssid = -1;

							// scan_wpa_entries();

							// go to keyboard mode to change the password...
							program_mode = 2;
							strcpy(inputText, psk);
							refresh_screen();

						}
						break;
					case 4:
						//halt
						exec("sudo halt");
						
						break;
					
					case 5:
						//reboot
						exec("sudo reboot");

					}
				}
			}
		}
		// printf("Current mode: %i\n", program_mode);

		// scan mode
		if (program_mode == 1) {
			for (i = 0; i < results_len; i++) {

				y = results[i].y;
				int inside = samp.y >= y && samp.y < y + RESULT_HEIGHT;

				if (inside) {
					if (samp.pressure > 0) {
						// click started
						// put_string_center(xres / 2, yres / 4 + RESULT_HEIGHT * i, "XXX", 1);
						results[i].selected = 2;
						put_wifi(i);

					}
					else if (samp.pressure == 0 && results[i].selected == 2) {
						// click ended
						results[i].selected = 6;
						put_wifi(i);
						currently_selected_ssid = i;
						buttons[1].visible = 1;
						button_draw(&buttons[1]);

						printf("Selected wifi %i\n", currently_selected_ssid);
					}
					else {
						results[i].selected = 1;
						put_wifi(i);
					}

					// printf("Wifi selected %i: %s, dbm: %i \n", results[i].selected, results[i].name, results[i].dbm);

				}
				else {
					results[i].selected = 1;
					put_wifi(i);
					// printf("Wifi unselected %i: %s, dbm: %i \n", results[i].selected, results[i].name, results[i].dbm);
				}
			}
		}
		// management mode
		else if (program_mode == 0) {

			for (i = 0; i < wpa_entries_len; i++) {

				y = wpa_entries[i].y;
				int inside = samp.y >= y && samp.y < y + RESULT_HEIGHT;

				if (inside) {
					if (samp.pressure > 0)
						// click started
						wpa_entries[i].selected = 2;
					else if (samp.pressure == 0 && wpa_entries[i].selected == 2) {
						// click ended
						wpa_entries[i].selected = 6;

						currently_selected_ssid = i;
						buttons[1].visible = 1;
						buttons[2].visible = 1;
						buttons[3].visible = 1;
						button_draw(&buttons[1]);
						button_draw(&buttons[2]);
						button_draw(&buttons[3]);
					}
					else
						wpa_entries[i].selected = 1;
				}
				else
					wpa_entries[i].selected = 1;

				put_wpa_entry(i);
			}

		}
		else if (program_mode == 2) {

		}
		else {
			printf("known program mode: %i\n", program_mode);
		}

		// printf("%ld.%06ld: %6d %6d %6d\n", samp.tv.tv_sec, samp.tv.tv_usec, samp.x, samp.y, samp.pressure);

		if (samp.pressure > 0) {
			if (mode == 0x80000001)
				line(x, y, samp.x, samp.y, 2);
			x = samp.x;
			y = samp.y;
			mode |= 0x80000000;
		} else
			mode &= ~0x80000000;
	}

	fillrect(0, 0, xres - 1, yres - 1, 0);
	close_framebuffer();
	ts_close(ts);

	return 0;
}
