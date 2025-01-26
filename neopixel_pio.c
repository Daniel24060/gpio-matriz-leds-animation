#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Biblioteca gerada pelo arquivo .pio durante compilação.
#include "ws2818b.pio.h"

// Definição do número de LEDs e pino.
#define ROWS 4
#define COLS 4
#define LED_COUNT 25
#define LED_PIN 7


//declaração das funções
void animation_rabbit();
void animation_button_2();
void animation_button_3();
void animation_button_4();
void animation_button_5();
void animation_button_6();
void animation_button_A();
void animation_button_B();
void animation_button_C();
void animation_button_D();
void animation_button_hastag();


// Definição dos pinos do teclado matricial
const uint LINHAS[4] = {1, 2, 3, 4};       // GPIOs das linhas do teclado
const uint COLUNAS[4] = {10, 11, 12, 13};  // GPIOs das colunas do teclado

const char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

void init_teclado_btn0() {
    // Configura as linhas como saída e colunas como entrada
    for (int i = 0; i < ROWS; i++) {
        gpio_init(LINHAS[i]);
        gpio_set_dir(LINHAS[i], GPIO_OUT);
        gpio_put(LINHAS[i], true); // Estado padrão
    }
    for (int i = 0; i < COLS; i++) {
        gpio_init(COLUNAS[i]);
        gpio_set_dir(COLUNAS[i], GPIO_IN);
        gpio_pull_up(COLUNAS[i]); // Ativa pull-up
    }
}

char ler_teclado() {
    for (int row = 0; row < ROWS; row++) {
        gpio_put(LINHAS[row], false); // Ativa a linha

        for (int col = 0; col < COLS; col++) {
            if (!gpio_get(COLUNAS[col])) { // Coluna baixa
                gpio_put(LINHAS[row], true); // Restaura a linha
                return keys[row][col];
            }
        }

        gpio_put(LINHAS[row], true); // Restaura a linha
    }
    return '\0'; // Nenhuma tecla pressionada
}

// Função para configurar os pinos do teclado matricial
void setup_matrix_keyboard() {
    for (int i = 0; i < 4; i++) {
        gpio_init(LINHAS[i]);
        gpio_set_dir(LINHAS[i], GPIO_OUT);
        gpio_put(LINHAS[i], 1); // Inicializa linhas em nível alto
    }
    for (int i = 0; i < 4; i++) {
        gpio_init(COLUNAS[i]);
        gpio_set_dir(COLUNAS[i], GPIO_IN);
        gpio_pull_up(COLUNAS[i]); // Habilita pull-up nas colunas
    }
}

// Função para ler o teclado matricial
int read_matrix_keyboard() {
    for (int i = 0; i < 4; i++) {
        gpio_put(LINHAS[i], 0); // Coloca a linha atual em nível baixo
        for (int j = 0; j < 4; j++) {
            if (!gpio_get(COLUNAS[j])) { // Detecta tecla pressionada
                gpio_put(LINHAS[i], 1); // Restaura linha para nível alto
                return i * 4 + j + 1;   // Retorna o número da tecla
            }
        }
        gpio_put(LINHAS[i], 1); // Restaura linha para nível alto
    }
    return -1; // Nenhuma tecla pressionada
}

// Definição de pixel GRB
struct pixel_t {
  uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin) {

  // Cria programa PIO.
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;

  // Toma posse de uma máquina PIO.
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0) {
    np_pio = pio1;
    sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
  }

  // Inicia programa na máquina PIO obtida.
  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

  // Limpa buffer de pixels.
  for (uint i = 0; i < LED_COUNT; ++i) {
    leds[i].R = 0;
    leds[i].G = 0;
    leds[i].B = 0;
  }
}

/**
 * Atribui uma cor RGB a um LED.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
  leds[index].R = r;
  leds[index].G = g;
  leds[index].B = b;
}

/**
 * Limpa o buffer de pixels.
 */
void npClear() {
  for (uint i = 0; i < LED_COUNT; ++i)
    npSetLED(i, 0, 0, 0);
}

