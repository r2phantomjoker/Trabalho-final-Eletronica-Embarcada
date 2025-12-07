/**
 * @file motor.h
 * @brief Cabeçalho do driver de motor e sensores.
 * @details Este arquivo contém as declarações das funções de controle de
 * movimentação, leitura de sensores e lógica de escalonamento do elevador.
 * @date Created on November 27, 2025
 */

#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

// ================================
// FUNÇÕES DE TELEMETRIA E SENSORES
// ================================

/**
 * @brief Atualiza a telemetria do sistema (Velocidade e Posição).
 * @details Esta função deve ser chamada periodicamente para garantir
 * a precisão do cálculo da velocidade do encoder.
 */
void SENSORES_CalcularVelocidade(void);

/**
 * @brief Lê os sensores de andar.
 * @details Atualiza a variável global de "andar atual" e verifica colisões
 * (segurança de hardware) nos extremos do elevador.
 */
void Verificar_Sensores(void);

// ================================
// FUNÇÕES DE CONTROLE DE MOVIMENTO
// ================================

/**
 * @brief Ativa o motor para subir o elevador.
 * @note Configura o PWM e a direção automaticamente.
 */
void Controle_Subir(void);

/**
 * @brief Ativa o motor para descer o elevador.
 * @note Configura o PWM e a direção automaticamente.
 */
void Controle_Descer(void);

/**
 * @brief Para o motor imediatamente.
 * @details Desliga o PWM e coloca o estado do motor em repouso.
 */
void Controle_Parar(void);

// ====================
// ALGORITMOS DE LÓGICA
// ====================

/**
 * @brief Executa o algoritmo de decisão de paradas.
 * Verifica a lista de solicitações e decide qual é o próximo andar alvo
 * com base na posição atual e direção do movimento.
 * @return int Índice do próximo andar (0 a 3) ou -1 se não houver chamadas.
 */
int Buscar_Proxima_Parada(void);

#endif	/* MOTOR_H */