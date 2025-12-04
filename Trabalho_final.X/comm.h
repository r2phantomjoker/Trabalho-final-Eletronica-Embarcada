/**
 * @file comm.h
 * @author Gabriel Celestino
 * @brief Cabeçalho da comunicação e  interface.
 * @date Created on November 30, 2025
 */

#ifndef COMM_H
#define	COMM_H

#include <stdint.h>

/**
 * @brief Recebe e salva os dados da UART no endereço designado.
 * Retorna 0 caso último caractere recebido seja  <CR>.
 * Retorna 1 caso último caractere recebido seja diferente de <CR>.
 * @note Os dados não recebem tratamento aqui.
 */
int UART_RecebePedido(char* OrigemPedido, char* DestinoPedido);

/*
 * @brief Acessa as variaveis globais e transmite pela UART
 * Os valores já são convertidos dentro da transmissão sem interferir nos valores.
 */
void UART_EnviaDados(void);

#endif	/* COMM_H */

