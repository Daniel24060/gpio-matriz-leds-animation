# Projeto: Controle de Pinos GPIO com Teclado Matricial 4x4

Este projeto simula o controle de uma matriz de LEDs RGB 5x5 WS2812 utilizando o Raspberry Pi Pico W e um teclado matricial 4x4. As teclas do teclado acionam diferentes animações e controlam a matriz de LEDs de acordo com os comandos definidos. O projeto inclui funcionalidades adicionais, como controle de intensidade e cores dos LEDs, além de um modo de bloqueio e reinicialização via software.

## Vídeo da Prática

Segue o link do vídeo dos resultados obtidos durante a simulação:

[Assista ao vídeo demonstrativo. Clique aqui!]()



---

## Tabela de Conteúdos
  - [Estrutura do Repositório](#estrutura-do-repositório)
  - [Como Rodar o Projeto](#como-rodar-o-projeto)
  - [Simulação no Wokwi:](#simulação-no-wokwi)
  - [Delegação de Tarefas](#delegação-de-tarefas)
  - [Prazos e Datas Importantes](#prazos-e-datas-importantes)

## Estrutura do Repositório



## Como Rodar o Projeto

Para rodar o projeto localmente, siga os seguintes passos:

1. **Clone o repositório**:
   ```bash
   git clone https://github.com/Daniel24060/gpio-matriz-leds-animation.git
   ````
2. Entre na pasta do projeto:
    ````
    cd gpio-matriz-leds-animation
    ````

# Instruções para Configuração e Compilação do Projeto

## Passo 1: Baixar o Ninja
1. Acesse a página oficial de releases do Ninja:  
   [Ninja Releases no GitHub](https://github.com/ninja-build/ninja/releases).
2. Baixe o arquivo binário mais recente para Windows (geralmente um `.zip`).
3. Extraia o conteúdo do arquivo `.zip` (haverá um arquivo `ninja.exe`).

---

## Passo 2: Adicionar o Ninja ao Path do sistema
Para que o `ninja` seja reconhecido como um comando em qualquer terminal, siga estas etapas:

1. Clique com o botão direito no botão **Iniciar** e selecione **Configurações**.
2. Vá em **Sistema** → **Sobre** → **Configurações avançadas do sistema** (no lado direito).
3. Na aba **Avançado**, clique em **Variáveis de Ambiente**.
4. Na seção **Variáveis do Sistema**, localize a variável `Path` e clique em **Editar**.
5. Clique em **Novo** e adicione o caminho completo para o diretório onde você extraiu o `ninja.exe`. Por exemplo:
````
C:\Users\SeuUsuario\Downloads\ninja-win
````
6. Clique em **OK** em todas as janelas.

---

## Passo 3: Verificar se o Ninja está funcionando
1. Abra um terminal (PowerShell ou Prompt de Comando).
2. Digite o comando abaixo para verificar a instalação:
```bash
ninja --version
````

## Passo 4: Configurar e Compilar o Projeto

1. Volte ao diretório do projeto e abra um terminal.
2. Crie ou limpe a pasta `build`:
   ```bash
   rmdir /s /q build
   mkdir build
   cd build

3. Configure o projeto com o CMake:
 ```bash
   cmake -G Ninja ..
  ````
4. Compile o projeto:

  ```bash
   ninja
  ````

5. Após isso, o arquivo main.elf será gerado na pasta build.

## Simulação no Wokwi:

Após a configuração, você pode usar o simulador Wokwi para testar o circuito. Acesse o simulador através do link [https://wokwi.com/](https://wokwi.com/) ou use a extensão do Wokwi no VSCode [https://marketplace.visualstudio.com/items?itemName=Wokwi.wokwi-vscode](https://marketplace.visualstudio.com/items?itemName=Wokwi.wokwi-vscode) e veja como a interação com o teclado matricial aciona os LEDs, o buzzer e exibe as informações no display LCD1602.


## Delegação de Tarefas

Cada membro do projeto ficou responsável por uma parte específica. Abaixo estão as tarefas delegadas, com o nome do responsável e o nome sugerido para a branch.

| **Membro**           | **Tarefa**                                  | **Branch Sugerida**            |
|----------------------|---------------------------------------------|--------------------------------|
| **Matheus Mato**      |               | ` ` |
| **Vivian Rodrigues**  |                     | ` `      |
| **Daniel Silva**      |                     | ` `     |
| **Joelson Santana**    |              | ` `|
| **Mychael Matos**     |                | ` ` |conversor-potencia`   |
| **Caio Bruno**        |                      | ` `       |conversor-tempo`      | `      |
| **Adimael Santos**      |             | ` `   |
| **Tarefa Livre**      |               | ` `          |  |

## Prazos e Datas Importantes

- Data de início do projeto: 22/01/2025

- Data para finalizar e subir as tarefas: 25/01/2025 (como combinado durante a reunião)

- Data para revisão e merge das branches: 25/01/2025

- Data final do projeto: 26/01/2025




