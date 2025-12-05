/**
 @file motor.c
 Autor: Arthur Marinho
 @brief Driver do Motor (Versão Otimizada "Light" - Completa)
 Implementa a lógica de movimentação, controle de velocidade (PWM),direção e proteções de hardware (reversão brusca e fim de curso).
*/

#include "motor.h"
#include "globals.h"               
#include "mcc_generated_files/mcc.h" 
#include "mcc_generated_files/pwm3.h"

// VARIÁVEIS INTERNAS (OTIMIZADAS PARA INTEIROS)
// Usamos 'static' para que elas só existam dentro deste arquivo.

// Antes era float. Mudamos para uint16 para economizar memória RAM e FLASH.
static uint16_t total_pulsos = 0;            

// Guarda a leitura anterior do Timer para saber o quanto andou (Delta).
static uint8_t ultimo_valor_timer0 = 0;

// Constante: 0.837 mm/pulso * 1000 = 837.
// Usamos isso para fazer conta com números inteiros e fugir das bibliotecas pesadas de 'float'.
#define MICRONS_POR_PULSO 837 

#define TEMPO_TMR4_MS    100  // Timer 4 roda a cada 100ms

// CÁLCULO DE FÍSICA (VELOCIDADE E POSIÇÃO) - CÓDIGO ATIVO E OTIMIZADO

/**
 * @brief Calcula Velocidade e Posição lendo o Hardware (TMR0).
 * @note Deve ser chamada periodicamente (ex: Timer 4 a cada 100ms).
 */
void SENSORES_CalcularVelocidade(void){
    
    // 1. LEITURA DO ENCODER (HARDWARE)
    // Lê o registrador TMR0 que conta os pulsos físicos do disco do motor.
    uint8_t valor_atual = TMR0_ReadTimer();
    
    // Calcula quantos pulsos aconteceram nesses 100ms (Atual - Anterior).
    // O tipo uint8_t lida automaticamente com o estouro (ex: se foi de 250 para 5, o resultado é 11).
    uint8_t delta = valor_atual - ultimo_valor_timer0;
    
    // Salva o valor atual para a próxima conta.
    ultimo_valor_timer0 = valor_atual;

    // 2. ATUALIZAÇÃO DA POSIÇÃO (CONTAGEM DE PULSOS)
    // Se o motor estiver subindo, somamos os pulsos.
    if (estado_motor == MOTOR_SUBINDO) {
        total_pulsos += delta;
        // Trava de segurança lógica: 220 pulsos é o topo (~180mm).
        // Impede que o número cresça infinitamente se o sensor falhar.
        if(total_pulsos > 220) total_pulsos = 220; 
    } 
    // Se estiver descendo, subtraímos.
    else if (estado_motor == MOTOR_DESCENDO) {
        // Proteção para não ficar negativo (o que seria um erro em 'unsigned').
        if(delta > total_pulsos) total_pulsos = 0; 
        else total_pulsos -= delta;
    }
    
    // 3. CONVERSÃO MATEMÁTICA (INTEIROS)
    // Aqui transformamos "pulsos" em "milímetros" para o celular mostrar.
    // Fórmula Otimizada: mm = (pulsos * 837) / 1000
    
    // Usamos uma variável temporária de 32 bits (uint32) para a multiplicação não estourar o limite de 16 bits.
    uint32_t calculo_posicao = (uint32_t)total_pulsos * MICRONS_POR_PULSO;
    posicao_mm = (uint8_t)(calculo_posicao / 1000); // Guarda na variável global (0-180mm)

    // 4. CÁLCULO DA VELOCIDADE
    // Velocidade = Distância / Tempo. Como o tempo é fixo (0.1s), a conta simplifica.
    // Truque matemático: (delta * 837) / 100 dá a velocidade já na escala mm/s.
    uint32_t calculo_velocidade = (uint32_t)delta * MICRONS_POR_PULSO;
    velocidade_atual = (uint8_t)(calculo_velocidade / 100);
    
    uint16_t leitura_adc = ADC_GetConversion(channel_AN2);

    // Fazemos o cast para uint32_t ANTES de multiplicar para evitar overflow de 16 bits
    // 1023 * 999 = 1.021.977 (cabe em uint32_t, mas não em uint16_t)
    uint32_t calc_temp = (uint32_t)leitura_adc * 999;
    
    // Agora dividimos e guardamos no resultado final
    temperatura_ponte = (uint16_t)(calc_temp / 1023);
    
}

// 1. FUNÇÕES DE CONTROLE DE MOVIMENTO
void Controle_Subir() {
    DIR = DIRECAO_SUBIR;           
    PWM3_LoadDutyValue(MOTOR_ON);
    estado_motor = MOTOR_SUBINDO;
}

void Controle_Descer() {
    DIR = DIRECAO_DESCER;          
    PWM3_LoadDutyValue(MOTOR_ON);  
    estado_motor = MOTOR_DESCENDO;
}

void Controle_Parar() {
    PWM3_LoadDutyValue(MOTOR_OFF); 
    estado_motor = MOTOR_PARADO;
}

// 2. LEITURA DE SENSORES E SEGURANÇA

void Verificar_Sensores() {
    // Atualiza andar atual
    // S1/S2: Digitais (Pull-up - Ativo em 0)
    if (SENSOR_S1 == 0) andar_atual = 0;
    if (SENSOR_S2 == 0) andar_atual = 1;
    // S3/S4: Analógicos (Comparador - Ativo em 1)
    if (SENSOR_S3 == 1) andar_atual = 2; 
    if (SENSOR_S4 == 1) andar_atual = 3; 

    // SEGURANÇA EXTREMA (Fim de Curso)
    // Se bater no chão descendo - PARA
    if (SENSOR_S1 == 0 && estado_motor == MOTOR_DESCENDO) {
        Controle_Parar();
        estado_atual = ESTADO_PARADO;
        posicao_mm = 0; // Recalibra
    }
    // Se bater no teto subindo - PARA
    if (SENSOR_S4 == 1 && estado_motor == MOTOR_SUBINDO) {
        Controle_Parar();
        estado_atual = ESTADO_PARADO;
        posicao_mm = 180; // Recalibra
    }
}

// 3. ALGORITMO DE OTIMIZAÇÃO
int Buscar_Proxima_Parada() {
    // 1. Se parado, atende qualquer um
    if (estado_atual == ESTADO_PARADO) {
        for (int i = 0; i < 4; i++) if (solicitacoes[i]) return i;
        return -1;
    }
    // 2. Se subindo, prioriza quem está ACIMA
    if (estado_atual == ESTADO_SUBINDO) {
        for (int i = andar_atual + 1; i <= 3; i++) {
            if (solicitacoes[i]) return i;
        }
    }
    // 3. Se descendo, prioriza quem está ABAIXO
    if (estado_atual == ESTADO_DESCENDO) {
        for (int i = andar_atual - 1; i >= 0; i--) {
            if (solicitacoes[i]) return i;
        }
    }
    // 4. Se não achou no sentido, varre tudo
    for (int i = 0; i < 4; i++) if (solicitacoes[i]) return i;
    
    return -1;
}