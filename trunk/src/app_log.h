/***************************************************************************
 *   Copyright (C) 2009 - 2010 by Simon Qian <SimonQian@SimonQian.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef __APP_LOG_H_INCLUDED__
#define __APP_LOG_H_INCLUDED__

#define LOG_LINE_END		"\n"

#define ERROR_LEVEL			0
#define WARNING_LEVEL		0
#define INFO_LEVEL			1
#define DEBUG_LEVEL			2

#define LOG_DEFAULT_LEVEL	INFO_LEVEL

extern int verbosity;
extern int verbosity_stack[1];

#define LOG_MUTE()			verbosity = -1
#define LOG_PUSH()			verbosity_stack[0] = verbosity
#define LOG_POP()			verbosity = verbosity_stack[0]

#define _GETTEXT(STR)		STR

#define LOG_BYTE_BUF(buff, len, func, format, n)	\
	do{\
		char line[256], s[4];\
		int __i, __j;\
		for (__i = 0; __i < (int)(len); __i += (n))\
		{\
			snprintf(line, 5, "%04X", __i);\
			for (__j = __i; __j < __i + (n) && __j < (int)(len); __j++)\
			{\
				snprintf(s, 4, " " format, (buff)[__j]);\
				strncat(line, s, sizeof(s));\
			}\
			func("%s", line);\
		}\
	}while(0)

#if 1
#	define LOG_ERROR(format, ...)	\
		do{\
			if (verbosity >= ERROR_LEVEL)\
			{\
				fprintf(stderr, _GETTEXT("Error:  " format LOG_LINE_END), \
						##__VA_ARGS__);\
			}\
		}while(0)
#	define LOG_WARNING(format, ...)	\
		do{\
			if (verbosity >= WARNING_LEVEL)\
			{\
				fprintf(stdout, _GETTEXT("Warning:" format LOG_LINE_END), \
						##__VA_ARGS__);\
				fflush(stdout);\
			}\
		}while(0)
#	define LOG_INFO(format, ...)	\
		do{\
			if (verbosity >= INFO_LEVEL)\
			{\
				fprintf(stdout, _GETTEXT("Info:   " format LOG_LINE_END), \
						##__VA_ARGS__);\
				fflush(stdout);\
			}\
		}while(0)
#	define LOG_DEBUG(format, ...)	\
		do{\
			if (verbosity >= DEBUG_LEVEL)\
			{\
				fprintf(stdout, _GETTEXT("Debug:  " format LOG_LINE_END), \
						##__VA_ARGS__);\
				fflush(stdout);\
			}\
		}while(0)
#	define LOG_BUG(format, ...)		\
		do{\
			fprintf(stderr, _GETTEXT("Bug:    " format LOG_LINE_END), \
					##__VA_ARGS__);\
		}while(0)
#elif 1
#	define LOG_ERROR(format, ...)	\
		do{\
			if (verbosity >= ERROR_LEVEL)\
			{\
				fprintf(stderr, "Error:  %s:%d %s: ", \
						__FILE__, __LINE__, __FUNCTION__);\
				fprintf(stderr, _GETTEXT(format LOG_LINE_END), ##__VA_ARGS__);\
			}\
		}while(0)
#	define LOG_WARNING(format, ...)	\
		do{\
			if (verbosity >= WARNING_LEVEL)\
			{\
				fprintf(stdout, "Warning:%s:%d %s: ", \
						__FILE__, __LINE__, __FUNCTION__);\
				fprintf(stdout, _GETTEXT(format LOG_LINE_END), ##__VA_ARGS__);\
				fflush(stdout);\
			}\
		}while(0)
#	define LOG_INFO(format, ...)	\
		do{\
			if (verbosity >= INFO_LEVEL)\
			{\
				fprintf(stdout, "Info:   %s:%d %s: ", \
						__FILE__, __LINE__, __FUNCTION__);\
				fprintf(stdout, _GETTEXT(format LOG_LINE_END), ##__VA_ARGS__);\
				fflush(stdout);\
			}\
		}while(0)
#	define LOG_DEBUG(format, ...)	\
		do{\
			if (verbosity >= DEBUG_LEVEL)\
			{\
				fprintf(stdout, "Debug:  %s:%d %s: ", \
						__FILE__, __LINE__, __FUNCTION__);\
				fprintf(stdout, _GETTEXT(format LOG_LINE_END), ##__VA_ARGS__);\
				fflush(stdout);\
			}\
		}while(0)
#	define LOG_BUG(format, ...)		\
		do{\
			fprintf(stderr, "Bug:    %s:%d %s: ", \
					__FILE__, __LINE__, __FUNCTION__);\
			fprintf(stderr, _GETTEXT(format LOG_LINE_END), ##__VA_ARGS__);\
		}while(0)
#endif

#endif /* __APP_LOG_H_INCLUDED__ */
