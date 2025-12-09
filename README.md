# Controle de Elevador de 4 Andares por Bluetooth

Este projeto de elevador foi desenvolvido para a disciplina de Eletrônica Embarcada 2025/2 da Universidade de Brasília, pelos alunos Arthur Marinho, Fernando de Melo e Gabriel Celestino. O principal objetivo deste projeto foi desenvolver um firmware para um elevador de 4 andares, utilizando um algoritmo de escalonamento SCAN para otimização do percurso. Além disso, o projeto conta com telemetria em tempo real via UART e interface visual por meio de uma Matriz de LEDs.

## Software Utilizado

Para a elaboração do projeto, foi utilizado o **MPLAB X IDE 6.25** para o desenvolvimento do firmware e para a testagem do sistema embarcado.

## Hardware Utilizado

* **Bluetooth HC-06**
* **PIC16F1827 (5V)**
* **Motor CC com engrenagem**
* **Ponte H TB6612FNG**
* **Sensores de Efeito Hall A3144**
* **Encoder óptico + disco**
* **Matriz de LEDs com MAX7219**
* **Sensor LM35**

## Funcionalidades

* **Algoritmo SCAN:** Prioriza chamadas no sentido do movimento atual, otimizando a rota do elevador.
* **Interface Visual (MAX7219):** A parte da direita exibe o andar atual do elevador, enquanto a esquerda mostra o estado do elevador (subindo, descendo, parado ou abrindo a porta) e os andares solicitados na rota.
* **Telemetria UART:** Envia dados via Bluetooth com informações sobre o andar atual, destino, status do motor, posição (em mm), velocidade e temperatura.
* **Controle PWM:** Lógica responsável pelo controle de velocidade e direção do motor.

## Pinagem

| Pino PIC | Função | Descrição |
| :--- | :--- | :--- |
| **RA0** | Entrada | Sensor 3º Andar (S3) |
| **RA1** | Entrada | Sensor 4º Andar (S4) |
| **RA2** | Entrada | ADC |
| **RA3** | Saída | PWM |
| **RA4** | Entrada | Encoder |
| **RA5** | Entrada | MCLR |
| **RA6** | Saída | SDO |
| **RA7** | Saída | DIR |
| **RB0** | Entrada | Sensor 1º Andar (S1) |
| **RB1** | Saída | CS |
| **RB2** | Entrada | Rx |
| **RB3** | Entrada | Sensor 2º Andar (S2) |
| **RB4** | Saída | SCK |
| **RB5** | Saída | Tx |
| **RB6** | Entrada | CSPCLK |
| **RB7** | Entrada/Saída | ICSDAT |

## Protocolo de Comunicação

### Protocolo de Entrada de Dados

O sistema processa até 5 solicitações, utilizando o seguinte formato CSV:

`$OD<CR>`

* **$**: Cabeçalho.
* **O**: Origem do elevador (0-3).
* **D**: Andar Destino (0-3).
* **<CR>**: Carriage Return (fim de linha).

### Protocolo de Saída de Dados

O sistema envia pacotes de telemetria via UART com baud rate de 19600, no seguinte formato CSV:

`$A,D,M,PPP,VV.V,TT.T<CR>`

* **$**: Cabeçalho.
* **A**: Andar Atual (0-3).
* **D**: Andar Destino (0-3).
* **M**: Estado do Motor (0=Parado, 1=Subindo, 2=Descendo).
* **PPP**: Posição em mm (ex: 180).
* **VV.V**: Velocidade em mm/s (ex: 12.5).
* **TT.T**: Temperatura em °C (ex: 45.0).
* **<CR>**: Carriage Return (fim de linha).

## Interface na Matriz de LEDs (MAX7219)

### Colunas 1 a 4:
* **Linhas de 7 a 4:** Andares presentes nas solicitações de movimento do elevador.
* **Linhas de 0 a 2:** Estado atual do elevador: *↑* - Elevador subindo, *↓* - Elevador descendo, *-* - Elevador esperando a porta abrir/fechar, **" "** - Elevador parado.

### Colunas 5 a 8: Andar atual do elevador.

## Máquina de Estados

O sistema opera com base em 5 estados:

1. **PARADO:** Aguardando chamadas.
2. **SUBINDO:** Motor ativo, monitorando sensores acima.
3. **DESCENDO:** Motor ativo, monitorando sensores abaixo.
4. **ESPERA_PORTA:** Temporização de 2 segundos para embarque/desembarque.
5. **REVERSÃO:** Tempo de segurança de 0.5 segundos antes de inverter a rotação.

## Estrutura do Firmware

O código é modularizado para facilitar a manutenção e compreensão do projeto:

* `main.c`: Loop principal, inicialização e orquestração das tarefas.
* `motor.c`: Driver de controle de hardware, PWM, sensores de efeito Hall, temperatura e encoder, além das funções de lógica relacionadas às solicitações.
* `comm.c`: Driver de controle dos LEDs e comunicação UART.
* `globals.c`: Alocação de variáveis globais e flags de estado.

## Como Rodar

1. Abra o projeto no **MPLAB X IDE**.
2. Certifique-se de ter o compilador **XC8** instalado.
3. Compile e grave no microcontrolador.
4. Utilize um programa como **Serial Bluetooth Terminal** para a comunicação Bluetooth e ler os dados pelo terminal.
5. Execute o código Python presente na pasta **elevator1x4** para ler a telemetria do elevador, conectando o elevador via Bluetooth.

## Vídeo
Vídeo explicativo do projeto, detalhes sobre o código utilizado, configurações do MCC, simulações feitas no Debugger e testes realizados no elevador com telemetria em tempo real: 
- [Trabalho final de EE- 2025/2 - Grupo 1](https://youtu.be/C-G2z3W_Hf0?si=PeSgyDbds9OFjuQ4)

## Fluxograma Lógico do Projeto

```mermaid
flowchart TB
    A(["Inicializa configurações
    básicas"]) --> 
    B{"UART
    recebeu algo?"}
    style B stroke:#2962FF,fill:#BBDEFB,color:#000000
    style b1 stroke:#2962FF,fill:#BBDEFB,color:#000000
    style b1t stroke:#2962FF,fill:#BBDEFB,color:#000000
    
    %% Estilo para os novos nós de decisão
    style E stroke:#2962FF,fill:#BBDEFB,color:#000000
    style F stroke:#2962FF,fill:#BBDEFB,color:#000000

    B -- Sim --> b1{"Dados 
    recebidos 
    são válidos?"}
    B -- Não --> 
    C["Realiza leitura de sensores"]
    b1 -- Sim --> b1t{"Andar
    origem &lt;  destino
    ?"}
    b1 -- não --> C
    b1t -- Origem >
    Destino --> b1t1["chamadas_subida = true"]
    b1t -- Origem &lt;
    Destino --> b1t2["chamadas_descida = true"]
    b1t1 & b1t2 --> C
    C --> c1["Identifica andar atual"]
    c1 --> c2{"Está se movendo
    e em um limite?"}
    c2 --Sim--> c2t["Desliga motor"]
    c2t --> D
    c2 --Não --> 
    D[Verifica estado atual]

    %% --- NOVOS NÓS (A PARTIR DE E) ---
    D --> E{"Há chamadas
    pendentes?"}
    
    E -- Sim --> F{"Qual a direção
    necessária?"}
    E -- Não --> I["Manter estado
    (Motor Parado)"]

    F -- Subir --> G["Acionar Motor
    Subida"]
    F -- Descer --> H["Acionar Motor
    Descida"]

    G & H & I --> J["Atualizar Matriz de LEDs
    e envio de dados via UART"]

    J --> B

    %% LEITURA DE SENSORES

    K([Disparo da Rotina
    Timer / Interrupção]) --> L["Ler TMR0
    delta = atual - último
    Atualiza último_valor"]
    
    L --> M{"Qual o estado
    do motor?"}
    
    %% Ramificação de Subida
    M -- Subindo --> N["total_pulsos += delta"]
    N --> O{"Passou do limite
    superior?"}
    O -- Sim --> O1["Altura no máximo"]
    O -- Não --> R
    O1 --> R

    %% Ramificação de Descida
    M -- Descendo --> P{"Vai negativar?
    (delta > total)"}
    P -- Sim --> P1["Altura em 0"]
    P -- Não --> Q["total_pulsos -= delta"]
    P1 --> R
    Q --> R
    
    %% Caso motor parado, segue direto
    M -- Parado --> R

    %% Cálculos Matemáticos
    R["Calcula Posição
    e Velocidade"]

    %% Loop de Temperatura
    R --> T["Inicializa Loop Média
    soma_adc = 0
    i = 0"]
    
    T --> U{"i < 10 ?"}
    
    U -- Sim --> V["Lê ADC"]
    V --> v1["soma_adc += leitura"] 
    v1 -->   v2["Delay 100ms"]
    v2  --> W["i++"]
    W --> U
    
    U -- Não --> X["Calcula Média Final
    temp = soma_adc / 10"]
    
    X --> Y([Fim da Rotina])

    %% Estilização
    style M stroke:#00C853,fill:#B9F6CA,color:#000000
    style O stroke:#00C853,fill:#B9F6CA,color:#000000
    style P stroke:#00C853,fill:#B9F6CA,color:#000000
    style U stroke:#FFD600,fill:#FFF9C4,color:#000000