/**
 * Escreve os dados do buffer nos LEDs.
 */
void npWrite() {
  // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
  for (uint i = 0; i < LED_COUNT; ++i) {
    pio_sm_put_blocking(np_pio, sm, leds[i].G);
    pio_sm_put_blocking(np_pio, sm, leds[i].R);
    pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

// Modificado do github: https://github.com/BitDogLab/BitDogLab-C/tree/main/neopixel_pio
// Função para converter a posição do matriz para uma posição do vetor.
int getIndex(int x, int y) {
    // Se a linha for par (0, 2, 4), percorremos da esquerda para a direita.
    // Se a linha for ímpar (1, 3), percorremos da direita para a esquerda.
    if (y % 2 == 0) {
        return 24-(y * 5 + x); // Linha par (esquerda para direita).
    } else {
        return 24-(y * 5 + (4 - x)); // Linha ímpar (direita para esquerda).
    }
}

void ajustarLuminosidade(int matriz[5][5][3], float fator) {
    // Limitar o fator para estar entre 0 e 1
    if (fator < 0) fator = 0;
    if (fator > 1) fator = 1;

    // Ajustar cada valor da matriz
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            // Ajustar os valores de vermelho, verde e azul
            matriz[i][j][0] = (int)(matriz[i][j][0] * fator); // Vermelho
            matriz[i][j][1] = (int)(matriz[i][j][1] * fator); // Verde
            matriz[i][j][2] = (int)(matriz[i][j][2] * fator); // Azul
        }
    }

    // Atualizar os LEDs com os valores ajustados
    for (int linha = 0; linha < 5; linha++) {
        for (int coluna = 0; coluna < 5; coluna++) {
            int posicao = getIndex(linha, coluna);
            npSetLED(posicao, matriz[coluna][linha][0], matriz[coluna][linha][1], matriz[coluna][linha][2]);
        }
    }

    npWrite();
}


void animation_button_A(){
  npClear();
  npWrite();
};

void animation_button_B(){
  int matriz_azul[5][5][3] = {
  {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
  {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
  {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
  {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
  {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}}
  };

  ajustarLuminosidade(matriz_azul, 1);

};

void animation_button_C(){
  int matriz_vermelha[5][5][3] = {
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}}
  };

  ajustarLuminosidade(matriz_vermelha, 0.2);

}

void animation_button_D(){
  int matriz_verde[5][5][3] = {
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}},
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}},
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}},
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}},
    {{0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}}
  };

  ajustarLuminosidade(matriz_verde, 0.6);

}

void animation_button_hastag(){
  int matriz_branca[5][5][3] = {
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}}
  };

  ajustarLuminosidade(matriz_branca, 0.8);

}



