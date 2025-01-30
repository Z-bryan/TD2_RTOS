#include <Arduino.h>
#define LED 19 //LED sur GPIO 19
// handle des taches
TaskHandle_t handle_reception;
TaskHandle_t handle_clignotant;
TaskHandle_t handle_affichage;
SemaphoreHandle_t mutex;
QueueHandle_t queue;

void reception(void *)
{
    String commande;
    handle_reception = xTaskGetCurrentTaskHandle();
    while (1)
    {
        commande = Serial.readString(); // recupère la commande
        Serial.printf("%s\n", commande); // affiche la commande
        if(commande == "off")
        {
            vTaskSuspend(handle_clignotant); // suspend la tache clignotant
            digitalWrite(LED, LOW); // eteint la LED
        }
        if(commande == "on"){
            vTaskResume(handle_clignotant);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void clignotant(void *)
{
    boolean etatLED = false;
    int compteur_cligno = 0;
    handle_clignotant = xTaskGetCurrentTaskHandle();
    vTaskSuspend(handle_clignotant);
    while (1)
    {
        etatLED = !etatLED; //inverse l'etat de la led
        digitalWrite(LED, etatLED);
        compteur_cligno++;
        if(compteur_cligno == 5)
        {
            compteur_cligno = 0; // reset le compteur
            xQueueSend(queue, &compteur_cligno, portMAX_DELAY);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void affichage(void *)
{
    int compteur;
    handle_affichage = xTaskGetCurrentTaskHandle();
    while (1)
    {
        if(xQueueReceive(queue, &compteur, portMAX_DELAY))
        {
            if(mutex)
            {
                // att que le mutex est dispo
                if(xSemaphoreTake(mutex, portMAX_DELAY))
                {
                    Serial.printf("100 clignotement atteint");
                    xSemaphoreGive(mutex); // rend le mutex
                }
            }
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void setup()
{
    Serial.begin(115200);
    while (!Serial);
    pinMode(LED, OUTPUT); // LED sur GPIO 19
    xTaskCreate(reception, "reception", 10000, NULL, 1, NULL);
    xTaskCreate(clignotant, "clignotant", 10000, NULL, 1, NULL);
    xTaskCreate(affichage, "affichage", 10000, NULL, 1, NULL);
    mutex = xSemaphoreCreateMutex();
    queue = xQueueCreate(1, sizeof(int));
    vTaskDelete(NULL);
}
void loop()
{
}