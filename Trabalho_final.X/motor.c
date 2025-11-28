/**
 * @file motor.c
 * @author Arthur Marinho
 * @brief Driver de controle de baixo nível para a Ponte H TB6612FNG.
 *
 * Implementa a lógica de movimentação, controle de velocidade (PWM),
 * direção e proteções de hardware (reversão brusca e fim de curso).
 */

#include "motor.h"
#include "globals.h"               // Para atualizar estado_motor (M na tabela)
#include "mcc_generated_files/mcc.h" // Para usar PWM e Delay

/** * @brief Armazena a última direção aplicada para evitar reversão brusca.
 * Inicializa como SUBINDO por segurança.
 */
static uint8_t ultima_direcao_aplicada = DIRECAO_SUBIR;

/** * @brief Acumulador de posição com precisão decimal (float).
 * Usado internamente para não perder os 0.83mm a cada pulso.
 */
static float posicao_mm_fina = 0.0;            

/** * @brief Contador de pulsos para o cálculo de velocidade.
 * Reiniciado a cada ciclo do Timer 4 (100ms).
 */
static uint16_t pulsos_para_velocidade = 0;


/**
 * @brief Armazena o estado anterior do pino RA4 para detectar bordas.
 * Inicializa em 1 (High) assumindo Pull-Up ativo.
 */
static bool ultimo_estado_ra4 = 1;

// Constantes Físicas
#define MM_POR_PULSO  0.8372f  ///< 180mm / 215 pulsos
#define TEMPO_TMR4    0.1f     ///< Tempo do Timer de Velocidade (100ms = 0.1s)
/**
 * @brief Para o motor imediatamente.
 * Define o Duty Cycle do PWM para 0 e atualiza o estado global.
 */
void MOTOR_parar(void){
    // Nota: Verifique se o MCC gerou 'PWM3_LoadDutyValue' ou 'CCP1_LoadDutyValue'
    PWM3_LoadDutyValue(MOTOR_OFF); 
    estado_motor = MOTOR_PARADO;
}

/**
 * @brief Realiza o procedimento de Calibração Inicial.
 * 1. Detecta o andar atual lendo os sensores diretamente.
 * 2. Se não estiver no térreo, força a descida até encontrar o sensor S1.
 * 3. Zera todas as variáveis de posição e solicitações.
 * @note Esta função é bloqueante (trava o sistema até calibrar).
 */
void MOTOR_reset(void){
    
    // --- 1. DETECÇÃO INICIAL DE POSIÇÃO ---
    // Verifica os sensores físicos para estimar onde o elevador ligou.
    // S1/S2 são Active Low (0). S3/S4 são Active High (1) via Comparador.
    
    if(SENSOR_S1 == 0){
        andar_atual = 0;
    }
    else if (SENSOR_S2 == 0){
        andar_atual = 1;
    }
    else if(SENSOR_S3 == 1){
        andar_atual = 2;
    }
    else if(SENSOR_S4 == 1){ 
        andar_atual = 3;
    }
    else {
        // Caso nenhum sensor esteja ativado (Elevador entre andares)
        andar_atual = 255; 
    }
   
    // --- 2. MOVIMENTAÇÃO PARA O TÉRREO ---
    if(andar_atual != 0){
        andar_destino = 0;
        
        // Se estiver "perdido" (255), simulamos que estamos no topo (4)
        // para garantir que a lógica decida DESCER.
        uint8_t origem_simulada = (andar_atual == 255) ? 4 : andar_atual;
        
        MOTOR_mover(andar_destino, origem_simulada);
    }
    
    // --- 3. RESET DE VARIÁVEIS ---
    MOTOR_parar();
    andar_atual = 0;
    posicao_mm = 0;
    andar_destino = 0;
    estado_motor = MOTOR_PARADO;
    
    // Limpa a fila de chamadas
    for(uint8_t i=0; i<4; i++) {
        solicitacoes[i] = false;
    }
}

/**
 * @brief Move o elevador de um andar de origem até o destino.
 * * Implementa lógica "Passo a Passo": o elevador viaja andar por andar,
 * parando brevemente em cada sensor até atingir o destino.
 * Inclui proteção de 500ms contra reversão de sentido.
 * * @param destino Andar alvo (0 a 3).
 * @param atual Andar atual (0 a 3, ou 255 se desconhecido).
 */
