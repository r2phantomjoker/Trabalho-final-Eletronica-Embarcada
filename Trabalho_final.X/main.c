/**
 * @file main.c
 * @brief Teste de Integração: Motor, Odometria (TMR0) e Lógica de Controle.
 */

#include "mcc_generated_files/mcc.h"
#include "motor.h"
#include "globals.h"
#include "comm.h"

void main(void)
{
    // 1. INICIALIZAÇÃO DO SISTEMA
    SYSTEM_Initialize();

    // 2. CONEXÃO DO VELOCÍMETRO (TIMER 4)
    // Registra a função que lê o hardware TMR0 e calcula a física a cada 100ms.
    TMR4_SetInterruptHandler(SENSORES_CalcularVelocidade);

    // 3. HABILITA INTERRUPÇÕES (Crucial para o Timer 4 funcionar)
    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();

    // =========================================================
    // TESTE 1: CALIBRAÇÃO (HOMING)
    // =========================================================
    // O código vai parar aqui e ligar o motor para DESCER.
    // [AÇÃO NO STIMULUS]: Coloque o pino RB0 (S1) em LOW (0) para simular chegada no chão.
    MOTOR_reset(); 

    // Se passou daqui, 'andar_atual' deve ser 0 e 'posicao_mm' deve ser 0.

    while (1)
    {
       
    }
}