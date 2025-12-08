# 🚧 Controle de Elevador 4 Andares por Bluetooth

Projeto acadêmico desenvolvido para a disciplina **Eletrônica Embarcada (FGA0096)** da Universidade de Brasília.  
O sistema simula o funcionamento de um elevador real utilizando microcontrolador PIC, controle de motor DC, sensores, comunicação Bluetooth e interface visual com matriz de LEDs.

---

## 📌 Objetivo

Integrar os conteúdos aprendidos na disciplina por meio da criação de um sistema embarcado funcional.  
O elevador deve atender solicitações de andares, otimizar seu percurso e apresentar informações ao usuário via Bluetooth e display LED.

---

## 🧠 Funcionalidades Principais

- Controle do motor DC por **PWM (10 bits)** + **direção digital**.
- Gerenciamento de movimento com **ponte H TB6612FNG**.
- **Sensores magnéticos Hall (S1–S4)** para detectar os andares.
- **Encoder óptico** para medir posição (mm) e velocidade (mm/s).
- Monitoramento da **temperatura da ponte H** com sensor LM35 (ADC 10 bits).
- Interface visual com **matriz de LEDs MAX7219 via SPI (1 MHz)**.
- Comunicação Bluetooth via **HC-06 UART (ASCII 19200bps)**.
- Sistema de atendimento de **até 5 solicitações simultâneas** (origem → destino).

---

## 🔌 Hardware Utilizado

- **PIC16F1827 (5V)**
- **Motor CC com engrenagem**
- **Ponte H TB6612FNG**
- **Sensores de efeito Hall A3144**
- **Encoder óptico + disco**
- **Matriz LED 8x8 com MAX7219**
- **Sensor LM35**
- **Bluetooth HC-06**

---

## 📡 Comunicação Bluetooth

O microcontrolador transmite um pacote a cada **300 ms** no formato ASCII:

Exemplo:
**Campos:**
- **A** → último andar atingido (0–3)
- **D** → destino solicitado
- **M** → estado do motor  
  - 0 = parado  
  - 1 = subindo  
  - 2 = descendo
- **HHH** → posição (0–180 mm)
- **VV.V** → velocidade (mm/s, 1 casa decimal)
- **TT.T** → temperatura da ponte H (°C)


## ▶️ Algoritmo de Atendimento

- São processadas **até 5 solicitações**.
- O elevador atende **todas as paradas no mesmo sentido** antes de inverter.
- Cada chamada possui **origem e destino**, que devem ser respeitados.
- Quando não há solicitações, o sistema retorna para **andar 0 (repouso)**.
- Proteções:
  - **2s** de parada para embarque/desembarque
  - **500ms** antes de inverter a direção

---

## 🖥️ Interface na Matriz de LEDs (MAX7219)

- Colunas 1–4: último andar atingido.
- Colunas superiores 5–7: seta indicando **direção do movimento**.
- Colunas inferiores 5–7: pontos representando os **andares pendentes**.

Essa visualização facilita o monitoramento do sistema em tempo real.

```mermaid
flowchart TB
    A(["Inicializa configurações
    basicas"]) --> 
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
    são validos?"}
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
    delta = atual - ultimo
    Atualiza ultimo_valor"]
    
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