void MOTOR_mover (uint8_t destino, uint8_t atual)
{
    // --- 1. VALIDAÇÕES DE SEGURANÇA (BOUNDS CHECK) ---
    if (atual > 3 && atual != 255) { // Proteção contra valores corrompidos
        MOTOR_parar(); return;
    }
    if (destino == atual){ // O elevador está no andar do destino
        MOTOR_parar(); return;
    }
   
    // --- 2. DECISÃO DE DIREÇÃO ---
    // Apenas decide a intenção, NÃO aplica no pino ainda.
    uint8_t nova_direcao;

    if(destino > atual){
        nova_direcao = DIRECAO_SUBIR;
    }
    else {
        nova_direcao = DIRECAO_DESCER;
    }
   
    // --- 3. PROTEÇÃO DE HARDWARE (PONTE H) ---
    // Regra do Roteiro: Esperar 500ms se inverter o sentido em movimento.
    if ((estado_motor != MOTOR_PARADO) && (nova_direcao != ultima_direcao_aplicada)) {
        MOTOR_parar();      
        __delay_ms(500);    
    }
   
    // --- 4. APLICAÇÃO NO HARDWARE ---
    // Agora é seguro mudar o pino DIR.
    if(nova_direcao == DIRECAO_SUBIR){
        DIR = DIRECAO_SUBIR;
        estado_motor = MOTOR_SUBINDO;
    }
    else {
        DIR = DIRECAO_DESCER;
        estado_motor = MOTOR_DESCENDO;
    }
   
    ultima_direcao_aplicada = nova_direcao;
   
    // --- 5. LOOP DE MOVIMENTO (BLOQUEANTE) ---
    // Mantém o motor ligado até chegar no destino final.
    while(atual != destino){
       
        PWM3_LoadDutyValue(MOTOR_ON); // Liga o motor (~60%)
       
        // Loop de Espera: Aguarda até ALGUM sensor ser acionado
        // S1/S2 = 1 (Desativado), S3/S4 = 0 (Desativado)
        while( (SENSOR_S1 == 1) && (SENSOR_S2 == 1) && 
               (SENSOR_S3 == 0) && (SENSOR_S4 == 0) ) 
        {
            // Proteção de Fim de Curso:
            // Se estiver descendo e bater no S1, sai do loop imediatamente.
            if (DIR == DIRECAO_DESCER && SENSOR_S1 == 0) break;
            
            // Se estiver subindo e bater no S4, sai do loop imediatamente.
            if (DIR == DIRECAO_SUBIR && SENSOR_S4 == 1) break;
        }
       
        // Sensor detectado! Para o motor para atualizar posição.
        MOTOR_parar();
        
        // Atualiza a variável 'atual' matematicamente para o próximo passo
        if(DIR == DIRECAO_DESCER) atual--;
        else if (DIR == DIRECAO_SUBIR) atual++;
       
        // Pequeno delay para estabilização mecânica
        __delay_ms(100);
    }
   
    // Chegou no destino final
    MOTOR_parar();
    andar_atual = atual; // Sincroniza a variável global
}

/**
 * @brief Cálculo Periódico de Física (Posição e Velocidade).
 * @note **FREQUÊNCIA: 10Hz (100ms)**
 * Executa toda a matemática do sistema baseada nos pulsos acumulados
 * na última janela de tempo.
 * * @see Chamada pelo Callback do Timer 4.
 */
void SENSORES_CalcularVelocidade(void){
    
    // 1. Calcula quantos milímetros andamos NESTA janela de 100ms
    // (Pulsos * 0.8372mm)
    float distancia_percorrida_janela = (float)pulsos_para_velocidade * MM_POR_PULSO;
    
    // 2. ATUALIZAÇÃO DA POSIÇÃO (ODOMETRIA)
    if (estado_motor == MOTOR_SUBINDO) {
        posicao_mm_fina += distancia_percorrida_janela;
        
        // Trava de segurança no teto (180mm)
        if (posicao_mm_fina > 180.0f){
            posicao_mm_fina = 180.0f;
        }
    } 
    else if (estado_motor == MOTOR_DESCENDO) {
        posicao_mm_fina -= distancia_percorrida_janela;
        
        // Trava de segurança no chão (0mm)
        if (posicao_mm_fina < 0.0f){
            posicao_mm_fina = 0.0f;
        }    
    }
    
    // Atualiza a variável global 
    posicao_mm = (uint8_t)posicao_mm_fina;

    // 3. CÁLCULO DA VELOCIDADE
    // V = Distância percorrida na janela / Tempo da janela
    velocidade_atual = distancia_percorrida_janela / TEMPO_TMR4;
    
    // 4. RESET
    // Zera o contador para começar a medir a próxima janela limpa
    pulsos_para_velocidade = 0;
}


/**
 * @brief Rotina de Serviço de Interrupção (ISR) do Encoder.
 * Esta função apenas incrementa um contador.
 * * @see Chamada pelo Callback do IOC (Pino RA4).
 */
void MOTOR_Encoder_ISR(void) {
    // contar pulsos brutos toda fez que detectar a borda de subida do enconder
    pulsos_para_velocidade++; 
}


/**
 * @brief Verifica manualmente se o pino RA4 mudou de 0 para 1 (Borda de Subida).
 * * Esta função deve ser chamada repetidamente dentro dos loops de movimento
 * para garantir que nenhum pulso do encoder seja perdido.
 */
void MOTOR_VerificarEncoder(void) {
    // 1. Leitura Física: Captura o estado atual do pino RA4
    // (Nota: IO_RA4_GetValue() é uma macro do MCC, ou use PORTAbits.RA4)
    bool estado_atual = PORTAbits.RA4; 
    
    // 2. Detecção de Borda de SUBIDA (Rising Edge)
    // Se estava 0 (Low) e foi para 1 (High)...
    if (ultimo_estado_ra4 == 0 && estado_atual == 1) {
        
        // Borda detectada! Chama a lógica de contagem.
        MOTOR_Encoder_ISR();
    }
    
    // 3. Atualização de Histórico: O atual vira o "último" para a próxima checagem
    ultimo_estado_ra4 = estado_atual;
}
