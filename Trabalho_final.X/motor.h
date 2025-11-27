/**
 * @file motor.h
 * @author Arthur Marinho
 * @brief Cabeçalho do driver de motor e sensores.
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
 */
void MOTOR_parar(void);

/**
 * @brief Realiza a calibração inicial (Homing).
 * Bloqueia o sistema até encontrar o térreo (S1).
 */
void MOTOR_reset(void);

/**
 * @brief Move o elevador até o andar de destino.
 * Função bloqueante que gerencia a rampa de movimento passo-a-passo.
 * @param destino Andar alvo (0-3).
 * @param atual Andar atual (0-3).
 */
void MOTOR_mover(uint8_t destino, uint8_t atual);

// ==========================================
// FUNÇÕES DE CALLBACK (INTERRUPÇÕES)
// ==========================================

/**
 * @brief Callback de alta frequência do Encoder (IOC).
 * Deve ser registrado no IOCAF4_SetInterruptHandler no main.c.
 */
void MOTOR_Encoder_ISR(void);

/**
 * @brief Callback de baixa frequência (100ms) para cálculos físicos.
 * Calcula posição (mm) e velocidade (mm/s).
 * Deve ser registrado no TMR4_SetInterruptHandler no main.c.
 */
void SENSORES_CalcularVelocidade(void);

#endif	/* MOTOR_H */