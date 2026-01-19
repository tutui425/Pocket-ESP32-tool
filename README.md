Pocket ESP32 Tool v1.5 – Final Real

Pocket ESP32 Tool é um projeto open-source baseado em ESP32 que reúne diversas ferramentas úteis em um único dispositivo portátil com display OLED, botões físicos, RTC, buzzer, LEDs de status e alimentação por bateria.

O objetivo do projeto é servir como um canivete suíço portátil para ESP32, combinando relógio, utilitários, jogos e monitoramento do sistema em um único firmware.

---

🔧 Hardware Utilizado

- ESP32 Dev Module / ESP32 Dev Board
- Display OLED SSD1306 0.96” 128x64 (I2C)
- RTC DS3231 (I2C)
- 6 botões (push buttons)
- 1 buzzer ou alto-falante ativo 3.3V
- 3 LEDs
- 3 resistores para LEDs (220Ω a 1kΩ)
- 2 resistores de 100kΩ (divisor de tensão da bateria)
- Bateria Li-Ion / LiPo 3.7V (até 4.2V carregada)
- Protoboard ou PCB
- Jumpers / fios

---

📦 Recursos do Firmware

- Menu com destaque invertido
- Barra superior com status
- Relógio e data (RTC DS3231)
- Calendário simples
- Temperatura do RTC
- Cronômetro
- Timer regressivo
- Monitoramento de bateria (V / % / horas estimadas)
- Informações do sistema (heap, CPU, WiFi)
- Jogo Snake completo
- Configurações de brilho e som (salvas na EEPROM)
- OTA via WiFi
- Economia de energia via desligamento do display
- LEDs com significado por estado

---

🔌 Ligações Elétricas (Explicado para Iniciantes)

Display OLED SSD1306 (I2C)

OLED VCC  → 3V3 do ESP32
OLED GND  → GND do ESP32
OLED SDA  → GPIO 21
OLED SCL  → GPIO 22

RTC DS3231 (I2C – compartilha o barramento com o OLED)

DS3231 VCC → 3V3
DS3231 GND → GND
DS3231 SDA → GPIO 21
DS3231 SCL → GPIO 22

«É normal o OLED e o RTC usarem os mesmos pinos SDA e SCL.»

---

Botões (usando INPUT_PULLUP interno)

Cada botão é ligado entre o GPIO e o GND.

BTN_UP     → GPIO 32
BTN_DOWN   → GPIO 33
BTN_LEFT   → GPIO 25
BTN_RIGHT  → GPIO 26
BTN_OK     → GPIO 27
BTN_BACK   → GPIO 14

---

LEDs (sempre usar resistor em série)

Ligação padrão:
GPIO → Resistor → LED → GND

LED_STATUS → GPIO 2
LED1       → GPIO 4
LED2       → GPIO 5

Significado dos LEDs

- LED_STATUS (GPIO 2): pisca continuamente indicando que o sistema está ligado
- LED1 (GPIO 4): aceso quando o modo economia de energia está ativo
- LED2 (GPIO 5): aceso quando a bateria está abaixo de 20%

---

Buzzer / Alto-falante

Buzzer positivo → GPIO 13
Buzzer negativo → GND

«Recomenda-se buzzer ativo para melhor som.»

---

🔋 Medição da Bateria (Divisor de Tensão 100kΩ + 100kΩ)

O ESP32 aceita no máximo 3.3V em suas entradas analógicas.
Como a bateria pode chegar a 4.2V, é necessário um divisor de tensão.

Ligação do divisor:

BATERIA + ----[100kΩ]----+----[100kΩ]---- GND
                         |
                      GPIO 34

O ponto central do divisor é conectado ao GPIO 34, que é apenas entrada e ideal para ADC.

---

Alimentação

- Bateria positiva → VIN (ou 5V, se a placa tiver regulador)
- Bateria negativa → GND
- Nunca ligar a bateria diretamente no pino 3V3

---

📌 Resumo de Pinos

OLED / RTC SDA  -> GPIO 21
OLED / RTC SCL  -> GPIO 22

BTN_UP          -> GPIO 32
BTN_DOWN        -> GPIO 33
BTN_LEFT        -> GPIO 25
BTN_RIGHT       -> GPIO 26
BTN_OK          -> GPIO 27
BTN_BACK        -> GPIO 14

BUZZER          -> GPIO 13

LED_STATUS      -> GPIO 2
LED1            -> GPIO 4
LED2            -> GPIO 5

BATERIA ADC     -> GPIO 34

---

📜 Licença

Este projeto é licenciado sob a MIT License.

Isso significa que você pode:

- Usar
- Copiar
- Modificar
- Distribuir
- Usar comercialmente

Desde que mantenha os créditos do autor.

---

👤 Autor

Tutui425
Pocket ESP32 Tool
Ano: 2026

---

Projeto em desenvolvimento ativo. Sugestões e melhorias são bem-vindas.