void animation_rabbit(){
  int rosto_frente[5][5][3] = {
  {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}},
    {{255, 255, 255}, {0, 21, 255}, {255, 255, 255}, {0, 21, 255}, {255, 255, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}}
  };

   // Desenhando Sprite contido na matriz.c
  for(int linha = 0; linha < 5; linha++){
    for(int coluna = 0; coluna < 5; coluna++){
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, rosto_frente[coluna][linha][0], rosto_frente[coluna][linha][1], rosto_frente[coluna][linha][2]);
    }
  }

  npWrite();
  sleep_ms(1000);
  npClear();

  int rosto_esquerda[5][5][3] = {
    {{255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}, {0, 0, 0}},
    {{255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}, {0, 0, 0}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {0, 0, 0}},
    {{0, 21, 255}, {255, 255, 255}, {0, 21, 255}, {255, 255, 255}, {0, 0, 0}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {0, 0, 0}}
  };

   // Desenhando Sprite contido na matriz.c
  for(int linha = 0; linha < 5; linha++){
    for(int coluna = 0; coluna < 5; coluna++){
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, rosto_esquerda[coluna][linha][0], rosto_esquerda[coluna][linha][1], rosto_esquerda[coluna][linha][2]);
    }
  }

  npWrite();
  sleep_ms(1000);
  npClear();

  int rosto_frente_2[5][5][3] = {
  {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}},
    {{255, 255, 255}, {0, 21, 255}, {255, 255, 255}, {0, 21, 255}, {255, 255, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}}
  };

   // Desenhando Sprite contido na matriz.c
  for(int linha = 0; linha < 5; linha++){
    for(int coluna = 0; coluna < 5; coluna++){
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, rosto_frente_2[coluna][linha][0], rosto_frente_2[coluna][linha][1], rosto_frente_2[coluna][linha][2]);
    }
  }

  npWrite();
  sleep_ms(1000);
  npClear();

  int rosto_direita[5][5][3] = {
      {{0, 0, 0}, {215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}},
    {{0, 0, 0}, {215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}},
    {{0, 0, 0}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}},
    {{0, 0, 0}, {255, 255, 255}, {0, 21, 255}, {255, 255, 255}, {0, 21, 255}},
    {{0, 0, 0}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}}
  };

   // Desenhando Sprite contido na matriz.c
  for(int linha = 0; linha < 5; linha++){
    for(int coluna = 0; coluna < 5; coluna++){
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, rosto_direita[coluna][linha][0], rosto_direita[coluna][linha][1], rosto_direita[coluna][linha][2]);
    }
  }

  npWrite();
  sleep_ms(1000);
  npClear();

  int rosto_frente_3[5][5][3] = {
  {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}},
    {{255, 255, 255}, {0, 21, 255}, {255, 255, 255}, {0, 21, 255}, {255, 255, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}}
  };

   // Desenhando Sprite contido na matriz.c
  for(int linha = 0; linha < 5; linha++){
    for(int coluna = 0; coluna < 5; coluna++){
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, rosto_frente_3[coluna][linha][0], rosto_frente_3[coluna][linha][1], rosto_frente_3[coluna][linha][2]);
    }
  }

  npWrite();
  sleep_ms(1000);
  npClear();

  int coracao_1[5][5][3] = {
    {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}},
    {{255, 255, 255}, {0, 21, 255}, {255, 255, 255}, {0, 21, 255}, {255, 255, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 0, 0}, {255, 255, 255}, {255, 255, 255}}
  };

   // Desenhando Sprite contido na matriz.c
  for(int linha = 0; linha < 5; linha++){
    for(int coluna = 0; coluna < 5; coluna++){
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, coracao_1[coluna][linha][0], coracao_1[coluna][linha][1], coracao_1[coluna][linha][2]);
    }
  }

  npWrite();
  sleep_ms(1000);
  npClear();

  int coracao_2[5][5][3] = {
    {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}},
    {{255, 255, 255}, {255, 0, 0}, {255, 255, 255}, {255, 0, 0}, {255, 255, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 0, 0}, {255, 255, 255}, {255, 255, 255}}
  };

   // Desenhando Sprite contido na matriz.c
  for(int linha = 0; linha < 5; linha++){
    for(int coluna = 0; coluna < 5; coluna++){
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, coracao_2[coluna][linha][0], coracao_2[coluna][linha][1], coracao_2[coluna][linha][2]);
    }
  }

  npWrite();
  sleep_ms(1000);
  npClear();

  int coracao_3[5][5][3] = {
     {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{215, 0, 255}, {255, 0, 0}, {0, 0, 0}, {255, 0, 0}, {215, 0, 255}},
    {{255, 0, 0}, {255, 255, 255}, {255, 255, 255}, {255, 255, 255}, {255, 0, 0}},
    {{255, 255, 255}, {255, 0, 0}, {255, 255, 255}, {255, 0, 0}, {255, 255, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 0, 0}, {255, 255, 255}, {255, 255, 255}}
  };

   // Desenhando Sprite contido na matriz.c
  for(int linha = 0; linha < 5; linha++){
    for(int coluna = 0; coluna < 5; coluna++){
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, coracao_3[coluna][linha][0], coracao_3[coluna][linha][1], coracao_3[coluna][linha][2]);
    }
  }

  npWrite();
  sleep_ms(1000);
  npClear();

  int coracao_4[5][5][3] = {
    {{215, 0, 255}, {255, 255, 255}, {0, 0, 0}, {255, 255, 255}, {215, 0, 255}},
    {{215, 0, 255}, {255, 0, 0}, {0, 0, 0}, {255, 0, 0}, {215, 0, 255}},
    {{255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}},
    {{255, 255, 255}, {255, 0, 0}, {255, 0, 0}, {255, 0, 0}, {255, 255, 255}},
    {{255, 255, 255}, {255, 255, 255}, {255, 0, 0}, {255, 255, 255}, {255, 255, 255}}
  };

   // Desenhando Sprite contido na matriz.c
  for(int linha = 0; linha < 5; linha++){
    for(int coluna = 0; coluna < 5; coluna++){
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, coracao_4[coluna][linha][0], coracao_4[coluna][linha][1], coracao_4[coluna][linha][2]);
    }
  }

  npWrite();
  sleep_ms(1000);
  npClear();  

}
void animation_monster() {
    while(true){
    // Matriz 1
    int matriz1[5][5][3] = {
        {{0, 0, 0}, {49, 49, 199}, {49, 49, 199}, {49, 49, 199}, {0, 0, 0}},
        {{49, 49, 199}, {41, 197, 94}, {49, 49, 199}, {41, 197, 94}, {49, 49, 199}},
        {{49, 49, 199}, {49, 49, 199}, {49, 49, 199}, {49, 49, 199}, {49, 49, 199}},
        {{0, 0, 0}, {49, 49, 199}, {0, 0, 0}, {49, 49, 199}, {0, 0, 0}},
        {{49, 49, 199}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {49, 49, 199}}};

    // Matriz 2
    int matriz2[5][5][3] = {
        {{199, 49, 49}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {199, 49, 49}},
        {{0, 0, 0}, {199, 49, 49}, {199, 49, 49}, {199, 49, 49}, {0, 0, 0}},
        {{199, 49, 49}, {94, 197, 41}, {199, 49, 49}, {94, 197, 41}, {199, 49, 49}},
        {{199, 49, 49}, {199, 49, 49}, {199, 49, 49}, {199, 49, 49}, {199, 49, 49}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}};

    // Matriz 3
    int matriz3[5][5][3] = {
        {{0, 0, 0}, {31, 144, 227}, {0, 0, 0}, {31, 144, 227}, {0, 0, 0}},
        {{31, 144, 227}, {182, 48, 48}, {31, 144, 227}, {182, 48, 48}, {31, 144, 227}},
        {{31, 144, 227}, {31, 144, 227}, {31, 144, 227}, {31, 144, 227}, {31, 144, 227}},
        {{31, 144, 227}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {31, 144, 227}},
        {{0, 0, 0}, {31, 144, 227}, {0, 0, 0}, {31, 144, 227}, {0, 0, 0}}};

    // Matriz 4
    int matriz4[5][5][3] = {
        {{0, 0, 0}, {181, 48, 118}, {181, 48, 118}, {181, 48, 118}, {0, 0, 0}},
        {{181, 48, 118}, {31, 227, 144}, {181, 48, 118}, {31, 227, 144}, {181, 48, 118}},
        {{181, 48, 118}, {181, 48, 118}, {181, 48, 118}, {181, 48, 118}, {181, 48, 118}},
        {{0, 0, 0}, {181, 48, 118}, {0, 0, 0}, {181, 48, 118}, {0, 0, 0}},
        {{181, 48, 118}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {181, 48, 118}}};

    // Matriz 5
    int matriz5[5][5][3] = {
        {{48, 118, 181}, {0, 0, 0}, {48, 118, 181}, {0, 0, 0}, {48, 118, 181}},
        {{0, 0, 0}, {48, 118, 181}, {0, 0, 0}, {48, 118, 181}, {0, 0, 0}},
        {{48, 118, 181}, {48, 118, 181}, {48, 118, 181}, {48, 118, 181}, {48, 118, 181}},
        {{0, 0, 0}, {48, 118, 181}, {0, 0, 0}, {48, 118, 181}, {0, 0, 0}},
        {{48, 118, 181}, {0, 0, 0}, {48, 118, 181}, {0, 0, 0}, {48, 118, 181}}};

    // Array de ponteiros para matrizes
    int (*matrizes[5])[5][3] = {matriz1, matriz2, matriz3, matriz4, matriz5};

    // Loop para exibir cada matriz
    for (int m = 0; m < 5; m++) {
        for (int linha = 0; linha < 5; linha++) {
            for (int coluna = 0; coluna < 5; coluna++) {
                int posicao = getIndex(linha, coluna);
                npSetLED(posicao, matrizes[m][linha][coluna][0], matrizes[m][linha][coluna][1], matrizes[m][linha][coluna][2]);
            }
        }
        npWrite();
        sleep_ms(1000);
        npClear();
    }
}}

