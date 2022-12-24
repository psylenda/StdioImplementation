#include <fcntl.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <strings.h>
#include <string.h>
#include <stdarg.h>   
#include <stdlib.h>    
using namespace std;

char decimal[100];
//Helper Function
int recursive_itoa(int arg) 
{
	int div = arg / 10;
	int mod = arg % 10;
	int index = 0;
	if (div > 0)
	{
		index = recursive_itoa(div);
	}
	decimal[index] = mod + '0';
	return ++index;
}
//Helper Function
char *itoa(const int arg) 
{
	bzero(decimal, 100);
	int order = recursive_itoa(arg);
	char *new_decimal = new char[order + 1];
	bcopy(decimal, new_decimal, order + 1);
	return new_decimal;
}

int printf(const void *format, ...) 
{
	va_list list;
	va_start(list, format);

	char *msg = (char *)format;
	char buf[1024];
	int nWritten = 0;

	int i = 0, j = 0, k = 0;
	while (msg[i] != '\0') 
	{
		if (msg[i] == '%' && msg[i + 1] == 'd')
		{
			buf[j] = '\0';
			nWritten += write(1, buf, j);
			j = 0;
			i += 2;

			int int_val = va_arg(list, int);
			char *dec = itoa(abs(int_val));
			if (int_val < 0)
			{
				nWritten += write(1, "-", 1);
			}	
			nWritten += write(1, dec, strlen(dec));
			delete dec;
		}
		else
		{
			buf[j++] = msg[i++];
		}	
	}
	if (j > 0)
	{
		nWritten += write(1, buf, j);
	}	
	va_end( list );
	return nWritten;
}

int setvbuf(FILE *stream, char *buf, int mode, size_t size) 
{
	if (mode != _IONBF && mode != _IOLBF && mode != _IOFBF)
	{
		return -1;
	}	
	stream->mode = mode;
	stream->pos = 0;
	if (stream->buffer != (char *)0 && stream->bufown == true)
	{
		delete stream->buffer;
	}
	
	switch ( mode ) 
	{
		case _IONBF:
			stream->buffer = (char *)0;
			stream->size = 0;
			stream->bufown = false;
			break;
		case _IOLBF: //not implemented
    
		case _IOFBF:
			if (buf != (char *)0) 
			{
				stream->buffer = buf;
				stream->size   = size;
				stream->bufown = false;
			}
			else 
			{
				stream->buffer = new char[BUFSIZ];
				stream->size = BUFSIZ;
				stream->bufown = true;
			}
			break;
	}
	return 0;
}

void setbuf(FILE *stream, char *buf) 
{
	setvbuf(stream, buf, ( buf != (char *)0 ) ? _IOFBF : _IONBF , BUFSIZ);
}

FILE *fopen(const char *path, const char *mode) 
{
	FILE *stream = new FILE();
	setvbuf(stream, (char *)0, _IOFBF, BUFSIZ);
	
	// fopen( ) mode
	// r or rb = O_RDONLY
	// w or wb = O_WRONLY | O_CREAT | O_TRUNC
	// a or ab = O_WRONLY | O_CREAT | O_APPEND
	// r+ or rb+ or r+b = O_RDWR
	// w+ or wb+ or w+b = O_RDWR | O_CREAT | O_TRUNC
	// a+ or ab+ or a+b = O_RDWR | O_CREAT | O_APPEND

  switch(mode[0]) 
  {
  case 'r':
	  if (mode[1] == '\0')            // r
	  {
		  stream->flag = O_RDONLY;
	  }  
	  else if ( mode[1] == 'b' ) 
	  {    
		  if (mode[2] == '\0')          // rb
		  {
			  stream->flag = O_RDONLY;
		  } 
		  else if (mode[2] == '+')      // rb+
		  {
			  stream->flag = O_RDWR;
		  }			  
	  }
	  else if (mode[1] == '+')        // r+  r+b
	  {
		  stream->flag = O_RDWR;
	  }  
	  break;
  case 'w':
	  if (mode[1] == '\0')            // w
	  {
		  stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
	  }	  
	  else if (mode[1] == 'b') 
	  {
		  if (mode[2] == '\0')          // wb
		  {
			  stream->flag = O_WRONLY | O_CREAT | O_TRUNC;
		  }	  
		  else if (mode[2] == '+')      // wb+
		  {
			  stream->flag = O_RDWR | O_CREAT | O_TRUNC;
		  }	  
	  }
	  else if (mode[1] == '+')        // w+  w+b
	  {
		  stream->flag = O_RDWR | O_CREAT | O_TRUNC;
	  }
	  break;
  case 'a':
	  if (mode[1] == '\0')            // a
	  {
		  stream->flag = O_WRONLY | O_CREAT | O_APPEND;
	  } 
	  else if (mode[1] == 'b')
	  {
		  if (mode[2] == '\0')          // ab
		  {
			  stream->flag = O_WRONLY | O_CREAT | O_APPEND;
		  }  
		  else if (mode[2] == '+')      // ab+
		  {
			  stream->flag = O_RDWR | O_CREAT | O_APPEND;
		  }	  
	  }
	  else if (mode[1] == '+')        // a+  a+b
	  {
		  stream->flag = O_RDWR | O_CREAT | O_APPEND;
	  } 
	  break;
  }
  
  mode_t open_mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;

  if ((stream->fd = open(path, stream->flag, open_mode)) == -1) 
  {
	  delete stream;
	  printf("fopen failed\n");
	  stream = NULL;
  }
  
  return stream;
}

