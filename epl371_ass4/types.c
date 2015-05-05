#include "types.h"

char *get_filename_ext(char *filename) {
	char *dot = strrchr(filename, '.');
	if (!dot || dot == filename)
		return "";
	return dot + 1;
}

char *get_content_type(char *filename) {
	char *ext = get_filename_ext(filename);

	if (strcmp(ext, "txt") == 0 || strcmp(ext, "sed") == 0
			|| strcmp(ext, "awk") == 0 || strcmp(ext, "c") == 0
			|| strcmp(ext, "h") == 0) {
		return ("text/plain");
	}

	if (strcmp(ext, "html") == 0 || strcmp(ext, "htm") == 0) {
		return ("text/html");
	}

	if (strcmp(ext, "jpeg") == 0 || strcmp(ext, "jpg") == 0) {
		return ("image/jpeg");
	}

	if (strcmp(ext, "png") == 0) {
		return ("image/png");
	}

	if (strcmp(ext, "css") == 0) {
		return ("text/css");
	}

	if (strcmp(ext, "js") == 0) {
		return ("text/javascript");
	}

	if (strcmp(ext, "gif") == 0) {
		return ("image/gif");
	}

	if (strcmp(ext, "pdf") == 0) {
		return ("application/pdf");
	}

	return "application/octet-stream";
}
