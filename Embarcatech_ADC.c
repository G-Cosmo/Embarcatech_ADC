#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "inc/ssd1306.h"


#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define address 0x3C
#define VRX 27
#define VRY 26
#define buttonJ 22
#define buttonA 5
#define sqrCenterX (WIDTH/2)-4
#define sqrCenterY (HEIGHT/2)-4
#define DEADZONE 100


uint led_rgb[3] = {13,11,12};
uint x_center = 2047;   //centro padrão do joystick
uint y_center = 2047;   //centro padrão do joystick
float dcX = 0;   //percentual do ciclo de trabalho de x (led vermelho)
float dcY = 0;   //percentual do ciclo de trabalho de y (led azul)
float sqrX = 0;  //percentual de movimentação no eixo X do quadrado
float sqrY = 0;  //percentual de movimentação no eixo y do quadrado
float moveFactor = 1.5; //fator divisivo de movimentação do quadrado, quanto maior for o valor, menor será a movimentação do quadrado
uint wrapX = 0;  //wrap do pwm que controla o led vermelho
uint wrapY = 0;  //wrap do pwm que controla o led azul
uint64_t volatile last_time = 0;    //variavel que indica o tempo da ultima demição
uint64_t volatile current_time = 0; //variavel que indica o tempo da atual medição
bool led_flag = true;   //flag que habilita o controle dos leds via pwm
bool edge_flag = false; //flag que habilita a mudaça de borda
bool color = true;  //variavel que indica que se o pixel está ligado ou desligado
ssd1306_t ssd; //inicializa a estrutura do display


uint init_pwm(uint gpio, uint wrap) {
    gpio_set_function(gpio, GPIO_FUNC_PWM);

    uint slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_wrap(slice_num, wrap);
    
    pwm_set_enabled(slice_num, true);  
    return slice_num;  
}

void init_rgb(uint *led)
{
    gpio_init(led[0]);
    gpio_init(led[1]);
    gpio_init(led[2]);

    gpio_set_dir(led[0], GPIO_OUT);
    gpio_set_dir(led[1], GPIO_OUT);
    gpio_set_dir(led[2], GPIO_OUT);

    gpio_put(led[0], false);
    gpio_put(led[1], false);
    gpio_put(led[2], false);
}

void init_buttons()
{
    gpio_init(buttonJ);
    gpio_init(buttonA);

    gpio_set_dir(buttonJ, GPIO_IN);
    gpio_set_dir(buttonA, GPIO_IN);

    gpio_pull_up(buttonJ);
    gpio_pull_up(buttonA);
}

void init_display()
{
    i2c_init(I2C_PORT, 400*1000);
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  //seta o pino gpio como i2c
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);  //seta o pino gpio como i2c
    gpio_pull_up(I2C_SDA);  //ativa o resistor de pull up para gantir o nível lógico alto
    gpio_pull_up(I2C_SCL);  //ativa o resistor de pull up para gantir o nível lógico alto


    ssd1306_init(&ssd, WIDTH, HEIGHT, false, address, I2C_PORT); //inicializa o display
    ssd1306_config(&ssd); //configura o display
    ssd1306_send_data(&ssd); //envia os dados para o display
  
    //limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

void gpio_irq_handler(uint gpio, uint32_t events)
{
    current_time = to_ms_since_boot(get_absolute_time());

    if((current_time - last_time > 200))//implement ao deboucing
    {
        last_time =  current_time;
        
        led_flag = !led_flag;//atualiza a flag que permite ou não o controle dos leds vermelho e azul

        pwm_set_gpio_level(led_rgb[0], 0);
        pwm_set_gpio_level(led_rgb[2], 0);
    }
}

