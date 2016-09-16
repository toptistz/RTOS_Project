#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 16, 2);
#define Lamp 4
int infraredpin = 5; 
int ldrpin = A0;

int ldrval, infraredval;
int receiveinfrared;
int receiveldr;
int checkldr = 0;
int checkinfrared = 0;

String showstatus, showldr;
#include "FreeRTOS_AVR.h"
//#include "task.h"
//#include "queue.h"

#include "basic_io_avr.h"


static void vSenderTask( void *pvParameters );
static void vReceiverTask( void *pvParameters );


QueueHandle_t xQueue;
QueueHandle_t xQueueldr;

void setup( void )
{
  Serial.begin(9600);
  lcd.begin();
  pinMode(infraredpin,INPUT);
  pinMode(Lamp, OUTPUT); 
    /* The queue is created to hold a maximum of 5 long values. */
    xQueue = xQueueCreate( 1, sizeof( int ) );
    xQueueldr = xQueueCreate( 1, sizeof( int ) );

  if( xQueue != NULL )
  {

    xTaskCreate( vSenderTask, "Sender1", 200, ( void * ) checkinfrared, 1, NULL );
    xTaskCreate( vSenderTaskldr, "Sender2", 200, ( void * ) checkldr, 1, NULL );


    xTaskCreate( vReceiverTask, "Receiver", 200, NULL, 1, NULL );

    vTaskStartScheduler();
  }
  else
  {

  }

  for( ;; );
//  return 0;
}
/*-----------------------------------------------------------*/

static void vSenderTask( void *pvParameters )
{
int lValueToSend;
portBASE_TYPE xStatus;

  checkinfrared = ( int ) pvParameters;
  for( ;; )
  {
    infrared();
    delay(300);
    xStatus = xQueueSendToBack( xQueue, &checkinfrared, 0 );
    if( xStatus != pdPASS )
    {
    //  vPrintString( "Could not send to the queue.\r\n" );
    }


    taskYIELD();
  }
}
/*-----------------------------------------------------------*/
static void vSenderTaskldr( void *pvParameters )
{
int lValueToSend;
portBASE_TYPE xStatusldr;

  checkldr = ( int ) pvParameters;
  for( ;; )
  {
    light();
   // delay(200);
    xStatusldr = xQueueSendToBack( xQueueldr, &checkldr, 0 );

    if( xStatusldr != pdPASS )
    {
    //  vPrintString( "Could not send to the queue.\r\n" );
    }


    taskYIELD();
  }
}
/*-----------------------------------------------------------*/
static void vReceiverTask( void *pvParameters )
{
/* Declare the variable that will hold the values received from the queue. */

portBASE_TYPE xStatus;
portBASE_TYPE xStatusldr;

const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;

  for( ;; )
  {
    //delay(200);
    if( uxQueueMessagesWaiting( xQueue ) != 0 )
    {
     // vPrintString( "Queue should have been empty!\r\n" );
    }

    xStatus = xQueueReceive( xQueue, &receiveinfrared, xTicksToWait );
    xStatusldr = xQueueReceive( xQueueldr, &receiveldr, xTicksToWait );

    //it OK
    if( xStatus == pdPASS && xStatusldr == pdPASS)
    {
      vPrintStringAndNumber( "check infrared == ", receiveinfrared );
      vPrintStringAndNumber( "check ldr == ", receiveldr );
      showlcd();
      relay();
    }
    else
    {
     // vPrintString( "Could not receive from the queue.\r\n" );
    }
  }
}
//------------------------------------------------------------------------------
void loop() {}
void infrared(){
  infraredval=digitalRead(infraredpin);
  Serial.print("Infrared digital logic: ");
  Serial.print(infraredval);
  Serial.print("  ");
  if(infraredval==LOW){ 
    checkinfrared += 1;  
    
  }else if(checkinfrared != 1 && checkinfrared != 2){
    showstatus = "OFF      ";
    checkinfrared = 0; 
  }
}
void light(){
  ldrval = analogRead(ldrpin); 
  Serial.print("LDR analog: ");
  Serial.print(ldrval);
  Serial.println("  ");
  if (ldrval <= 500) {
     checkldr = 0;
  }
  else{
     checkldr = 1;
   }  
}
void relay(){
  if (receiveldr == 1 && receiveinfrared == 1){
    digitalWrite(Lamp, HIGH);
  }else if(receiveinfrared == 2){
    digitalWrite(Lamp, HIGH);
  }else{    
    digitalWrite(Lamp, LOW);
  }
}
void showlcd(){
   if(receiveinfrared == 1){ 
       lcd.setCursor(0, 0); 
       lcd.print("Status :ON      ");
     }else if(receiveinfrared == 2){
       lcd.setCursor(0, 0); 
       lcd.print("Status :ON Always");
     }else if(receiveinfrared != 1 && receiveinfrared != 2){
       lcd.setCursor(0, 0); 
       lcd.print("Status :OFF      ");
     }
     
   if(receiveldr == 1){ 
       lcd.setCursor(0, 1); 
       lcd.print("Light :ON      ");
     }else if(receiveldr == 0){
       lcd.setCursor(0, 1); 
       lcd.print("Light :OFF");
     }
}





