/**
 * @file globals.h
 * @author Arthur Marinho
 * @date Created on November 27, 2025, 11:17 AM
 * @brief Definições globais, constantes de hardware e variáveis de estado do Elevador.
 *
 * Este arquivo centraliza as variáveis exigidas para a telemetria (Tabela 1 do Roteiro)
 * e permite o compartilhamento de dados entre os módulos de Motor, Sensores e UART.
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <xc.h>         // Necessário para reconhecer LATAbits
#include <stdint.h>     // Necessário para uint8_t, int16_t, etc.
#include <stdbool.h>    // Necessário para o tipo bool

// ==========================================
// MÁQUINA DE ESTADOS E CONTROLE
// ==========================================

/** * @brief Estados possíveis do motor (Variável 'M' na Tabela 1). 
 */
#define MOTOR_PARADO    0   ///< Motor desligado e travado
#define MOTOR_SUBINDO   1   ///< Movimento ascendente (Direção = 1)
#define MOTOR_DESCENDO  2   ///< Movimento descendente (Direção = 0)

// ==========================================
// MAPEAMENTO DE HARDWARE (Camada HAL)
// ==========================================

/** * @brief Controle do Pino de Direção (RA7).
 * Usa o registrador LAT para escrita segura na saída.
 */
#define DIR             LATAbits.LATA7 
#define DIRECAO_SUBIR   1   ///< Define pino RA7 em Nível Alto (Subida)
#define DIRECAO_DESCER  0   ///< Define pino RA7 em Nível Baixo (Descida)

/**
 * @brief Valores de PWM (0 a 1023 - 10 bits).
 * O roteiro sugere ciclo útil entre 40% e 50%[cite: 388].
 * 614 = ~60% de Duty Cycle.
 */
#define MOTOR_OFF       0
#define MOTOR_ON        614 

// ==========================================
// VARIÁVEIS GLOBAIS (TELEMETRIA - TABELA 1)
// ==========================================

/**
 * @brief 'Andar (A)': Último andar detectado pelos sensores.
 * Faixa: 0 a 3.
 */
extern volatile uint8_t andar_atual;

/**
 * @brief 'Destino (D)': Andar alvo da solicitação atual.
 * Faixa: 0 a 3.
 */
extern volatile uint8_t andar_destino;

/**
 * @brief 'Motor (M)': Estado atual do movimento.
 * Valores: 0 (Parado), 1 (Subindo), 2 (Descendo).
 */
extern volatile uint8_t estado_motor;

/**
 * @brief 'Posição (HHH)': Altura em milímetros em relação ao solo.
 * Faixa: 0 a 180 mm[cite: 385].
 */
extern volatile uint8_t posicao_mm;

/**
 * @brief 'Velocidade (VV.V)': Velocidade instantânea calculada via Encoder.
 * Unidade: mm/s.
 */
extern volatile float velocidade_atual;

/**
 * @brief 'Temperatura (TT.T)': Temperatura monitorada na Ponte H.
 * Unidade: Graus Celsius (°C).
 */
extern volatile float temperatura_ponte;

// ==========================================
// VARIÁVEIS DE LÓGICA INTERNA
// ==========================================

/**
 * @brief Fila de solicitações de chamadas.
 * Índice [0] = Térreo ... [3] = 3º Andar.
 * true = Botão pressionado / Solicitação ativa.
 */
extern volatile bool solicitacoes[4];

#endif /* GLOBALS_H */