int fpurge(FILE *stream)
{
	// complete it
  memset(stream->buffer, '\0', stream->size);
  stream->actual_size = 0;
  stream->pos = 0;
	return 0;
}

int fflush(FILE *stream) 
{
	// complete it
  int bytesWritten = write(stream->fd, stream->buffer, stream->actual_size);
  memset(stream->buffer, '\0', stream->size);
  stream->actual_size =0;
  stream->pos = 0;
	return 0;
}



/*returns the total number of successfully read elements
takes in pointer to array with size nmemb * size. If return value
 is lower than that, then either there was an error or eof.
 Stream is the object that is being read.
 */
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream) 
{

  int count = (int)nmemb * (int)size;
  //printf("count is %d\n", count);
  
  ((char*)ptr)[count];

  //check what the last operation was, if it was write purge buffer
  if (stream->lastop == 'w') {
    //purge buffer
    fpurge(stream);
  }
  
  //check the mode. If it is 
  if (stream->mode == _IONBF) {
    int bytesRead = read(stream->fd, ptr, count);
    return bytesRead;
  }
  int progress = 0;
  
  //run a while loop for each character
  while ((progress-1) != (count-1) && stream->eof != true){
    if (stream->pos == 0) {
      stream->actual_size = read(stream->fd, stream->buffer, stream->size);
    }
    if (stream->pos == stream->actual_size) {
      fpurge(stream);
      stream->actual_size = read(stream->fd, stream->buffer, stream-> size);
    }
    if (stream->actual_size == 0) {
      break;
    }
    ((char*)ptr)[progress] = stream->buffer[stream->pos];
    progress++;
    stream->pos++;
  }
  //return # of bytes successfully read
  //printf("progress is %d\n", progress);
  //printf("progress is %d\n", progress);
	stream->lastop = 'r';
	return progress;
}


/* fwrite
This function takes in a buffer pointer, a size of each member, number of members,
and a file stream. It multiplies size and nmemb to create count. This is the number
of bytes that must be written. It takes this bytes from the buffer as a parameter,
and moves them into the stream's buffer. When the stream's position hits the limit
it flushes all of that into the file itself. Then it continues filling the buffer
from the parameter buffer. The last thing it does
*/
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream) 
{
	// complete it
  
  
  int count = (int)nmemb * (int)size; //number of bytes being written

  if (stream->mode == _IONBF) {
    int bytesWritten = write(stream->fd, ptr, count);
    return bytesWritten;
  }
  if (stream->lastop == 'w') {
    //purge buffer
    fpurge(stream);
  }
  
  ((char*)ptr)[count+1];
  if (stream->lastop == 'r') {
    fpurge(stream);
  }
  int progress = 0;

  while(progress != count) {
    if (stream->pos == stream->size) {
      fflush(stream);
    }
    stream->buffer[stream->pos] = ((char*)ptr)[progress];
    progress++;
    stream->pos++;
    stream->actual_size++;
  }
  fflush(stream);
  stream->lastop = 'w';
	return 0;
}



