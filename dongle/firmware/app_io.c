/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       main.c                                                    *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    main.c file                                               *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2008-11-07:     created(by SimonQian)                             *
 **************************************************************************/

#include <stdlib.h>
#include <stdarg.h>

#include "app_io.h"
#include "app_err.h"
#include "app_log.h"

#include "scripts.h"
#include "interfaces.h"
#include "tool/buffer/buffer.h"
#include "dal/usart_stream/usart_stream.h"

#include "usb_protocol.h"

VSS_HANDLER(appio_set_dummy);

static const struct vss_cmd_t appio_cmd[] =
{
	VSS_CMD(	"dummy",
				"set dummy mode of appio, format: appio.dummy DUMMY",
				appio_set_dummy,
				NULL),
	VSS_CMD_END
};
struct vss_cmd_list_t appio_cmd_list =
							VSS_CMD_LIST("appio", appio_cmd);

#if APPIO_DUMMY
static bool appio_dummy = true;
#else
static bool appio_dummy = false;
#endif

VSS_HANDLER(appio_set_dummy)
{
	VSS_CHECK_ARGC(2);
	
	appio_dummy = (strtoul(argv[1], NULL, 0) != 0);
	return VSFERR_NONE;
}

static uint8_t shell_buff_tx[64], shell_buff_rx[64];
struct usart_stream_info_t shell_stream =
{
	IFS_DUMMY_PORT,								// usart_index
	{
		{shell_buff_rx, sizeof(shell_buff_rx)}	// fifo
	},											// struct vsf_stream_t stream_rx;
	{
		{shell_buff_tx, sizeof(shell_buff_tx)}	// fifo
	}											// struct vsf_stream_t stream_tx;
};

static char app_io_local_buff[APPIO_BUFFER_SIZE];

static void app_io_out_sync(void)
{
	int free_space;
	
	do
	{
		usb_protocol_poll();
		free_space = vsf_fifo_get_data_length(&shell_stream.stream_rx.fifo);
	} while (free_space);
}

// virtual files
struct appio_file_t
{
	char *filename;
	uint8_t *addr;
	uint64_t size;
	
	// private
	uint64_t pos;
	FILE fn;
} static appio_filelist[] =
{
#if defined(EVSPROG_SCRIPT_FILE) && defined(EVSPROG_SCRIPT_ADDR)
	{EVSPROG_SCRIPT_FILE, (uint8_t *)EVSPROG_SCRIPT_ADDR},
#endif
#if defined(TARGET0_SCRIPT_FILE) && defined(TARGET0_SCRIPT_ADDR)
	{TARGET0_SCRIPT_FILE, (uint8_t *)TARGET0_SCRIPT_ADDR},
#endif
#if defined(TARGET1_SCRIPT_FILE) && defined(TARGET1_SCRIPT_ADDR)
	{TARGET1_SCRIPT_FILE, (uint8_t *)TARGET1_SCRIPT_ADDR},
#endif
};

void APP_IO_INIT(void)
{
	int i;
	uint8_t ch;
	for (i = 0; i < dimof(appio_filelist); i++)
	{
		appio_filelist[i].size = 0;
		for (ch = appio_filelist[i].addr[appio_filelist[i].size];
				(ch != 0x00) && (ch != 0xFF);
				ch = appio_filelist[i].addr[++appio_filelist[i].size]);
	}
}

void APP_IO_FINI(void)
{
	
}

static struct appio_file_t* appio_file_byname(char *filename)
{
	int i;
	for (i = 0; i < dimof(appio_filelist); i++)
	{
		if (!strcmp(filename, appio_filelist[i].filename))
		{
			return &appio_filelist[i];
		}
	}
	return NULL;
}
static struct appio_file_t* appio_file_byfn(FILE *fn)
{
	int i;
	for (i = 0; i < dimof(appio_filelist); i++)
	{
		if (fn == &appio_filelist[i].fn)
		{
			return &appio_filelist[i];
		}
	}
	return NULL;
}

FILE *FOPEN(const char *filename, const char *mode)
{
	struct appio_file_t *file = appio_file_byname((char *)filename);
	if (file != NULL)
	{
		file->pos = 0;
		return &file->fn;
	}
	return NULL;
}

int FCLOSE(FILE *f)
{
	if ((f != stdin) && (f != stdout) && (f != stderr))
	{
		struct appio_file_t *file = appio_file_byfn(f);
		if (file != NULL)
		{
			file->pos = 0;
			return 0;
		}
	}
	return 0;
}

int FEOF(FILE *f)
{
	if ((stdin == f) || (stdout == f) || (stderr == f))
	{
		return 0;
	}
	else
	{
		struct appio_file_t *file = appio_file_byfn(f);
		if (file != NULL)
		{
			return file->pos >= file->size ? 1 : 0;
		}
		return 1;
	}
}

void REWIND(FILE *f)
{
	if ((f != stdin) && (f != stdout) && (f != stderr))
	{
	}
	else
	{
		struct appio_file_t *file = appio_file_byfn(f);
		if (file != NULL)
		{
			file->pos = 0;
		}
	}
}

int FFLUSH(FILE *f)
{
	if ((stdout == f) || (stderr == f))
	{
		if (!appio_dummy)
		{
			app_io_out_sync();
		}
		return 0;
	}
	else if (stdin == f)
	{
		if (!appio_dummy)
		{
			uint32_t i, size = vsf_fifo_get_data_length(&shell_stream.stream_tx.fifo);
			for (i = 0; i < size; i++)
			{
				vsf_fifo_pop8(&shell_stream.stream_tx.fifo);
			}
		}
		return 0;
	}
	else
	{
		struct appio_file_t *file = appio_file_byfn(f);
		if (file != NULL)
		{
			// TODO: flush appio_file
		}
	}
	return 0;
}

