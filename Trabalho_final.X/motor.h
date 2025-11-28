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
 * Zera todas as variáveis de posição ao finalizar.
 */
void MOTOR_reset(void);

/**
 * @brief Move o elevador até o andar de destino.
 * Função bloqueante que gerencia o movimento passo-a-passo, proteções de
 * reversão e leitura constante do encoder via Polling.
 * @param destino Andar alvo (0-3).
 * @param atual Andar atual (0-3).
 */
void MOTOR_mover(uint8_t destino, uint8_t atual);

// ==========================================
// FUNÇÕES DE SENSORES (ENCODER E VELOCIDADE)
// ==========================================

/**
 * @brief Função de Polling do Encoder (Substitui Interrupção RA4).
 * Lê o estado atual do pino RA4 e compara com o anterior.
 * Se detectar borda de subida, chama MOTOR_Encoder_ISR.
 * @note Deve ser chamada repetidamente dentro de loops de espera.
 */
void MOTOR_VerificarEncoder(void);

/**
 * @brief Lógica interna de contagem de pulsos.
 * Chamada automaticamente pela função MOTOR_VerificarEncoder.
 * Incrementa o contador bruto de pulsos para o cálculo de velocidade.
 */
void MOTOR_Encoder_ISR(void);

/**
 * @brief Callback de baixa frequência (100ms) para cálculos físicos.
 * Transforma pulsos contados em velocidade (mm/s) e atualiza posição (mm).
 * @note Deve ser registrado no TMR4_SetInterruptHandler no main.c.
 */
void SENSORES_CalcularVelocidade(void);

#endif	/* MOTOR_H */