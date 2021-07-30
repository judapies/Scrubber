// Cuatro pulsadores de entrada (Derecha, Izquierda, arriba y abajo).
// Reloj con XT de 4 MHz.
// Programación para Cabina de Flujo Laminar Vertical.
// Tiene Contraseña de incio para permitir funcionamiento de Cabina.
// Tiene Menú:Luz UV, Luz Blanca, Motor Ventilador y Cambio de Contraseña.
// Permite cambiar la velocidad del motor digitalmente.
// Ing. Juan David Piñeros.
// JP Inglobal.

#include <18F4550.h>
#device adc=8
#fuses XTPLL,NOWDT,NOPROTECT,NOLVP,NODEBUG,USBDIV,PLL1,CPUDIV1,NOVREGEN,NOMCLR// PLL1 para 4 MHz
#use delay(clock=48000000)
//#fuses XT,NOWDT,NOPROTECT,NOLVP,NODEBUG,NOVREGEN,NOMCLR// PLL1 para 4 MHz
//#use delay(clock=4000000)
#include <LCD420.c>
#include <MATH.h>

// Definición de teclado - NC
#define   UP            !input(PIN_A2)
#define   DOWN          !input(PIN_A3)
#define   RIGHT         !input(PIN_A4)
#define   LEFT          !input(PIN_A5)

#define   HIGH          input(PIN_E1)
#define   LOW           input(PIN_E0)
#define   NLBAS         input(PIN_E2)

// Definición de otros puertos
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


short estadobp=0,estadoba=0,estadoagua=0,Flanco=0,Flanco1=0,Flanco2=0,Flanco3=0,Lectura=0,Grabar=0,SensadoNivel=0,visualiza=0;
int8 Menu=0, n_opcionH=8,n_opcionL=2,unidad=11,Flecha=2,Flecha1=3,Flecha2=2, nMenuL=2, nMenuH=4,segundos=0,minutos=0,horas=0;
int8 r=0,q=0,i=0,MediaMovil=6,punto=1,negativo=10,treset=0,tcontrol=0,tpulso=3;
int8 BandaInt=0,BandaDec=0,sPpHInt=0,sPpHDec=0,bInt=0,bDec=0,Y0Int=0,Y0Dec=0,Y1Int=0,Y1Dec=0;
signed int8   paso=0;
char t[3]={'>',' ','^'}; 
signed  int8 clave[4]={0,0,0,0};   // 0=Verdadero, 1=Falso
int8 contrasena[4]={0,0,0,0};   // 0=Verdadero, 1=Falso

int16 tiempos,TiempoSensadoNivel=0;
float PromPH[20]={0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};
float V1=0.0,PH=0.0,X0=0.0,Y0=0.0,X1=0.0,Y1=0.0,pHpoint=4.0,pVpH=0.0,sPpH=7.0,m=0.0,b=0.0,Banda=0.0,tmp=0.0,sPH=0.0,sPL=0.0;

#int_TIMER1
void temp1s(void){
   set_timer1(5536);  // 5 ms
   tiempos++;
   if(tiempos>=200)   // Ha transcurrido una decima de segundo (PIC18F4550 con XT = 16MHZ)
   {tiempos=0;Lectura=1;treset++;tcontrol++;visualiza=1;
      if(SensadoNivel){
         TiempoSensadoNivel++;
      }
   //----------------------------- Tiempo Total-----------------------------//
   if(estadoba==1)
   {
      segundos++;
      if(segundos==60)
      {
         segundos=0;minutos++;
      }
      if(minutos==60)
      {
         minutos=0;horas++;
      }
   }
        
   }
}


