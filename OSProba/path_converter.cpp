
#include "fs.h"
#include <string>

using namespace std;

void parse(char* fname, char& part, char* name, char* ext) {
	part = fname[0];
	bool dot_occured = false;
	int dot_position;
	for (int i = 0; i < 8; i++) {
		if (fname[i + 3] == '.') {
			dot_occured = true;
			dot_position = i + 3;
		}
		if (!dot_occured) name[i] = fname[3 + i];
		else name[i] = 0;
	}
	bool end_reached = false;
	for (int i = 0; i < 3; i++) {
		if (fname[dot_position + i + 1] == '\0') end_reached = true;
		if (!end_reached) ext[i] = fname[dot_position + i + 1];
		else ext[i] = 0;
	}
}

bool compare(const Entry& entry, char* fname) {
	char name[9];
	char ext[4];
	char part;

	parse(fname, part, name, ext);
	name[8] = ext[3] = '\0';
	string entry_name = string(1, fname[0]) + string(":\\") + string(name) + '.' + string(ext);

	if (entry_name == string(fname)) return true;
	else return false;
}