int main()
{
    stdio_init_all();

    init_rgb(led_rgb);//inicializa o led rgb

    init_buttons();//inicializa os botões

    init_display();//inicializa o display
    
    adc_init();//inicializa o adc
    adc_gpio_init(VRX);//inicializa o pino X do adc
    adc_gpio_init(VRY);//inicializa o pino Y do adc

    //bloco com leitura inicial do adc para definir onde é o centro do joystick
    sleep_ms(100); 
    adc_select_input(1);
    uint16_t vrx_value = adc_read();
    x_center = vrx_value;
    wrapX = vrx_value;
    printf("\nFirst Values vrx, x_center, wrap: %d, %d, %d", vrx_value, x_center, wrapX);

    adc_select_input(0);
    uint16_t vry_value = adc_read();
    y_center = vry_value;
    wrapY = vry_value;
    printf("\nFirst Values vry, y_center, wrap: %d, %d, %d", vry_value, y_center, wrapY);
    sleep_ms(100);
    //fim da leitura inicial

    //configura o pwm com base na leitura do centro
    uint pwm_red = init_pwm(led_rgb[0],wrapX);
    uint pwm_blue = init_pwm(led_rgb[2],wrapY);

    //ativa a interrupção do botão A
    gpio_set_irq_enabled_with_callback(buttonA, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);   

    //limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    while (true) {
        adc_select_input(1);//seleciona a entrada do adc para x
        uint16_t vrx_value = adc_read();//realiza a leitura do valor de x
        printf("\n X value: %d", vrx_value);

        adc_select_input(0);//seleciona a entrada do adc para y
        uint16_t vry_value = adc_read();//realiza a leitura do valor de y
        printf("\n Y value: %d", vry_value);

        if(abs((int)vrx_value - (int)x_center) < DEADZONE)  //verifica se o eixo x está dentro da zona morta
        {
            dcX = 0;    //mantém o led desligado caso esteja
            sqrX = sqrCenterX;  //mantém o quadrado no centro caso esteja
        }else if(vrx_value < x_center)//verifica se o valor lido no adc foi menor do que o centro do joystick
        {
            vrx_value = x_center + (x_center - vrx_value);//ajusta o valor lido do adc para trabalhar com valores maiores do que o centro
            dcX = ((float)(vrx_value - x_center) / (float)wrapX);//calcula o percentual do ciclo de trabalho do eixo x
            printf("\ndcX: %f", dcX);

            sqrX = (1-(dcX/moveFactor))*sqrCenterX; //calcula o percentual de movimentação do quadrado no eixo x
        }else
        {
            dcX = ((float)(vrx_value - x_center) / (float)wrapX);//calcula o percentual do ciclo de trabalho do eixo x
            printf("\ndcX: %f", dcX);

            sqrX = (1+(dcX/moveFactor))*sqrCenterX; //calcula o percentual de movimentação do quadrado no eixo x
        }

        if(abs((int)vry_value - (int)y_center) < DEADZONE)  //verifica se o eixo y está dentro da zona morta
        {
            dcY = 0;    //mantém o led desligado caso esteja
            sqrY = sqrCenterY;  //mantém o quadrado no centro caso esteja
        }else if(vry_value < y_center)//verifica se o valor lido no adc foi menor do que o centro do joystick
        {
            vry_value = y_center + (y_center - vry_value);//ajusta o valor lido do adc para trabalhar com valores maiores do que o centro
            dcY = ((float)(vry_value - y_center) / (float)wrapY);//calcula o percentual do ciclo de trabalho do eixo y
            printf("\ndcy: %f", dcY);

            sqrY = (1+(dcY/moveFactor))*sqrCenterY; //calcula o percentual de movimentação do quadrado no eixo y
        }else
        {
            dcY = ((float)(vry_value - y_center) / (float)wrapY);    //calcula o percentual do ciclo de trabalho do eixo y
            printf("\ndcy: %f", dcY);

            sqrY = (1-(dcY/moveFactor))*sqrCenterY; //calcula o percentual de movimentação do quadrado no eixo y
        }
    
        ssd1306_fill(&ssd, !color); //limpa o display
        ssd1306_rect(&ssd, sqrY, sqrX, 8, 8, color, color); //desenha o quadrado 8x8 
        ssd1306_rect(&ssd, 3, 3, 122, 58, color, !color); //desenha um retângulo nas bordas do display
        if(edge_flag) ssd1306_rect(&ssd, 2, 2, 124, 60, color, !color); //desenha um retângulo mais largo nas bordas do display
        ssd1306_send_data(&ssd);    //atualiza o display

        if(led_flag) pwm_set_gpio_level(led_rgb[0], (uint)(dcX*wrapX));//atualiza o ciclo de trabalho do eixo x
        if(led_flag) pwm_set_gpio_level(led_rgb[2], (uint)(dcY*wrapY));//atualiza o ciclo de trabalho do eixo y

        current_time = to_ms_since_boot(get_absolute_time());   //pega o tempo atual

        if(!gpio_get(buttonJ) && (current_time - last_time > 200))//implementa o debouncing
        {
            last_time = current_time;
            gpio_put(led_rgb[1], !gpio_get(led_rgb[1]));    //alterna o estado do led verde

            edge_flag = !edge_flag; //alterna o estado da flag para ligar a borda mais espessa
        }

       sleep_ms(50);
    }
}