void mensajes(int8 x,y)// Funcion para imprimir mensajes de Menu Principal.
{  
   if(x==1)
   {lcd_gotoxy(2,y);printf(lcd_putc,"Cal. un Punto      ");}
   if(x==2)
   {lcd_gotoxy(2,y);printf(lcd_putc,"Cal. dos Puntos    ");}
   if(x==3)
   {lcd_gotoxy(2,y);printf(lcd_putc,"Cambio Contraseña  ");}
   if(x==4)
   {lcd_gotoxy(2,y);printf(lcd_putc,"Media Movil        ");}
   if(x==5)
   {lcd_gotoxy(2,y);printf(lcd_putc,"Banda Muerta       ");}
   if(x==6)
   {lcd_gotoxy(2,y);printf(lcd_putc,"Setpoint           ");}
   if(x==7)
   {lcd_gotoxy(2,y);printf(lcd_putc,"Mantenimiento      ");}
}

float sensores(int x){
float y;set_adc_channel(x);delay_ms(10);y=read_adc();return (y);
}

float Leer_Sensor_pH(int media){
   float promediopH=0.0;
   V1 = sensores(0);   //Lectura de ADC de 10 bit
   m=(Y1-Y0)/(X1-X0); //Y1=pH ingresado, Y0=pH ingresado, X0=ADC de Y0, X1=ADC de Y1
   if(negativo==10)
   {
      PH=(m*V1)+b;// PH=mx+b
   }
   if(negativo==20)
   {
      PH=(m*V1)-b;// PH=mx+b
   }  
   
   if(PH<0.0)
   {
      PH=0.0;
   }

   if(r>media-1)
   {r=0;}
   PromPH[r]=PH;r++;
         
   for(q=0;q<=(media-1);q++)
   {
      promediopH+=PromPH[q];
   } 
   promediopH=promediopH/media;   
   
   return promediopH;
}

void Controla(void){
   sPH=sPpH+Banda;
   sPL=sPpH-Banda;
   if(pVpH>sPL && pVpH<sPH)
   {
      Peristaltica_off;
      estadobp=0;
   }
   else
   {
      if(pVpH<sPL)
      {
         if(tcontrol<tpulso)
         {
            Peristaltica_on;
            estadobp=1;
         }
         else
         {
            Peristaltica_off;
            estadobp=0;
         }
      }
      else
      {
         Peristaltica_off;
         estadobp=0;
      }
   }
}