/* fgetc
I had to write this rather redundantly unlike fgets because I was having trouble
making it work. But basically, it does the same thing. It checks to see if the
position in the buffer has reached the limit, and if so, it clears out the buffer
and refills it with a read call. If nothing is returned, then it's hit eof. So
it marks that as true and returns EOF. Otherwise, it copies the index of the 
buffer's current position to a char, increments the position, and returns the
char.
*/
int fgetc(FILE *stream) 
{
  int curPos = lseek(stream->fd, 0, SEEK_CUR);
  if (stream->pos == stream->actual_size) {
    fpurge(stream);
    stream->actual_size = read(stream->fd, stream->buffer, stream->size);
    if (stream->actual_size == 0) {
      stream->eof == true;
      return EOF;
    }
  }
  char rVal = stream->buffer[stream->pos];
  stream->pos++;
  return rVal;
}

/* fputc
This function takes an int c and file stream, and typecasts that int into a 
character before writing it to the filestream, using fwrite. 
*/
int fputc(int c, FILE *stream) 
{
  char str[1] = {(char)c};
  int bytesWritten = fwrite(str, sizeof(char), 1, stream);
	return 0;
}

/* fgets
This function takes a buffer, an int for size, and a file stream. It reads
size number of bytes from the filestream to the buffer. It returns the string
that was taken from the file stream. It also appends the terminating character
for a string so that the driver knows when the string ends.
*/
char *fgets(char *str, int size, FILE *stream) 
{
  int bytesRead = fread(str, 1, size, stream);
  str[bytesRead] = '\0';
  if (bytesRead == 0) {
    stream->eof = true;
    return NULL;
  }
	return str;
}

/* fputs
simple function to write a string to a file. It takes in the character array
to be written to the file, and the file stream itself. It passes both of those
into an fwrite call. The number of bytes successfully written is returned as
an integer value.
*/
int fputs(const char *str, FILE *stream) 
{  
  int bytesWritten = fwrite(str, sizeof(char), strlen(str), stream);
	return bytesWritten;
}

/* feof
This function checks to see if the current position in the file
is at its end or not. It does this through three lseek calls. The first
saves the current position in the file as an integer, prevPos. The second
jumps to the end of the file and saves that position to eofPos. The third
jumps back to the location saved at prevPos so that nothing is altered in
the actual file reading.

if the current file position and the eof position are the same, then you've
arrived at the end of the file, and this function will return true. If not,
it will return false.
*/
int feof(FILE *stream) 
{
  int prevPos = lseek(stream->fd, 0, SEEK_CUR);
  int eofPos = lseek(stream->fd, 0, SEEK_END);
  int curPos = lseek(stream->fd, prevPos, SEEK_SET);

  if(curPos == eofPos) {
    return true;
  }
	else {
    return false;
  }
}

/* fseek
This is a simple function which takes a file stream, an offset, and a whence.
The whence tells you where to start from, and you can start with three options:
SEEK_CUR - where you're at already, SEEK_END, the end of the file, and SEEK_SET, 
the file's beginning. The position is then incremented by the amount specified
by the value of offset. 

I also added a check to make sure if the position landed at the end of the file. 
if so, the stream->eof value gets updated.
*/
int fseek(FILE *stream, long offset, int whence) 
{
  lseek(stream->fd, offset, whence);
  stream->eof = false;
  //using lseek calls to make sure that the point fseek landed on is not the end of the file
  int curPos = lseek(stream->fd, 0, SEEK_CUR);
  int eofPos = lseek(stream->fd, 0, SEEK_END);
  int returnPos = lseek(stream->fd, curPos, SEEK_SET);
  if (eofPos == curPos) {
    stream->eof = true;
  }
	return 0;
}

/*fclose
This takes an open file stream and closes it. It also resets
all of its members to their default values, and returns 0.
*/
int fclose(FILE *stream) 
{
  fpurge(stream);
  stream->eof = false;
  stream->mode = _IONBF;
  stream->flag = 0;
  stream->pos = 0;
  stream->lastop = 0;
  stream->size = 0;
  stream->actual_size = 0;
  stream->buffer = (char *) 0;
	close(stream->fd);
	return 0;
}
