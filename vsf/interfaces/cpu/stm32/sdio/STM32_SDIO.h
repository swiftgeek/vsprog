/**************************************************************************
 *  Copyright (C) 2008 - 2010 by Simon Qian                               *
 *  SimonQian@SimonQian.com                                               *
 *                                                                        *
 *  Project:    Versaloon                                                 *
 *  File:       GPIO.h                                                    *
 *  Author:     SimonQian                                                 *
 *  Versaion:   See changelog                                             *
 *  Purpose:    GPIO interface header file                                *
 *  License:    See license                                               *
 *------------------------------------------------------------------------*
 *  Change Log:                                                           *
 *      YYYY-MM-DD:     What(by Who)                                      *
 *      2012-01-23:     created(by SimonQian)                             *
 **************************************************************************/

vsf_err_t stm32_sdio_init(uint8_t index);
vsf_err_t stm32_sdio_config(uint8_t index, uint16_t kHz, uint8_t buswide);
vsf_err_t stm32_sdio_fini(uint8_t index);
vsf_err_t stm32_sdio_start(uint8_t index);
vsf_err_t stm32_sdio_stop(uint8_t index);
vsf_err_t stm32_sdio_send_cmd(uint8_t index, uint8_t cmd, uint32_t arg,
						uint8_t resp);
vsf_err_t stm32_sdio_send_cmd_isready(uint8_t index, uint8_t resp);
vsf_err_t stm32_sdio_get_resp(uint8_t index, uint8_t *cresp, uint32_t *resp,
						uint8_t resp_num);
vsf_err_t stm32_sdio_data_tx(uint8_t index, uint32_t to_ms,
						uint32_t size, uint32_t block_size);
vsf_err_t stm32_sdio_data_tx_isready(uint8_t index, uint32_t size,
						uint8_t *buffer);
vsf_err_t stm32_sdio_data_rx(uint8_t index, uint32_t to_ms,
						uint32_t size, uint32_t block_size);
vsf_err_t stm32_sdio_data_rx_isready(uint8_t index, uint32_t size,
						uint8_t *buffer);
