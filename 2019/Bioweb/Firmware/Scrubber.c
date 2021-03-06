// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programaci?n para Cabina de Flujo Laminar Vertical.
// Tiene Contrase?a de incio para permitir funcionamiento de Cabina.
// Tiene Men?:Luz UV, Luz Blanca, Motor Ventilador y Cambio de Contrase?a.
// Permite cambiar la velocidad del motor digitalmente.
// Ing. Juan David Pi?eros.
// JP Inglobal.

#include <18F4550.h>
#device adc=8
#fuses XTPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,NOVREGEN,NOMCLR// PLL1 para 4 MHz
#use delay(clock=48000000)
//#fuses XT,NOWDT,NOPROTECT,NOLVP,NODEBUG,NOVREGEN,NOMCLR// PLL1 para 4 MHz
//#use delay(clock=4000000)
#include <LCD420.c>
#include <MATH.h>

// Definici?n de teclado - NC
#define   UP            !input(PIN_A2)
#define   DOWN          !input(PIN_A3)
#define   RIGHT         !input(PIN_A4)
#define   LEFT          !input(PIN_A5)

#define   HIGH           !input(PIN_E1)
#define   LOW            !input(PIN_E0)
#define   AUX            !input(PIN_E2)

// Definici?n de otros puertos
#define   Bomba_on            output_bit(PIN_D1,1)
#define   Bomba_off           output_bit(PIN_D1,0)
#define   Agua_on             output_bit(PIN_D2,1)
#define   Agua_off            output_bit(PIN_D2,0)
#define   Alarma_on           output_bit(PIN_D3,1)
#define   Alarma_off          output_bit(PIN_D3,0)
#define   Desague_on          output_bit(PIN_D4,1)
#define   Desague_off         output_bit(PIN_D4,0)
#define   Peristaltica_on     output_bit(PIN_C2,1)
#define   Peristaltica_off    output_bit(PIN_C2,0)

short estadobp=0,estadoba=0,estadoagua=0,Flanco=0,Flanco1=0,Flanco2=0,Lectura=0,Grabar=0,SensadoNivelL=0,visualiza=0,renovar=0,adicionBase=0;
short estadode=0,activo=0,SensadoNivelH=0;
int8 Menu=0, n_opcionH=7,n_opcionL=2,Flecha=2,Flecha1=3,Flecha2=2, nMenuL=2, nMenuH=4,segundos=0,minutos=0,horas=0;
int8 r=0,q=0,MediaMovil=6,punto=1,negativo=10,treset=0,tcontrol=0,tpulso=3,tiempoRenovacion=10,cargaH=0,cargaL=0;
int8 bInt=0,bDec=0,Y0Int=0,Y0Dec=0,Y1Int=0,Y1Dec=0;
signed int8   paso=0;
char t[3]={'>',' ','^'}; 

