/**
 * @file main.c
 * @brief Lógica Avançada: SCAN (Elevador Inteligente) com Display Sincronizado
 */

#include "mcc_generated_files/mcc.h"
#include "globals.h"
#include "comm.h"
#include "motor.h"

// --- VETORES DE SOLICITAÇÃO (Lógica Interna) ---
bool chamadas_subida[4] = {false, false, false, false};  
bool chamadas_descida[4] = {false, false, false, false}; 

// VARIÁVEIS DE CONTROLE
uint16_t contador_telemetria = 0;
uint16_t contador_espera = 0;
char buffer_origem, buffer_destino;

// --- FUNÇÕES DE LÓGICA ---

bool Existe_Chamada_Acima(uint8_t andar_ref) {
    for (int i = andar_ref + 1; i <= 3; i++) {
        if (chamadas_subida[i] || chamadas_descida[i]) return true;
    }
    return false;
}

bool Existe_Chamada_Abaixo(uint8_t andar_ref) {
    for (int i = 0; i < andar_ref; i++) {
        if (chamadas_subida[i] || chamadas_descida[i]) return true;
    }
    return false;
}

void Limpar_Chamada_Atual() {
    if (estado_atual == ESTADO_SUBINDO || estado_atual == ESTADO_PARADO) {
        chamadas_subida[andar_atual] = false;
    }
    if (estado_atual == ESTADO_DESCENDO || estado_atual == ESTADO_PARADO) {
        chamadas_descida[andar_atual] = false;
    }
    // Caso especial: Chegou nos extremos, limpa a direção impossível
    if (andar_atual == 3) chamadas_subida[3] = false; 
    if (andar_atual == 0) chamadas_descida[0] = false;
}

// --- LOOP PRINCIPAL ---
void main(void) {
    SYSTEM_Initialize();

    // 1. Configurações de Hardware
    ANSELB = 0x00;  
    TRISBbits.TRISB1 = 0;  
    LATBbits.LATB1 = 1;    
    INTCONbits.IOCIE = 0;  
    TMR4_SetInterruptHandler(SENSORES_CalcularVelocidade);

    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    
    // Reset SPI
    SSP1CON1bits.SSPEN = 0; 
    SSP1CON1bits.SSPEN = 1; 

    Controle_Parar(); 
    MatrizInicializa();

    while (1) {
        // A. COMUNICAÇÃO BLUETOOTH
        if(EUSART_is_rx_ready()) {
            if (UART_RecebePedido(&buffer_origem, &buffer_destino) == 0) { 
                int origem = buffer_origem - '0';
                int destino = buffer_destino - '0';
                
                if (origem >= 0 && origem <= 3 && destino >= 0 && destino <= 3) {
                    if (origem < destino) { 
                        chamadas_subida[origem] = true;
                        chamadas_subida[destino] = true;
                    } 
                    else if (origem > destino) {
                        chamadas_descida[origem] = true;
                        chamadas_descida[destino] = true;
                    }
                }
            }
        }

        // B. SENSORES
        Verificar_Sensores();

        // C. MÁQUINA DE ESTADOS
        switch (estado_atual) {
            case ESTADO_PARADO:
                if (chamadas_subida[andar_atual]) {
                    Limpar_Chamada_Atual(); 
                    estado_atual = ESTADO_ESPERA_PORTA;
                    contador_espera = 0;
                }
                else if (chamadas_descida[andar_atual]) {
                    Limpar_Chamada_Atual();
                    estado_atual = ESTADO_ESPERA_PORTA;
                    contador_espera = 0;
                }
                else if (Existe_Chamada_Acima(andar_atual)) {
                    Controle_Subir();
                    estado_atual = ESTADO_SUBINDO;
                }
                else if (Existe_Chamada_Abaixo(andar_atual)) {
                    Controle_Descer();
                    estado_atual = ESTADO_DESCENDO;
                }
                else if (andar_atual != 0) {
                    chamadas_descida[0] = true; // Homing
                }
                break;
            
            case ESTADO_SUBINDO:
                if (chamadas_subida[andar_atual]) {
                    Controle_Parar();
                    Limpar_Chamada_Atual();
                    estado_atual = ESTADO_ESPERA_PORTA;
                    contador_espera = 0;
                }
                else if (!Existe_Chamada_Acima(andar_atual)) {
                    if (chamadas_descida[andar_atual]) {
                        Controle_Parar();
                        Limpar_Chamada_Atual();
                        estado_atual = ESTADO_ESPERA_PORTA;
                        contador_espera = 0;
                    } else {
                        Controle_Parar();
                        estado_atual = ESTADO_PARADO;
                    }
                }
                break;

            case ESTADO_DESCENDO:
                if (chamadas_descida[andar_atual]) {
                    Controle_Parar();
                    Limpar_Chamada_Atual();
                    estado_atual = ESTADO_ESPERA_PORTA;
                    contador_espera = 0;
                }
                else if (!Existe_Chamada_Abaixo(andar_atual)) {
                    if (chamadas_subida[andar_atual]) {
                         Controle_Parar();
                         Limpar_Chamada_Atual();
                         estado_atual = ESTADO_ESPERA_PORTA;
                         contador_espera = 0;
                    } else {
                        Controle_Parar();
                        estado_atual = ESTADO_PARADO;
                    }
                }
                break;

            case ESTADO_ESPERA_PORTA:
                contador_espera++;
                if (contador_espera >= 200) { 
                    estado_atual = ESTADO_REVERSAO;
                    contador_espera = 0;
                }
                break;

            case ESTADO_REVERSAO:
                contador_espera++;
                if (contador_espera >= 50) { 
                    estado_atual = ESTADO_PARADO; 
                }
                break;
        }

        // D. TELEMETRIA E VISUALIZAÇÃO
        contador_telemetria++;
        if (contador_telemetria >= 30) { 
            UART_EnviaDados();
            
            // --- SINCRONIZAÇÃO PARA OS LEDS (O Truque) ---
            // Atualiza o vetor antigo 'solicitacoes' combinando subida e descida
            // para que a função MatrizLed() na comm.c desenhe os pontos corretamente.
            for(int i=0; i<4; i++) {
                solicitacoes[i] = chamadas_subida[i] || chamadas_descida[i];
            }
            
            MatrizLed();
            contador_telemetria = 0; 
        }

        __delay_ms(10);
    }
}