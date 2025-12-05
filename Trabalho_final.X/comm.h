/**
 * @file comm.h
 * @author Gabriel Celestino
 * @brief Cabeçalho da comunicação e interface.
 * @date Created on November 30, 2025
 */

#ifndef COMM_H
#define	COMM_H

#include <stdint.h>

/* * DECLARAÇÕES DE CONSTANTES (EXTERN)
 * O 'extern' avisa ao compilador que os valores estão no arquivo .c
 * Isso evita erro de "Multiple Definition" ao incluir em vários lugares.
 */
extern const uint8_t LUT_Andar[];
extern const uint8_t LUT_dir[];
extern const uint8_t matrix_conf[];

/**
 * @brief Recebe e salva os dados da UART no endereço designado.
 * Retorna 0 caso último caractere recebido seja <CR>.
 * Retorna 1 caso último caractere recebido seja diferente de <CR>.
 */
int UART_RecebePedido(char* OrigemPedido, char* DestinoPedido);

/*
 * @brief Acessa as variaveis globais e transmite pela UART
 */
void UART_EnviaDados(void);

/*
 * Pega os valores das variaveis globais e atualiza a matriz de LEDs
 */
void MatrizLed (void);

/*
 * Inicializa a Matriz (Envia configurações iniciais)
 */
void MatrizInicializa(void);

#endif	/* COMM_H */