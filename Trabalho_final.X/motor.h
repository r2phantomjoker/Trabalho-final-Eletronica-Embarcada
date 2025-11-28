/**
 * @file motor.h
 * @author Arthur Marinho
 * @brief Cabeçalho do driver de motor e sensores (Controle e Odometria).
 * @date Created on November 27, 2025
 */

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

// ==========================================
// FUNÇÕES DE CONTROLE DE MOVIMENTO
// ==========================================

/**
 * @brief Para o motor imediatamente e atualiza o estado global.
 * Zera o PWM e define o estado como MOTOR_PARADO.
 */
void MOTOR_parar(void);

/**
 * @brief Realiza a calibração inicial (Homing).
 * Bloqueia o sistema, forçando a descida até encontrar o sensor S1 (Térreo).
 * Zera todas as variáveis de posição e o hardware do Timer 0.
 */
void MOTOR_reset(void);

/**
 * @brief Move o elevador até o andar de destino.
 * Função bloqueante que gerencia o movimento passo-a-passo.
 * O Timer 0 (hardware) continua contando pulsos automaticamente durante a execução.
 * @param destino Andar alvo (0-3).
 * @param atual Andar atual (0-3).
 */
void MOTOR_mover(uint8_t destino, uint8_t atual);

// ==========================================
// FUNÇÕES DE SENSORES (CALLBACKS)
// ==========================================

/**
 * @brief Callback de baixa frequência (100ms) para cálculos físicos.
 * Lê o registrador de hardware TMR0 (Encoder), calcula a velocidade (mm/s)
 * e atualiza a posição (mm).
 * @note Deve ser registrado no TMR4_SetInterruptHandler no main.c.
 */
void SENSORES_CalcularVelocidade(void);

#endif	/* MOTOR_H */