int16 tiempos,TiempoSensadoNivelL=0,TiempoSensadoNivelH=0,cargaInicial=100,tRenovacion=0,tPeristaltica=0;
float PromPH[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float V1=0.0,PH=0.0,X0=0.0,Y0=0.0,X1=0.0,Y1=0.0,pHpoint=4.0,pVpH=0.0,sPpH=7.0,m=0.0,b=0.0,Banda=0.0,tmp=0.0,sPH=0.0,sPL=0.0;
#include <Funciones.h>

#int_TIMER1
void temp1s(void){
   set_timer1(5536);  // 5 ms
   tiempos++;
   if(tiempos>=200){   // Ha transcurrido una decima de segundo (PIC18F4550 con XT = 16MHZ)
      tiempos=0;Lectura=1;treset++;tcontrol++;visualiza=1;
      if(SensadoNivelL){
         TiempoSensadoNivelL++;
      }
      if(SensadoNivelH){
         TiempoSensadoNivelH++;
      }
      if(renovar)
         tRenovacion++;
      if(adicionBase)
         tPeristaltica++;
   //----------------------------- Tiempo Total-----------------------------//
      if(estadoba==1){
         segundos++;
         if(segundos==60){
            segundos=0;minutos++;
         }
         if(minutos==60){
            minutos=0;horas++;
         }
      }     
   }
}

void main ()
{
   ConfigInicial();
   LeeEEPROM();
   LimitaValores();
   lcd_gotoxy(1,1);
   printf(lcd_putc,"     SCRUBBER       ");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"      HUMEDO        ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"    JP INGLOBAL     ");
   lcd_gotoxy(1,4);
   printf(lcd_putc,"  REF:OPSCR120-01   ");
   delay_ms(1500);
   printf(lcd_putc,"\f");
   
   if(UP){
      delay_ms(1000);
      if(UP){
         write_eeprom(50,0);
         delay_ms(10);
         write_eeprom(51,0);
         delay_ms(10);
         write_eeprom(52,0);
         delay_ms(10);
         reset_cpu();
      }
   }
  
   Menu=100;
   while(true){

//------------Menu1------------------------------------------------------------------   
   if(Menu == 1){ // Menu de seleccion de lo que desea encender
   lcd_gotoxy(1,1);
   printf(lcd_putc,"---MENU PRINCIPAL---");
      if(paso<0)
         paso=0;
        
      if(UP){//Si oprime hacia arriba
         if(Flanco == 0){
            Flecha2--;Flecha--;Flecha1=Flecha+1;Flanco = 1;delay_ms(30);
         }
      }else{
         Flanco = 0;
      }
            
      if(DOWN){//Si oprime hacia abajo
         if(Flanco2 == 0){
            Flecha2++;Flecha++;Flecha1=Flecha-1;Flanco2 = 1;delay_ms(30);
         }
      }else{
         Flanco2 = 0;
      }
   
        if(Flecha2>nMenuH)
        {paso++;nMenuH=Flecha2;nMenuL=nMenuH-2;Flecha=4;}
        
        if(Flecha2<nMenuL)
        {paso--;nMenuL=Flecha2;nMenuH=nMenuL+2;Flecha=2;}
        
        if(Flecha2>n_opcionH)
        {Flecha2=n_opcionL;Flecha=2;paso=0;nMenuL=Flecha2;nMenuH=nMenuL+2;}
        
        if(Flecha2<n_opcionL)
        {Flecha2=n_opcionH;Flecha=4;paso=n_opcionH-4;nMenuH=Flecha2;nMenuL=nMenuH-2;}               
        
        mensajes(1+paso,2);
        mensajes(2+paso,3);
        mensajes(3+paso,4);

        lcd_gotoxy(1,Flecha);// Para mostrar la flecha de seleccion
        lcd_putc(t[0]);

        if(Flecha==2)
          {lcd_gotoxy(1,4);// Para mostrar la flecha de seleccion
            lcd_putc(t[1]);
            lcd_gotoxy(1,3);// Para mostrar la flecha de seleccion
            lcd_putc(t[1]);
          }
        
        if(Flecha==4)
          {lcd_gotoxy(1,2);// Para mostrar la flecha de seleccion
            lcd_putc(t[1]);
            lcd_gotoxy(1,3);// Para mostrar la flecha de seleccion
            lcd_putc(t[1]);
          }
        
        if(Flecha==3){
            lcd_gotoxy(1,4);// Para mostrar la flecha de seleccion
            lcd_putc(t[1]);
            lcd_gotoxy(1,2);// Para mostrar la flecha de seleccion
            lcd_putc(t[1]);
        }

       if(RIGHT){// Si oprime derecha
         if(Flanco1 == 0){
            Flanco1 = 1;Menu=Flecha2;Flecha=3;delay_ms(300);printf(lcd_putc,"\f");
         }
       }else{
         Flanco1 = 0;
       }
            
       if(LEFT){
         Menu=100;printf(lcd_putc,"\f");delay_ms(500);
       }

   }
//----------------Fin-Menu1---------------------------------------------------------------      

//----------------Menu2---------------------------------------------------------------
   if(Menu == 2){ // Menu de Calibracion en un punto
   
   lcd_gotoxy(1,1);
   printf(lcd_putc,"Calibracion un Punto");
   lcd_gotoxy(1,2);
   printf(lcd_putc," Ingrese valor  pH  ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"     %2.1f pH  ",pHpoint); 
   
         
   if(pHpoint<0.5)
      pHpoint=0.5;
   
   if(pHpoint>14.0)
      pHpoint=14.0;
      
   if(UP){
      if(pHpoint<14.0){
         pHpoint+=0.1;delay_ms(50);
      }
   }
   
   if(DOWN){
      if(pHpoint>0.5){
         pHpoint-=0.1;delay_ms(50);
      }
   }
   
   if(RIGHT){
      b=pHpoint-pVpH;
      if(b>=0){
         negativo=10;write_eeprom(60,negativo);
      }
      if(b<0){
         negativo=20;write_eeprom(60,negativo);
      }
      b=abs(b);
      bInt=(int8)b;
      tmp=b-bInt;
      bDec=tmp*100;
      write_eeprom(20,bInt);
      delay_ms(20);
      write_eeprom(21,bDec);
      delay_ms(200);
      printf(lcd_putc,"\f");
      lcd_gotoxy(1,2);
      printf(lcd_putc,"   Calibracion   ");
      lcd_gotoxy(1,3);
      printf(lcd_putc,"     Exitosa     ");
      delay_ms(500);
      printf(lcd_putc,"\f");
      Menu=100;
   }
   
   if(LEFT){
      Menu=1;paso=0;delay_ms(1000);Flecha=2;Flecha2=2;printf(lcd_putc,"\f");
   }  
  }
//----------------Fin-Menu2---------------------------------------------------------------
  
//----------------Menu3---------------------------------------------------------------
   if(Menu == 3){ // Menu de calibracion en dos puntos
   
      lcd_gotoxy(1,1);
      printf(lcd_putc,"Calibracion dos Punt");
      lcd_gotoxy(1,2);
      printf(lcd_putc,"Ingrese valor  pH %u",punto);
      lcd_gotoxy(1,3);
      printf(lcd_putc,"     %2.1f pH  ",pHpoint); 
      //lcd_gotoxy(1,4);
      //printf(lcd_putc,"     %2.1f  ",sensores(0)); 
            
      if(pHpoint<0.5)
         pHpoint=0.5;
      
      if(pHpoint>14.0)      
         pHpoint=14.0;
      
      if(UP){
         if(pHpoint<14.0){
            pHpoint+=0.1;delay_ms(50);
         }
      }
      
      if(DOWN){
         if(pHpoint>0.5){
            pHpoint-=0.1;delay_ms(50);
         }
      }
      
      if(RIGHT){
         if(punto==1){
            X0=sensores(0);
            Y0=pHpoint;
            punto++;
            delay_ms(1000);
            pHpoint=7.0;
         }else{
            if(punto==2){
               X1=sensores(0);
               Y1=pHpoint;
               
               Y0Int=(int8)Y0;
               tmp=Y0-Y0Int;
               Y0Dec=tmp*10;
               
               Y1Int=(int8)Y1;
               tmp=Y1-Y1Int;
               Y1Dec=tmp*10;
               
               write_eeprom(30,(int8)X0);
               delay_ms(20);
               write_eeprom(31,(int8)X1);
               delay_ms(20);
               write_eeprom(32,Y0Int);
               delay_ms(20);
               write_eeprom(33,Y0Dec);
               delay_ms(20);
               write_eeprom(34,Y1Int);
               delay_ms(20);
               write_eeprom(35,Y1Dec);
               delay_ms(20);
               
               b=Y1-(X1*(Y1-Y0)/(X1-X0));
               if(b>=0){
                  negativo=10;write_eeprom(60,negativo);delay_ms(20);
               }
               if(b<0){
                  negativo=20;write_eeprom(60,negativo);delay_ms(20);
               }
               b=abs(b);
               bInt=(int8)b;
               tmp=b-bInt;
               bDec=tmp*100;
               write_eeprom(20,bInt);
               delay_ms(20);
               write_eeprom(21,bDec);
               delay_ms(200);
               printf(lcd_putc,"\f");
               lcd_gotoxy(1,2);
               printf(lcd_putc,"   Calibracion   ");
               lcd_gotoxy(1,3);
               printf(lcd_putc,"     Exitosa     ");
               delay_ms(500);
               printf(lcd_putc,"\f"); 
               punto=1;
               Menu=100;
            }
         }
      }
      
      if(LEFT){
         Menu=1;paso=0;delay_ms(200);Flecha=2;Flecha2=2;printf(lcd_putc,"\f");punto=1;
         X0=read_eeprom(30);
         Y0Int=read_eeprom(32);Y0Dec=read_eeprom(33);
         tmp=Y0Dec/10.0;
         Y0=Y0Int+tmp;
      }  
   }      
//----------------Fin-Menu3---------------------------------------------------------------

//----------------Menu4---------------------------------------------------------------
   if(Menu == 4){ // Menu de Media Movil
   
   lcd_gotoxy(1,1);
   printf(lcd_putc,"    Media   Movil   ");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"    Ingrese valor   ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"        %02u   ",MediaMovil); 
   
         
   if(MediaMovil>16){
      MediaMovil=16;
   }
   
   if(UP){
      if(MediaMovil<16){
         MediaMovil++;delay_ms(50);
      }
   }
   
   if(DOWN){
      if(MediaMovil>0){
         MediaMovil--;delay_ms(50);
      }
   }
   
   if(RIGHT || LEFT){
      delay_ms(500);
      if(RIGHT || LEFT){
         write_eeprom(5,MediaMovil);
         delay_ms(100);
         MensajeGuardado();
         Menu=1;paso=0;delay_ms(500);Flecha=2;Flecha2=2;printf(lcd_putc,"\f");
      }
   }
  }
//----------------Fin-Menu4---------------------------------------------------------------

//----------------Menu5---------------------------------------------------------------
   if(Menu == 5){ // Menu de Tiempo de Renovaci?n de Agua
   
   lcd_gotoxy(1,1);
   printf(lcd_putc,"Renovacion   Liquido");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"    Ingrese valor   ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"   Tiempo:%03u horas ",tiempoRenovacion); 
   
         
   if(tiempoRenovacion>100)
      tiempoRenovacion=100;
   if(tiempoRenovacion<1)
      tiempoRenovacion=1;
   
   if(UP){      
      tiempoRenovacion++;delay_ms(50);
   }
   
   if(DOWN){      
      tiempoRenovacion--;delay_ms(50);
   }
   
   if(RIGHT || LEFT){
      delay_ms(1000);
      if(RIGHT || LEFT){
         write_eeprom(6,tiempoRenovacion);
         delay_ms(500);
         Menu=1;paso=0;delay_ms(500);Flecha=2;Flecha2=2;printf(lcd_putc,"\f");
         MensajeGuardado();
      }
   }
  }
//----------------Fin-Menu5---------------------------------------------------------------

//----------------Menu6---------------------------------------------------------------
   if(Menu == 6){ // Menu de Carga Inicial de Base
   
   lcd_gotoxy(1,1);
   printf(lcd_putc," Carga Inicial Base ");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"Ingrese valor en ml ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"   Base:%04Lu ml  ",cargaInicial); 
   
         
   if(cargaInicial>5000)
      cargaInicial=5000;
   
   if(cargaInicial<100)
      cargaInicial=100;
   
   if(UP){
      cargaInicial+=10;delay_ms(50);
   }
   
   if(DOWN){
      cargaInicial-=10;delay_ms(50);
   }
   
   if(RIGHT || LEFT){
      delay_ms(500);
      if(RIGHT || LEFT){
         write_eeprom(8,make8(cargaInicial,0));
         delay_ms(100);
         write_eeprom(9,make8(cargaInicial,1));
         delay_ms(500);
         Menu=1;paso=0;delay_ms(500);Flecha=2;Flecha2=2;printf(lcd_putc,"\f");
         MensajeGuardado();
      }
   }
  }
//----------------Fin-Menu6---------------------------------------------------------------

//----------------Menu7---------------------------------------------------------------
   if(Menu == 7){ // Menu de Mantenimiento
   
   lcd_gotoxy(1,1);
   printf(lcd_putc,"MTO.  pH:%2.1f ",pVpH);   
   lcd_gotoxy(1,2);
   printf(lcd_putc,"b:%2.2f X=%2.0f X1=%2.0f" ,b,X0,X1);
   lcd_gotoxy(1,3);
   printf(lcd_putc,"Y0=%2.1f Y1=%2.1f %02u" ,Y0,Y1,negativo);
   lcd_gotoxy(1,4);
   printf(lcd_putc,"T. Pulso=%02u ADC:%2.0f ",tpulso,sensores(0));   
   
   if(RIGHT){            
      delay_ms(500);
      printf(lcd_putc,"\f");
      Menu=100;
   }
   
   if(LEFT){
      Menu=1;paso=0;delay_ms(500);Flecha=2;Flecha2=2;printf(lcd_putc,"\f");
   }  
  }
//----------------Fin-Menu7---------------------------------------------------------------

//----------------Menu100---------------------------------------------------------------
   if(Menu == 100){ // Menu de seleccion de Estado de Luz UV
   
   if(visualiza==1){
      lcd_gotoxy(1,2);
      printf(lcd_putc,"pH:%2.1f",pVpH);
      
      lcd_gotoxy(1,3);
      printf(lcd_putc,"Time:%03u:%02u",horas,minutos);
   
      if(estadoba==1){
         lcd_gotoxy(1,4);
         printf(lcd_putc,"BOMBA:ON ");
         EnciendeBomba();
      }else{
         lcd_gotoxy(1,4);
         printf(lcd_putc,"BOMBA:OFF");
         ApagaBomba();
      }
   
      if(estadobp==1){
         lcd_gotoxy(13,4);
         printf(lcd_putc,"BASE:ON ");
         Peristaltica_on;
      }else{
         lcd_gotoxy(13,4);
         printf(lcd_putc,"BASE:OFF");
         Peristaltica_off;
      }
   
      if(estadoagua==1){
         lcd_gotoxy(13,2);
         printf(lcd_putc,"AGUA:ON ");
         Agua_on;
      }else{
         lcd_gotoxy(13,2);
         printf(lcd_putc,"AGUA:OFF");
         Agua_off;
      }
      
      if(estadode==1){
         lcd_gotoxy(13,3);
         printf(lcd_putc,"DESA:ON ");
         Desague_on;
      }else{
         lcd_gotoxy(13,3);
         printf(lcd_putc,"DESA:OFF");
         Desague_off;
      }
      visualiza=0;
   }
   
   if(!AUX){
      if(activo){
         activo=0;
         estadoba=0;
         estadobp=0;
         estadode=0;
         estadoagua=0;
      }
      if(horas>=tiempoRenovacion){
         renovar=1;
         if(tRenovacion<1800){
            estadode=1;
            Desague_on;
            ApagaBomba();
            Agua_off;
            estadoagua=0;
         }else{
            Desague_off;
            estadode=0;
            if(tPeristaltica>=(0.6*cargaInicial)){
               estadobp=0;
               Peristaltica_off;
               adicionBase=0;
            }else{
               estadobp=1;
               Peristaltica_on;
               adicionBase=1;
            }
            if(!LOW){//Si no sensa agua en el nivel alto y no se ha llenado el calderin         
               estadoba=0;
               ApagaBomba();               
               Agua_on;
               estadoagua=1;
            }else{
               if(HIGH){
                  estadoagua=0;
                  Agua_off;
                  if(!adicionBase){
                     renovar=0;
                     tRenovacion=0;
                     tPeristaltica=0;
                     horas=0;
                     minutos=0;
                     segundos=0;
                     write_eeprom(50,segundos);delay_ms(20);
                     write_eeprom(51,minutos);delay_ms(20);
                     write_eeprom(52,horas);delay_ms(20);
                  }
               }
            }
         }
      }else{ 
         if(RIGHT){
            if(Flanco == 0){
               estadoagua=!estadoagua;Flanco = 1;delay_ms(30);
            }
         }else{
            Flanco = 0;
         }
         
         if(DOWN){
            if(Flanco1 == 0){
               estadoba=!estadoba;Flanco1 = 1;delay_ms(30);
            }
         }else{
            Flanco1 = 0;
         }
         
         if(LEFT){
            if(Flanco2 == 0){
               estadobp=!estadobp;Flanco2 = 1;delay_ms(30);
            }
         }else{
            Flanco2 = 0;
         }
      }
   }else{
      renovar=0;
      adicionBase=0;
      if(tRenovacion<1800)
         tRenovacion=0;
      estadode=0;
      estadobp=0;
      Desague_off;
      Peristaltica_off;
      activo=1;
   }
   
// Monitoreo Nivel de Agua//--------------------------------------
   if(!LOW){//Si no sensa agua en el nivel alto y no se ha llenado el calderin         
      estadoba=0;
      ApagaBomba();
      lcd_gotoxy(1,1);
      printf(lcd_putc,"!NIVEL DE AGUA BAJO!");
      if(AUX){
         Agua_on;
         estadoagua=1;
      }
      SensadoNivelL=0;
      TiempoSensadoNivelL=0;
   }else{
      lcd_gotoxy(1,1);
      printf(lcd_putc,"JP SCRUBBER TR:%03u h",tiempoRenovacion);
      Alarma_off;
      if(TiempoSensadoNivelL>3){
         SensadoNivelL=0;
         if(AUX){
            EnciendeBomba();
            estadoba=1;
         }
         if(HIGH){         
            if(TiempoSensadoNivelH>3){
               SensadoNivelH=0;
               estadoagua=0;
               Agua_off;
            }else{
               SensadoNivelH=1;
            }
         }else{
            SensadoNivelH=0;
            TiempoSensadoNivelH=0;
         }
      }else{
         SensadoNivelL=1;
      }
   }
// Monitoreo Nivel de Agua//--------------------------------------

   if(UP){
      delay_ms(100);
      if(UP){
         delay_ms(2000);
         if(UP){
            printf(lcd_putc,"\f");delay_ms(30);Menu=1; paso=0;Flecha=2;Flecha2=2;Flanco=1;
         }else if(!AUX){
            estadode=!estadode;
            delay_ms(100);
         }
      }
   }
    
   }
//----------------Fin-Menu100--------------------------------------------------------------   

   if(treset>=30){
      lcd_init();
      treset=0;
   }
   
   if(tcontrol>10)
      tcontrol=0;
   
   if(Lectura==1){
      if(!LOW)
         output_toggle(PIN_D3);
      pVpH=Leer_Sensor_ph(MediaMovil);
      Lectura=0;
   }
   
   }
}

