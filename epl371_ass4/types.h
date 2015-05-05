#include "def.h"

#ifndef TYPES_H_
#define TYPES_H_

/**
 * Returns the extension of the filename or null.
 * @param filename Filename to find it's extension.
 * @return Filename's extension or null.
 */
char *get_filename_ext(char *filename);

/**
 * By using the get_filename_ext, this function
 * returns the content_type of the given filename.
 * @param filename Filename to find it's content-type.
 * @return Filename's content-type.
 */
char *get_content_type(char *filename);

#endif /* TYPES_H_ */