void displayFrame(int frame[5][5][3])
{
  for (int linha = 0; linha < 5; linha++)
  {
    for (int coluna = 0; coluna < 5; coluna++)
    {
      int posicao = getIndex(linha, coluna);
      npSetLED(posicao, frame[coluna][linha][0], frame[coluna][linha][1], frame[coluna][linha][2]);
    }
  }
  npWrite();
  sleep_ms(1000);
  npClear();
}

void animation_snake(){
  int snake[15][5][5][3] = {
      // Frame 1
      {{{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {255, 0, 0}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 255, 0}, {0, 255, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}}},
      // Frame 2
      {{{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {255, 0, 0}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 255, 0}, {0, 255, 0}, {0, 0, 255}, {0, 0, 255}}},
      // frame 3
      {{{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {255, 0, 0}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 255, 0}, {0, 255, 0}, {0, 0, 255}}},
      // Frame 4
      {{{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {255, 0, 0}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 255, 0}, {0, 255, 0}}},
      // Frame 5
      {{{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {255, 0, 0}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 255, 0}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 255, 0}}},
      // Frame 6
      {{{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {255, 0, 0}, {0, 0, 255}, {0, 255, 0}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 255, 0}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}}},
      // Frame 7
      {{{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {255, 0, 0}, {0, 255, 0}, {0, 255, 0}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}}},
      // Frame 8
      {{{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}}},
      // Frame 9
      {{{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 255, 0}, {0, 255, 0}, {0, 255, 0}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}}},
      // Frame 10
      {{{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 255, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 255, 0}, {0, 255, 0}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}}},
      // Frame 11
      {{{0, 0, 255}, {0, 255, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 255, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 255, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}}},
      // Frame 12
      {{{0, 0, 255}, {0, 255, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 255, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}}},
      // Frame 13
      {{{0, 0, 255}, {0, 255, 0}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}}},
      // Frame 14
      {{{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}},
       {{0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}, {0, 0, 255}}}};

  for (int i = 0; i < 14; i++){
    displayFrame(snake[i]);
  }
}


int main() {

  // Inicializa entradas e saídas.
  stdio_init_all();

  // Configuração do teclado matricial
    setup_matrix_keyboard();

  // Inicializa matriz de LEDs NeoPixel.
  npInit(LED_PIN);
  npClear();

  // Aqui, você desenha nos LEDs.

  npWrite(); // Escreve os dados nos LEDs.

  // Não faz mais nada. Loop infinito.
  while (true) {
    
    char tecla = ler_teclado();
        switch (tecla)
        {
        case '1':
            animation_rabbit();
            break;

        case '2':
            animation_monster();
            break;

        case '3':
            animation_snake();
            break;

        case '4':
            
            break;

        case '5':
            
            break;

        case '6':

            break;

        case '7':
            
            break;

        case '8':
            
            break;

        case '9':
            
            break;

        case '0':
            break;
        
        case 'A':
            animation_button_A();
            break;

        case 'B':
            animation_button_B();
            break;

        case 'C':
            animation_button_C();
            break;

        case 'D':
            animation_button_D();
            break;

        case '*':
            break;

        case '#':
            animation_button_hastag();
            break;    

        default:
            break;
        }
        sleep_ms(100); // Pequena pausa para evitar múltiplas leituras da mesma tecla
  }
}

