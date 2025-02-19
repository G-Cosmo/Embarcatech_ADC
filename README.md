# Embarcatech_ADC

Repositório criado para a tarefa relacionada à utilização do conversor analógico digital e do joystick.


O programa implementa o controle de dois canais (azul e vermelho) de um LED RGB através de um PWM, cujo ciclo de trabalho é alterado utilizando os valores obtidos no ADC da leitura do joystick.
Além disso, o canal verde é controlado utilizando o botão embutido no joystick.
O programa também realiza o controle do display de LEDs SSD1306 de acordo com o movimento do joystick e o pressionamento do botão.
O outro botão presente no circuito faz com o controle do PWM seja ativado ou desativado.

# Instruções de compilação

Para compilar o código, são necessárias as seguintes extensões: 

*Wokwi Simulator*

*Raspberry Pi Pico*

*Cmake*

Após instalá-las basta buildar o projeto pelo CMake. A partir daí, abra o arquivo 
diagram.json e clique no botão verde para iniciar a simulação.

# Instruções de utilização

Enquanto na simulação, o usuário pode interagir com o programa utilizando o joystick e o push button presentes no circuito.

É importante ressaltar que o simulador pode apresentar um comportamento levemente incoerente no início da simulação. Para corrigir, basta clicar no botão de reiniciar a simulação depois de iniciá-la pela primeira vez e certificar-se de que o joystick está no centro.

Ao movimentar o joystick no eixo horizontal, a intensidade do LED vermelho variará de 0% até 100%. Enquanto ele estiver no centro, o LED permanecerá apagado. Da mesma forma, ao movimentar o joystick no eixo vertical, a intensidade do LED azul será igualmente ajustada.

O quadrado presente no display de LEDs também se movimenta conforme o joystick. No simulador, o movimento é invertido, ou seja, se o joystick for movido para a direita, o quadrado se moverá para a esquerda. Essa é uma propriedade do joystick do simulador.

Além disso, o botão do joystick faz com que o LED verde alterne seu estado e faz a borda do display ficar levemente mais espessa.

O push button apenas desabilita ou reabilita o controle dos canais azul e vermelho via interrupção, mas o LED verde continua podendo ser alternado.

# Vídeo demonstrativo

https://youtube.com/shorts/VzevYOjp9XQ?feature=share

https://www.youtube.com/watch?v=27e5EETN3q4 (melhorias)