int FGETC(FILE *f)
{
	if ((stdout == f) || (stderr == f))
	{
		return 0;
	}
	else if (stdin == f)
	{
		if (!appio_dummy)
		{
			uint32_t size;
			do
			{
				usb_protocol_poll();
				size = vsf_fifo_get_data_length(&shell_stream.stream_tx.fifo);
			} while (!size);
			return vsf_fifo_pop8(&shell_stream.stream_tx.fifo);
		}
	}
	else
	{
		struct appio_file_t *file = appio_file_byfn(f);
		if (file != NULL)
		{
			return file->pos >= file->size ? EOF : file->addr[file->pos++];
		}
	}
	return 0;
}

int GETCHAR(void)
{
	return FGETC(stdin);
}

char* FGETS(char *buf, int count, FILE *f)
{
	char cur_char, *result = buf;
	int size = 0, cur_size, pos;
	
	if ((NULL == buf) || (NULL == f) || (stdout == f) || (stderr == f))
	{
		return NULL;
	}
	
	if (stdin == f)
	{
		if (!appio_dummy)
		{
			pos = 0;
			cur_char = '\0';
			while ((size < count) && (cur_char != '\r'))
			{
				usb_protocol_poll();
				cur_size = vsf_fifo_get_data_length(&shell_stream.stream_tx.fifo);
				
				while (cur_size && (size < count) && (cur_char != '\r'))
				{
					cur_char = (char)vsf_fifo_pop8(&shell_stream.stream_tx.fifo);
					if ('\r' == cur_char)
					{
						vsf_fifo_push8(&shell_stream.stream_rx.fifo, '\n');
					}
					else if ('\b' == cur_char)
					{
						if (pos)
						{
							vsf_fifo_push8(&shell_stream.stream_rx.fifo, '\b');
							vsf_fifo_push8(&shell_stream.stream_rx.fifo, ' ');
							vsf_fifo_push8(&shell_stream.stream_rx.fifo, '\b');
							pos--;
						}
						cur_size--;
						continue;
					}
					else if (!((cur_char >= ' ') && (cur_char <= '~')))
					{
						cur_size--;
						continue;
					}
					vsf_fifo_push8(&shell_stream.stream_rx.fifo, (uint8_t)cur_char);
					
					buf[pos++] = cur_char;
					size++;
					cur_size--;
				}
			}
			buf[pos] = '\0';
			app_io_out_sync();
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		struct appio_file_t *file = appio_file_byfn(f);
		if (file != NULL)
		{
			if (count < 3)
			{
				return NULL;
			}
			count -= 3;
			
			while ((file->addr[file->pos] != '\0') &&
					(file->addr[file->pos] != 0xFF) &&
					(file->pos < file->size) &&
					((file->addr[file->pos] == '\n') ||
						(file->addr[file->pos] == '\r')))
			{
				file->pos++;
			}
			while (count-- && 
					(file->pos < file->size) &&
					(file->addr[file->pos] != '\0') &&
					(file->addr[file->pos] != '\n') &&
					(file->addr[file->pos] != '\r') &&
					(file->addr[file->pos] != 0xFF))
			{
				*buf++ = file->addr[file->pos++];
			}
			if (result == buf)
			{
				return NULL;
			}
			*buf++ = '\n';
			*buf++ = '\r';
			*buf++ = '\0';
		}
	}
	return result;
}

static void APPIO_OUTBUFF(uint8_t *buff, uint32_t size)
{
	uint32_t free_space, cur_size;
	
	while (size > 0)
	{
		do
		{
			usb_protocol_poll();
			free_space = vsf_fifo_get_avail_length(&shell_stream.stream_rx.fifo);
		} while (!free_space);
		
		if (free_space > size)
		{
			cur_size = size;
		}
		else
		{
			cur_size = free_space;
		}
		
		vsf_fifo_push(&shell_stream.stream_rx.fifo, cur_size, buff);
		
		size -= cur_size;
		buff += cur_size;
	}
	
	app_io_out_sync();
}

int FPRINTF(FILE *f, const char *format, ...)
{
	int number = 0;
	char *pbuff = app_io_local_buff;
	va_list ap;
	
	if ((NULL == f) || (stdin == f) || (appio_file_byfn(f) != NULL))
	{
		return 0;
	}
	
	va_start(ap, format);
	number = vsnprintf(app_io_local_buff, sizeof(app_io_local_buff), format, ap);
	va_end(ap);
	
	if ((stdout == f) || (stderr == f))
	{
		if (!appio_dummy)
		{
			APPIO_OUTBUFF((uint8_t *)pbuff, (uint32_t)number);
		}
	}
	else
	{
	}
	return number;
}

int PRINTF(const char *format, ...)
{
	int number = 0;
	char *pbuff = app_io_local_buff;
	va_list ap;
	
	if (!appio_dummy)
	{
		va_start(ap, format);
		number = vsnprintf(app_io_local_buff, sizeof(app_io_local_buff), format, ap);
		va_end(ap);
	
		APPIO_OUTBUFF((uint8_t *)pbuff, (uint32_t)number);
	}
	return number;
}
