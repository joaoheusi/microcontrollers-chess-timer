/*
 * File:   xadrez_main.c
 * Authors: joaoheusi
 *
 * Created on 3 de Maio de 2018, 08:41
 */
#include <xc.h>
#include <string.h>
#include <stdio.h>
#pragma config WDT=OFF
#define _XTAL_FREQ 16000000

//Flags para debounce
#define INT_0_FLAG 0x01L
#define INT_1_FLAG 0x02L
//Flag principal do debounce
volatile long mainEventFlag = 0;

//LCD
#define LCD_D4          PORTDbits.RD0
#define LCD_D5          PORTDbits.RD1
#define LCD_D6          PORTDbits.RD2
#define LCD_D7          PORTDbits.RD3
#define LCD_EN          PORTDbits.RD4
#define LCD_RS          PORTDbits.RD5

//Controle do LCD
#define LINHA_1 0
#define LINHA_2 1



//Vari?veis de controle do programa
bit player = 0, inicia = 0, x = 0, tipo = 0,configura = 0;

//Tempos
int seg1 = 0,seg2=0;
int min1 = 0,min2=0,minConfigura=1;
char mili1 = 10, mili2 = 10;

//Vari?veis de controle para EEPROM
unsigned char eeprom_address = 0xFF, write_char, read_char;
long int count = 0; // Contador para o delay do LCD
void passa_jogada(void);

//Fun??es para o LCD
void LCD_Init(void);
void LCD_SetPosition(unsigned int c);
void LCD_PutCmd(unsigned int c);
void LCD_PulseEnable(void);
void delay(void);
void upper(unsigned int c);
void lower(unsigned int c);
void LCD_PutChar(unsigned int c);
void escreve_tela(char mensagem[],char linha);
void tela_placar(void);
void atualiza_tempo(void);
void tela_configura(void);
void bipa(void);

void interrupt interrupcoes(void) {
    if (INTCONbits.TMR0IF) { //interrup??o pelo timer
        INTCONbits.TMR0IF = 0;
        if (player) {
            TMR0 = 15536;
			mili2--;
			if(mili2 == 0){
            seg2--;
			mili2 = 10;
			}
            if(min2 ==0 && seg2 == 0){
                T0CONbits.TMR0ON = 0;//desabilita timer
                atualiza_tempo();
                __delay_ms(1000);
                LCD_PutCmd(0x01);
                escreve_tela("Vencedor",LINHA_1);
                escreve_tela("Jogador 1",LINHA_2);  
				bipa();
				Reset();
            }else{
            if (seg2 == -1) {
                seg2 = 59;
                    min2--;
                }
                atualiza_tempo();
            }
        }
        if (!player) {
            TMR0 = 15536;
			mili1--;
			if(mili1 == 0){
				seg1--;
				mili1 = 10;
			}
			if(min1 ==0 && seg1 == 0){
                T0CONbits.TMR0ON = 0;//desabilita timer
                atualiza_tempo();
                __delay_ms(1000);
                LCD_PutCmd(0x01);
                escreve_tela("Vencedor",LINHA_1);
                escreve_tela("Jogador 2",LINHA_2);  
				bipa();
				Reset();
            }else{
			if(seg1 == -1) {
                seg1 = 59;
                min1--;
            }
            atualiza_tempo();
        }
	}
    }
    if (INTCONbits.INT0F) { //interrup??o pelo RB0 (bot?o configura)
        mainEventFlag |= INT_0_FLAG;
        INTCONbits.INT0F = 0;
    }
    if (INTCON3bits.INT1F) { //interrup??o RB1 (iniciar jogo e passar jogada)
        //escreve_tela("chegou no tempo",LINHA_1);
        mainEventFlag |= INT_1_FLAG;
        INTCON3bits.INT1F = 0;
    }
}

void bipa(void){
	PORTBbits.RB7 = 1;
	__delay_ms(500);
	PORTBbits.RB7 = 0;
	__delay_ms(500);
	PORTBbits.RB7 = 1;
	__delay_ms(500);
	PORTBbits.RB7 = 0;
	__delay_ms(500);
	PORTBbits.RB7 = 1;
	__delay_ms(500);
	PORTBbits.RB7 = 0;
	__delay_ms(500);
	PORTBbits.RB7 = 1;
	__delay_ms(500);
	PORTBbits.RB7 = 0;

}
void LCD_Init(void) {
    PORTD = 0x00;
    TRISD = 0x00;
    delay(); 
    delay();
    delay();
    delay();
    LCD_RS = 0;
    PORTD = 0x03;
    LCD_PulseEnable();
    delay();
    LCD_PulseEnable();
    delay();
    LCD_PulseEnable();
    PORTD = 0x02; /* set 4-bit interface */
    LCD_PulseEnable();
    LCD_PutCmd(0x2C); /* function set (all lines, 5x7 characters) */
    LCD_PutCmd(0x0C); /* display ON, cursor off, no blink */
    LCD_PutCmd(0x01); /* clear display */
    LCD_PutCmd(0x06); /* entry mode set, increment & scroll left */
}