void main ()
{
   lcd_init();
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_1);
   enable_interrupts(INT_TIMER1);
   setup_adc_ports(AN0_TO_AN1);
   setup_adc(ADC_CLOCK_DIV_32 );
   enable_interrupts(global);
   sensores(0);
   lcd_gotoxy(1,1);
   printf(lcd_putc,"     SCRUBBER       ");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"                    ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"    JP INGLOBAL     ");
   lcd_gotoxy(1,4);
   printf(lcd_putc,"  REF:JPOPSCBB02   ");
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
   //Solo para simulacion
   //write_eeprom(0,0);delay_ms(20);write_eeprom(1,0);delay_ms(20);
   //write_eeprom(2,0);delay_ms(20);write_eeprom(3,0);delay_ms(20);
   
   // Lee la contraseña almacenada en la eeprom para poder comprobar con la que escribe el usuario.
   contrasena[0]=read_eeprom(0);delay_ms(10);
   if(contrasena[0]>9){
      contrasena[0]=0;write_eeprom(0,0);delay_ms(10);
   }
   contrasena[1]=read_eeprom(1);delay_ms(20);
   if(contrasena[1]>9){
      contrasena[1]=0;write_eeprom(1,0);delay_ms(10);
   }
   contrasena[2]=read_eeprom(2);delay_ms(20);
   if(contrasena[2]>9){
      contrasena[2]=0;write_eeprom(2,0);delay_ms(10);
   }
   contrasena[3]=read_eeprom(3);delay_ms(20);
   if(contrasena[3]>9){
      contrasena[3]=0;write_eeprom(3,0);delay_ms(10);
   }
   MediaMovil=read_eeprom(5);delay_ms(20);
   if(MediaMovil>16){
      MediaMovil=16;write_eeprom(5,16);delay_ms(10);
   }
   BandaInt=read_eeprom(6);delay_ms(20);
   if(BandaInt>10){
      BandaInt=10;write_eeprom(6,10);delay_ms(10);
   }
   BandaDec=read_eeprom(7);delay_ms(20);
   if(BandaDec>9){
      BandaDec=0;write_eeprom(7,0);delay_ms(10);
   }
   sPpHInt=read_eeprom(8);delay_ms(20);
   if(sPpHInt>14){
      sPpHInt=7;write_eeprom(8,7);delay_ms(10);
   }
   sPpHDec=read_eeprom(9);delay_ms(20);
   if(sPpHDec>9){
      sPpHDec=0;write_eeprom(9,0);delay_ms(10);
   }
   bInt=read_eeprom(20);
   delay_ms(20);
   bDec=read_eeprom(21);
   delay_ms(20);
   X0=read_eeprom(30);
   delay_ms(20);
   X1=read_eeprom(31);
   delay_ms(20);
   Y0Int=read_eeprom(32);
   delay_ms(20);
   Y0Dec=read_eeprom(33);
   delay_ms(20);
   Y1Int=read_eeprom(34);
   delay_ms(20);
   Y1Dec=read_eeprom(35);
   delay_ms(20);
   segundos=read_eeprom(50);
   delay_ms(20);
   minutos=read_eeprom(51);
   delay_ms(20);
   horas=read_eeprom(52);
   delay_ms(20);
   negativo=read_eeprom(60);
   delay_ms(20);
   tpulso=read_eeprom(70);
   delay_ms(20);
   
   tmp=Y0Dec/10.0;
   Y0=Y0Int+tmp;
   
   tmp=Y1Dec/10.0;
   Y1=Y1Int+tmp;
   
   tmp=bDec/100.0;
   b=bInt+tmp;
   
   tmp=sPpHDec/10.0;
   sPpH=sPpHInt+tmp;
   
   tmp=BandaDec/10.0;
   Banda=BandaInt+tmp;
   
   if(b>255){
      b=0.32;
   }
   if(X0>250){
      X0=60;
   }
   if(X1>250){
      X1=103;
   }
   if(Y0>250){
      Y0=4.0;
   }
   if(Y1>250){
      Y1=7.0;
   }
   if(negativo>20){
      negativo=20;
   }
   //b=0.32
   //x=60
   //x1=103
   //Y0=4.0
   //Y1=7.0
   //negativo=20;
  
   Menu=100;
   while(true){

//------------Menu0------------------------------------------------------------------   
   if(Menu == 0){ // Menu de Contraseña para Poder iniciar el equipo
   lcd_gotoxy(1,1);
   printf(lcd_putc,"     Ingrese        ");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"    Contraseña      ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"    CLAVE=%i%i%i%i  ",clave[0],clave[1],clave[2],clave[3]);
   
      
      if(UP)//Si oprime hacia arriba
      {
         if(Flanco == 0)
         {
            clave[unidad-11]++;Flanco = 1;delay_ms(30);
         }
      }
      else
      {
         Flanco = 0;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {
         if(Flanco2 == 0)
         {
            clave[unidad-11]--;Flanco2 = 1;delay_ms(30);
         }
      }
      else
      {
         Flanco2 = 0;
      }
   
      if(RIGHT)// Si Oprime Derecha
      {
         if(Flanco1 == 0)
         {
            Flanco1 = 1;unidad++;delay_ms(30);printf(lcd_putc,"\f");
         }
      }
      else
      {
         Flanco1 = 0;
      }
            
      if(LEFT)// Si Oprime izquierda
      {
         if(Flanco3 == 0)
         {
            Flanco3 = 1;unidad--;delay_ms(30);printf(lcd_putc,"\f");
         }
      }
      else
      {
         Flanco3 = 0;
      }
            
       if(clave[unidad-11]<0)     // Si la unidad donde se encuentra ubicado el cursor es menor que 0 pasa a 9.
       {clave[unidad-11]=9;}
       if(clave[unidad-11]>9)     // Si la unidad donde se encuentra ubicado el cursor es mayor que 9 pasa a 0.
       {clave[unidad-11]=0;}
       if(unidad<11)             // Si trata de correr mas a la izquierda de la primera unidad, deja el cursor en esa posicion.
       {unidad=11;}
       lcd_gotoxy(unidad,4);// Para mostrar cursor.
       lcd_putc(t[2]);
       if(unidad>14)             // Si a Terminado de ingresar la clave, verifica si es correcta o no.
       {
         if(clave[0]==3&&clave[1]==8&&clave[2]==9&&clave[3]==2) // Si Ingresa clave para reset general del sistema.
            {write_eeprom(0,0);delay_ms(20);write_eeprom(1,0);delay_ms(20);// Reestablece a contraseña de Fabrica y reinicia Programa.
             write_eeprom(2,0);delay_ms(20);write_eeprom(3,0);delay_ms(20);
             reset_cpu();}
            
         if(clave[0]==contrasena[0]&&clave[1]==contrasena[1]&&clave[2]==contrasena[2]&&clave[3]==contrasena[3]) // Si las claves coinciden pasa a Menu Principal.
            {lcd_gotoxy(1,1);
            printf(lcd_putc,"                   ");
            lcd_gotoxy(1,2);
            printf(lcd_putc,"     Contraseña    ");
            lcd_gotoxy(1,3);
            printf(lcd_putc,"      Correcta     ");
            lcd_gotoxy(1,4);
            printf(lcd_putc,"                   ");
            delay_ms(1000);Menu=100;unidad=11;printf(lcd_putc,"\f");}
         else                                         // Si la clave no coincide vuelve a mostrar el menu para ingresar la clave.
         {lcd_gotoxy(1,1);
            printf(lcd_putc,"");
            lcd_gotoxy(1,2);
            printf(lcd_putc,"     Contraseña    ");
            lcd_gotoxy(1,3);
            printf(lcd_putc,"     Incorrecta    ");
            lcd_gotoxy(1,4);
            printf(lcd_putc,"                   ");
            delay_ms(1000);unidad=11;printf(lcd_putc,"\f");}
       }
   
       if(unidad>11&&unidad<14)
          { lcd_gotoxy(unidad-1,4);// Para mostrar cursor.
            lcd_putc(t[1]);
          }

   }
//----------------Fin-Menu0---------------------------------------------------------------   

//------------Menu1------------------------------------------------------------------   
   if(Menu == 1){ // Menu de seleccion de lo que desea encender
   lcd_gotoxy(1,1);
   printf(lcd_putc,"---MENU PRINCIPAL---");
      if(paso<0)
        {paso=0;}
        
      if(UP)//Si oprime hacia arriba
      {
         if(Flanco == 0)
         {
            Flecha2--;Flecha--;Flecha1=Flecha+1;Flanco = 1;delay_ms(30);
         }
      }
      else
      {
         Flanco = 0;
      }
            
      if(DOWN)//Si oprime hacia abajo
      {
         if(Flanco2 == 0)
         {
            Flecha2++;Flecha++;Flecha1=Flecha-1;Flanco2 = 1;delay_ms(30);
         }
      }
      else
      {
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
        
        if(Flecha==3)
          { lcd_gotoxy(1,4);// Para mostrar la flecha de seleccion
            lcd_putc(t[1]);
            lcd_gotoxy(1,2);// Para mostrar la flecha de seleccion
            lcd_putc(t[1]);
          }

       if(RIGHT)// Si oprime derecha
       {
         if(Flanco1 == 0)
         {
            Flanco1 = 1;Menu=Flecha2;Flecha=3;delay_ms(300);printf(lcd_putc,"\f");
            if(Menu==6)
            {
               clave[0]=0;clave[1]=0;clave[2]=0;clave[3]=0;
            }
         }
       }
       else
       {
         Flanco1 = 0;
       }
            
       if(LEFT)
       {
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
   {
      pHpoint=0.5;
   }
   if(pHpoint>14.0)
   {
      pHpoint=14.0;
   }
   
   if(UP)
   {
      if(pHpoint<14.0)
      {
         pHpoint+=0.1;delay_ms(50);
      }
   }
   
   if(DOWN)
   {
      if(pHpoint>0.5)
      {
         pHpoint-=0.1;delay_ms(50);
      }
   }
   
   if(RIGHT)
   {
      b=pHpoint-pVpH;
      if(b>=0)
      {
         negativo=10;write_eeprom(60,negativo);
      }
      if(b<0)
      {
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
   
   if(LEFT)
   {
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
      {
         pHpoint=0.5;
      }
      if(pHpoint>14.0)
      {
         pHpoint=14.0;
      }
      
      if(UP)
      {
         if(pHpoint<14.0)
         {
            pHpoint+=0.1;delay_ms(50);
         }
      }
      
      if(DOWN)
      {
         if(pHpoint>0.5)
         {
            pHpoint-=0.1;delay_ms(50);
         }
      }
      
      if(RIGHT)
      {
         if(punto==1)
         {
            X0=sensores(0);
            Y0=pHpoint;
            punto++;
            printf(lcd_putc,"\f");
            lcd_gotoxy(1,2);
            printf(lcd_putc,"   Ajustando     ");
            lcd_gotoxy(1,3);
            printf(lcd_putc,"    Punto 1      ");
            delay_ms(500);
            delay_ms(1000);
            printf(lcd_putc,"\f");
            pHpoint=7.0;
         }
         else
         {
            if(punto==2)
            {
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
               if(b>=0)
               {
                  negativo=10;write_eeprom(60,negativo);delay_ms(20);
               }
               if(b<0)
               {
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
      
      if(LEFT)
      {
         Menu=1;paso=0;delay_ms(200);Flecha=2;Flecha2=2;printf(lcd_putc,"\f");punto=1;
         X0=read_eeprom(30);
         Y0Int=read_eeprom(32);Y0Dec=read_eeprom(33);
         tmp=Y0Dec/10.0;
         Y0=Y0Int+tmp;
      }  
   }      
//----------------Fin-Menu3---------------------------------------------------------------

//----------------Menu4---------------------------------------------------------------
   if(Menu == 4){ // Menu para Cambio de Contraseña
   lcd_gotoxy(1,1);
   printf(lcd_putc,"      Ingrese       ");
   lcd_gotoxy(1,2);
   printf(lcd_putc," Contraseña Actual  ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"    CLAVE=%i%i%i%i  ",clave[0],clave[1],clave[2],clave[3]);
   
      
   if(UP)//Si oprime hacia arriba
      {  if(Flanco == 0) 
            {clave[unidad-11]++;Flanco = 1;delay_ms(30);}}
         else
            {Flanco = 0;}
            
      if(DOWN)//Si oprime hacia abajo
      {  if(Flanco2 == 0) 
            {clave[unidad-11]--;Flanco2 = 1;delay_ms(30);}}
         else
            {Flanco2 = 0;}
   
      if(RIGHT)// Si oprime derecha
      {  if(Flanco1 == 0) 
            {Flanco1 = 1;unidad++;delay_ms(30);printf(lcd_putc,"\f");}}
         else
            {Flanco1 = 0;}   
            
       if(LEFT)// Si oprime Izquierda
      {  if(Flanco3 == 0) 
            {Flanco3 = 1;unidad--;delay_ms(30);printf(lcd_putc,"\f");}}
         else
            {Flanco3 = 0;}    
            
       if(clave[unidad-11]<0)// Si la unidad donde se encuentra ubicado el cursor es menor que 0 pasa a 9.     
       {clave[unidad-11]=9;}
       if(clave[unidad-11]>9)// Si la unidad donde se encuentra ubicado el cursor es mayor que 9 pasa a 0.     
       {clave[unidad-11]=0;}
       if(unidad<11)
       {Menu=1;paso=0;Flecha=2;Flecha2=2;unidad=11;delay_ms(500);}
       lcd_gotoxy(unidad,4);// Para mostrar la flecha de seleccion
       lcd_putc(t[2]);
       if(unidad>14)// Si ya ingreso la contraseña muestra si es correcta o no, dependiendo si ingreso la clave correctamente.
       {
         if(clave[0]==contrasena[0]&&clave[1]==contrasena[1]&&clave[2]==contrasena[2]&&clave[3]==contrasena[3])
            {lcd_gotoxy(1,1);
            printf(lcd_putc,"                    ");
            lcd_gotoxy(1,2);
            printf(lcd_putc,"     Contraseña     ");
            lcd_gotoxy(1,3);
            printf(lcd_putc,"      Correcta      ");
            lcd_gotoxy(1,4);
            printf(lcd_putc,"                    ");
            delay_ms(500);Menu=61;unidad=11;printf(lcd_putc,"\f");
            clave[0]=0;clave[1]=0;clave[2]=0;clave[3]=0;}
         else
         {lcd_gotoxy(1,1);
            printf(lcd_putc,"                    ");
            lcd_gotoxy(1,2);
            printf(lcd_putc,"     Contraseña     ");
            lcd_gotoxy(1,3);
            printf(lcd_putc,"     Incorrecta     ");
            lcd_gotoxy(1,4);
            printf(lcd_putc,"                    ");
            delay_ms(500);unidad=11;printf(lcd_putc,"\f");}
       }
               
          if(unidad>11&&unidad<14)
          { lcd_gotoxy(unidad-1,4);// Para mostrar cursor.
            lcd_putc(t[1]);
          }
   }      
//----------------Fin-Menu4---------------------------------------------------------------

//----------------Menu6.1---------------------------------------------------------------
   if(Menu == 61){ // Menu cuando ingresa correctamente la contraseña, permite que digite nueva contraseña.
   lcd_gotoxy(1,1);
   printf(lcd_putc,"     Ingrese        ");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"  Contraseña Nueva  ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"    CLAVE=%i%i%i%i  ",clave[0],clave[1],clave[2],clave[3]);
   
      
   if(UP)//Si oprime hacia arriba
      {  if(Flanco == 0) 
            {clave[unidad-11]++;Flanco = 1;delay_ms(30);}}
         else
            {Flanco = 0;}
            
      if(DOWN)//Si oprime hacia abajo
      {  if(Flanco2 == 0) 
            {clave[unidad-11]--;Flanco2 = 1;delay_ms(30);}}
         else
            {Flanco2 = 0;}
   
      if(RIGHT)// Si oprime Derecha
      {  if(Flanco1 == 0) 
            {Flanco1 = 1;unidad++;delay_ms(30);printf(lcd_putc,"\f");}}
         else
            {Flanco1 = 0;}   
            
       if(LEFT)// Si oprime Izquierda
      {  if(Flanco3 == 0) 
            {Flanco3 = 1;unidad--;delay_ms(30);printf(lcd_putc,"\f");}}
         else
            {Flanco3 = 0;}    
            
       if(clave[unidad-11]<0)// Si la unidad donde se encuentra ubicado el cursor es menor que 0 pasa a 9.     
       {clave[unidad-11]=9;}
       if(clave[unidad-11]>9)// Si la unidad donde se encuentra ubicado el cursor es mayor que 9 pasa a 0.     
       {clave[unidad-11]=0;}
       if(unidad<11)
       {Menu=5;unidad=11;}
       lcd_gotoxy(unidad,4);// Para mostrar la flecha de seleccion
       lcd_putc(t[2]);
       if(unidad>14)// Si ya ingreso la nueva contraseña.
       {
            lcd_gotoxy(1,1);
            printf(lcd_putc,"                    ");
            lcd_gotoxy(1,2);
            printf(lcd_putc,"     Contraseña     ");
            lcd_gotoxy(1,3);
            printf(lcd_putc,"     Almacenada     ");
            lcd_gotoxy(1,4);
            printf(lcd_putc,"                    ");
            write_eeprom(0,clave[0]);delay_ms(20);write_eeprom(1,clave[1]);delay_ms(20);
            write_eeprom(2,clave[2]);delay_ms(20);write_eeprom(3,clave[3]);delay_ms(20);
            delay_ms(500);Menu=1;paso=0;Flecha=2;Flecha2=2;
       }
      
           if(unidad>11&&unidad<14)
          { lcd_gotoxy(unidad-1,4);// Para mostrar cursor.
            lcd_putc(t[1]);
          }
   }      
//----------------Fin-Menu6.1---------------------------------------------------------------

//----------------Menu5---------------------------------------------------------------
   if(Menu == 5){ // Menu de Media Movil
   
   lcd_gotoxy(1,1);
   printf(lcd_putc,"    Media   Movil    ");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"    Ingrese valor    ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"        %02u   ",MediaMovil); 
   
         
   if(MediaMovil>16)
   {
      MediaMovil=16;
   }
   
   if(UP)
   {
      if(MediaMovil<16)
      {
         MediaMovil++;delay_ms(50);
      }
   }
   
   if(DOWN)
   {
      if(MediaMovil>0)
      {
         MediaMovil--;delay_ms(50);
      }
   }
   
   if(RIGHT)
   {
      write_eeprom(5,MediaMovil);
      delay_ms(100);
      printf(lcd_putc,"\f");
      Menu=100;
   }
   
   if(LEFT)
   {
      Menu=1;paso=0;delay_ms(500);Flecha=2;Flecha2=2;printf(lcd_putc,"\f");
   }  
  }
//----------------Fin-Menu5---------------------------------------------------------------

//----------------Menu6---------------------------------------------------------------
   if(Menu == 6){ // Menu de Banda Muerta
   
   lcd_gotoxy(1,1);
   printf(lcd_putc,"    Banda Muerta     ");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"    Ingrese valor    ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"      +/-%2.1f   ",Banda); 
   
         
   if(Banda>10.0)
   {
      Banda=10.0;
   }
   
   if(UP)
   {
      if(Banda<10.0)
      {
         Banda+=0.1;delay_ms(50);
      }
   }
   
   if(DOWN)
   {
      if(Banda>0.1)
      {
         Banda-=0.1;delay_ms(50);
      }
   }
   
   if(RIGHT)
   {
      BandaInt=(int8)Banda;
      tmp=Banda-BandaInt;
      BandaDec=tmp*10;
      write_eeprom(6,BandaInt);
      delay_ms(20);
      write_eeprom(7,BandaDec);
      delay_ms(500);
      printf(lcd_putc,"\f");
      Menu=100;
   }
   
   if(LEFT)
   {
      Menu=1;paso=0;delay_ms(500);Flecha=2;Flecha2=2;printf(lcd_putc,"\f");
   }  
  }
//----------------Fin-Menu6---------------------------------------------------------------

//----------------Menu7---------------------------------------------------------------
   if(Menu == 7){ // Menu de Setpoint
   
   lcd_gotoxy(1,1);
   printf(lcd_putc,"    SetPoint  pH     ");
   lcd_gotoxy(1,2);
   printf(lcd_putc,"    Ingrese valor    ");
   lcd_gotoxy(1,3);
   printf(lcd_putc,"      %2.1f  pH ",sPpH); 
   
         
   if(sPpH>14.0)
   {
      sPpH=14.0;
   }
   
   if(UP)
   {
      if(sPpH<13.9)
      {
         sPpH+=0.1;delay_ms(50);
      }
   }
   
   if(DOWN)
   {
      if(sPpH>0.1)
      {
         sPpH-=0.1;delay_ms(50);
      }
   }
   
   if(RIGHT)
   {
      spPhInt=(int8)sPpH;
      tmp=spPh-spPhInt;
      spPhDec=tmp*10;
      write_eeprom(8,spPhInt);
      delay_ms(20);
      write_eeprom(9,spPhDec);
      delay_ms(500);
      printf(lcd_putc,"\f");
      Menu=100;
   }
   
   if(LEFT)
   {
      Menu=1;paso=0;delay_ms(500);Flecha=2;Flecha2=2;printf(lcd_putc,"\f");
   }  
  }
//----------------Fin-Menu7---------------------------------------------------------------

//----------------Menu8---------------------------------------------------------------
   if(Menu == 8){ // Menu de Mantenimiento
   
   lcd_gotoxy(1,1);
   printf(lcd_putc,"MTO.  pH:%2.1f ",pVpH);   
   lcd_gotoxy(1,2);
   printf(lcd_putc,"b:%2.2f X=%2.0f X1=%2.0f" ,b,X0,X1);
   lcd_gotoxy(1,3);
   printf(lcd_putc,"Y0=%2.1f Y1=%2.1f %02u" ,Y0,Y1,negativo);
   lcd_gotoxy(1,4);
   printf(lcd_putc,"T. Pulso=%02u ADC:%2.0f ",tpulso,sensores(0));
         
   if(tpulso>10.0)
   {
      tpulso=10.0;
   }
   
   if(UP)
   {
      if(tpulso<10)
      {
         tpulso++;delay_ms(50);
      }
   }
   
   if(DOWN)
   {
      if(tpulso>0)
      {
         tpulso--;delay_ms(50);
      }
   }
   
   if(RIGHT)
   {      
      write_eeprom(70,tpulso);
      delay_ms(500);
      printf(lcd_putc,"\f");
      Menu=100;
   }
   
   if(LEFT)
   {
      Menu=1;paso=0;delay_ms(500);Flecha=2;Flecha2=2;printf(lcd_putc,"\f");
   }  
  }
//----------------Fin-Menu8---------------------------------------------------------------

//----------------Menu100---------------------------------------------------------------
   if(Menu == 100){ // Menu de seleccion de Estado de Luz UV
   
   if(visualiza==1){
      lcd_gotoxy(1,2);
      printf(lcd_putc,"pH:%2.1f",pVpH);
      
      lcd_gotoxy(1,3);
      printf(lcd_putc,"Time: %03u:%02u (H:M)",horas,minutos);
   
      if(estadoba==1){
         lcd_gotoxy(1,4);
         printf(lcd_putc,"BOMBA:ON ");
         Bomba_on;
         Grabar=0;
      }else{
         lcd_gotoxy(1,4);
         printf(lcd_putc,"BOMBA:OFF");
         Bomba_off;
         if(Grabar==0){
            write_eeprom(50,segundos);delay_ms(20);
            write_eeprom(51,minutos);delay_ms(20);
            write_eeprom(52,horas);delay_ms(20);
            Grabar=1;
         }
      }
   
      if(estadobp==1){
         lcd_gotoxy(12,4);
         printf(lcd_putc,"BASE:ON ");
         Peristaltica_on;
      }else{
         lcd_gotoxy(12,4);
         printf(lcd_putc,"BASE:OFF");
         Peristaltica_off;
      }
   
      if(estadoagua==1){
         lcd_gotoxy(12,2);
         printf(lcd_putc,"AGUA:ON ");
         Agua_on;
      }else{
         lcd_gotoxy(12,2);
         printf(lcd_putc,"AGUA:OFF");
         Agua_off;
      }
      visualiza=0;
   }
   
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
   
   // Monitoreo Nivel de Agua//--------------------------------------
  if(!LOW){//Si no sensa agua en el nivel alto y no se ha llenado el calderin         
      estadoba=0;
      Bomba_off;
      lcd_gotoxy(1,1);
      printf(lcd_putc,"!NIVEL DE AGUA BAJO!");
   }else{
      lcd_gotoxy(1,1);
      printf(lcd_putc,"JP SCRUBBER         ");
      if(HIGH){
         estadoagua=0;
         Agua_off;
      }
   }
   
   if(UP){
      printf(lcd_putc,"\f");delay_ms(30);Menu=1; paso=0;Flecha=2;Flecha2=2;
   }
    
   }
//----------------Fin-Menu100--------------------------------------------------------------   

   if(treset>=10)
   {
      lcd_init();
      treset=0;
   }
   
   if(tcontrol>10)
   {
      tcontrol=0;
   }
   
   if(Lectura==1)
   {
      pVpH=Leer_Sensor_ph(MediaMovil);
      
      if(Menu>0)
      {
         Controla();
      }
      Lectura=0;
   }
   
   }
}