void LCD_SetPosition(unsigned int c) {
    upper(c | 0x08);
    LCD_PulseEnable();
    lower(c);
    LCD_PulseEnable();
}

void LCD_PutCmd(unsigned int c) {
    upper(c); /* send high nibble */
    LCD_PulseEnable();
    lower(c); /* send low nibble */
    LCD_PulseEnable();
}

void LCD_PulseEnable(void) {
    LCD_EN = 1;
    delay(); // was 10
    LCD_EN = 0;
    delay(); // was 5
}

void delay(void) {
    for (count = 1; count < 500; count++){
    }
}

void upper(unsigned int c) {
    if (c & 0x80) LCD_D7 = 1;
    else LCD_D7 = 0;
    if (c & 0x40) LCD_D6 = 1;
    else
        LCD_D6 = 0;
    if (c & 0x20) LCD_D5 = 1;
    else LCD_D5 = 0;
    if (c & 0x10) LCD_D4 = 1;
    else LCD_D4 = 0;
}

void lower(unsigned int c) {
    if (c & 0x08) LCD_D7 = 1;
    else LCD_D7 = 0;
    if (c & 0x04) LCD_D6 = 1;
    else LCD_D6 = 0;
    if (c & 0x02) LCD_D5 = 1;
    else LCD_D5 = 0;
    if (c & 0x01) LCD_D4 = 1;
    else LCD_D4 = 0;
}

void LCD_PutChar(unsigned int c) {
    /* this subroutine works specifically for 4
    -
    bit Port A */
    LCD_RS = 1;
    upper(c); /* send high nibble */
    LCD_PulseEnable();
    lower(c); /* send low nibble */
    LCD_PulseEnable();
    LCD_RS = 0;
}

void passa_jogada(void) {
    player = ~player;
}

void escreve_tela(char mensagem[],char linha){
    //LCD_PutCmd(0x01); limpa tela
    char i = 0;
    if(linha){
        LCD_PutCmd(0xC0);
    }
    for(i = 0;i<strlen(mensagem);i++){
        LCD_PutChar(mensagem[i]);
    }
}

void atualiza_tempo(void){	
	char aux[16]={0};
    char aux1[16]={0};
    char aux2[16]={0};

    if(!player){
		//valida?ao jogador 01
		if(min1<10){
			sprintf(aux1, "0%d", min1);
		}else{
			sprintf(aux1, "%d", min1);
		}
		if(seg1<10){
			sprintf(aux2, "0%d", seg1);
		}else{
			sprintf(aux2, "%d", seg1);
		}
    }else{
		if(min2<10){
			sprintf(aux1, "0%d", min2);
		}else{
			sprintf(aux1, "%d", min2);
		}
		if(seg2<10){
			sprintf(aux2, "0%d", seg2);
		}else{
			sprintf(aux2, "%d", seg2);
		}
	}	
		strcat(aux,aux1);
		strcat(aux,":");
		strcat(aux,aux2);
	
		LCD_PutCmd(0x02); //seta cursor no inicio do display (0)
	
		if(player){
			LCD_PutCmd(0xC0); //escreve na segunda linha do display (jogador 02)
		}
		// sen?o escreve na primeira linha (jogador 01)
		
		for(int i = 0;i<5;i++){//atualiza apenas 5 posi?oes do display, para n?o ficar em efeito de escrita em onda
			LCD_PutChar(aux[i]);
		}	
}

void tela_configura(void){
	char mensagem[9] = "CONFIGURA";
	char aux1[2];	
	LCD_PutCmd(0x01); // limpa display
	for(int i=0;i<9;i++){
		LCD_PutChar(mensagem[i]);	}
		
	LCD_PutCmd(0xC0); //escreve segunda linha
	if(minConfigura<10)sprintf(aux1,"0%d",minConfigura);
	sprintf(aux1,"%d",minConfigura);
	
	if(minConfigura>=10){
		for(int i=0;i<2;i++){ // sera possivel configurar apenas os minutos e centraliza na tela o minuto piscando
			LCD_PutChar(aux1[i]);
		}
	}else{
		for(int i=0;i<1;i++){ // sera possivel configurar apenas os minutos e centraliza na tela o minuto piscando
			LCD_PutChar("O");
			LCD_PutChar(aux1[i]);
		}
	}
	
	
	
}

void inicia_registradores(void) {
    ADCON1 = 0xFF; //todos os pinos como digitais
    TRISD = 0x00; //PORTD como output para o lcd
    TRISB = 0x03; //RB0, RB1, RB5, RB6 E RB7  como entrada e resto como sa?da
    INTCON = 0xB0; //HABILITA INTERRUP??O GLOBAL, POR TIMER O, EXTERNA E PELO PORTB
    T0CON = 0x02; //TIMER0 COM 16 BITS E PRESCALE EM 8 para contar 100 milisegundos
    TMR0 = 15536;
    INTCON3bits.INT1IE = 1;	//INTERRUP??O EXTERNA PARA RB1
	PORTBbits.RB7 = 0;	//BUZZER = 0
    
}

void tela_placar(void){
    char aux[16]={0};
    char aux1[16]={0};
    char aux2[16]={0};
    char aux3[16]={0};
    char aux4[16]={0};
    char aux5[16]={0};    
    
    LCD_PutCmd(0x01);   //limpa a tela
    
    //valida?ao jogador 01
    if(min1<10){
        sprintf(aux1, "0%d", min1);
    }else{
        sprintf(aux1, "%d", min1);
    }
    if(seg1<10){
        sprintf(aux2, "0%d", seg1);
    }else{
        sprintf(aux2, "%d", seg1);
    }
    
    strcat(aux,aux1);
    strcat(aux,":");
    strcat(aux,aux2);
    strcat(aux,"-Jogador 1");
    escreve_tela(aux,LINHA_1);
    
    //valida?ao jogador 02
    if(min2<10){
        sprintf(aux3, "0%d", min2);
    }else{
        sprintf(aux3, "%d", min2);
    }
    if(seg2<10){
        sprintf(aux4, "0%d", seg2);
    }else{
        sprintf(aux4, "%d", seg2);
    }
    
    strcat(aux5,aux3);
    strcat(aux5,":");
    strcat(aux5,aux4);
    strcat(aux5,"-Jogador 2");
    escreve_tela(aux5,LINHA_2); 
}

void EEPROM_Write (int address, char data)
{
    /* Write Operation*/
    EEADR=address;		/* Write address to the EEADR register */
    EEDATA=data;		/* Copy data to the EEDATA register for
				write to EEPROM location */
    EECON1bits.EEPGD=0;		/* Access data EEPROM memory */
    EECON1bits.CFGS=0;		/* Access flash program or data memory */
    EECON1bits.WREN=1;		/* Allow write to the memory */
    INTCONbits.GIE=0;		/* Disable global interrupt */
    
    /* Assign below sequence to EECON2 Register is necessary
       to write data to EEPROM memory */

    EECON2=0x55;
    EECON2=0xaa;
    
    EECON1bits.WR=1;		/* Start writing data to EEPROM memory */
    INTCONbits.GIE=1;		/* Enable interrupt*/
    
    while(PIR2bits.EEIF==0);	/* Wait for write operation complete */
    PIR2bits.EEIF=0;		/* Reset EEIF for further write operation */
}

char EEPROM_Read(int address)
{
    /*Read operation*/
    EEADR=address;	/* Read data at location 0x00*/
    EECON1bits.WREN=0;	/* WREN bit is clear for Read operation*/  
    EECON1bits.EEPGD=0;	/* Access data EEPROM memory*/
    EECON1bits.RD=1;	/* To Read data of EEPROM memory set RD=1*/
    return(EEDATA);
}

void main(void) {
    inicia_registradores();
    read_char = EEPROM_Read(0x00);//EEPROM_ReadByte(eeprom_address); //lendo o valor que esta na memoria eeprom
	

	if(read_char == 0){
        min1 = 1;
        min2 = min1;
    }else{
        min1 = read_char;
        min2 = min1;
		minConfigura = read_char;
    }   
	LCD_Init();
    tela_placar(); 


    while (1) {
		
	if(configura){
		if(!PORTAbits.RA4)
			__delay_ms(30);
			if(!PORTAbits.RA4){
				if(minConfigura<60){
					minConfigura++;	
									
				}else{
					minConfigura=1;
				}				
				tela_configura();
				__delay_ms(350);
			}
		
	}
        if(mainEventFlag & INT_1_FLAG){
            __delay_ms(30);
            mainEventFlag &= ~INT_1_FLAG;
            if(PORTBbits.RB1 == 1){
                if(!inicia){
                    inicia = 1;
                    T0CONbits.TMR0ON = 1;
                }
                else
                    passa_jogada();
            }
        }
        if(mainEventFlag & INT_0_FLAG){
            __delay_ms(30);
            mainEventFlag &=~INT_0_FLAG;
            if(PORTBbits.RB0 == 1){
                if(!configura){
                configura = 1;
				inicia = 0;
                T0CONbits.TMR0ON = 0; //desliga o timer
				seg1  = 0;
				seg2  = 0;
                tela_configura();
                }else{
                   configura = 0;
                   //EEPROM_WriteByte(eeprom_address,((unsigned char)minConfigura));    //escreve o tempo seleciondo na eeprom
					EEPROM_Write(0x00,minConfigura);
				   min2=minConfigura;
				   min1=minConfigura;
                   tela_placar();
                    Reset();                   
                }
            }            
        }

    }
    return;
}

