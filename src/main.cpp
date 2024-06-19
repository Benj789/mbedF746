#include <mbed.h>
#include <threadLvgl.h>
#include "demos/lv_demos.h"
#include <cstdio>
#include "empreinte.h"

uint8_t getFingerprintID();
uint8_t getFingerprintEnroll();
uint8_t id = 2;

ThreadLvgl threadLvgl(30);

static DigitalOut led(LED1);
static DigitalOut gache(A0);

Adafruit_Fingerprint finger = Adafruit_Fingerprint();

// fonctions des deux textes à afficher
void enroll_callback(lv_event_t *e);
void verify_callback(lv_event_t *e);

lv_obj_t *status_label;

int main()
{
     //  gache = 0;
    //ThisThread::sleep_for(5s);
   // gache = 1;
    ThisThread::sleep_for(5s);
    gache = 0;

    threadLvgl.lock();
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x003a57), LV_PART_MAIN); //couleur de fond


    // Création du text "Fingerprint Security System"
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Fingerprint Security System");
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

   // création et configuration du bouton "Enregistrer empreinte"
    lv_obj_t *enroll_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(enroll_btn, 200, 80); // Increase button size
    lv_obj_align(enroll_btn, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_t *enroll_label = lv_label_create(enroll_btn);
    lv_label_set_text(enroll_label, "Enregistrer empreinte");
    lv_obj_center(enroll_label);
    lv_obj_add_event_cb(enroll_btn, enroll_callback, LV_EVENT_CLICKED, NULL);

   // création et configuration du bouton "Deblocage porte"
    lv_obj_t *verify_btn = lv_btn_create(lv_scr_act());
    lv_obj_set_size(verify_btn, 200, 80); 
    lv_obj_align(verify_btn, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_t *verify_label = lv_label_create(verify_btn);
    lv_label_set_text(verify_label, "Deblocage porte");
    lv_obj_center(verify_label);
    lv_obj_add_event_cb(verify_btn, verify_callback, LV_EVENT_CLICKED, NULL);

    // Création  d'un label
    status_label = lv_label_create(lv_scr_act());
    lv_label_set_text(status_label, "");
    lv_obj_set_style_text_color(status_label, lv_color_hex(0xffffff), LV_PART_MAIN);
    lv_obj_align(status_label, LV_ALIGN_BOTTOM_MID, 0, -10);

    threadLvgl.unlock();

    if (finger.verifyPassword())
    {
        printf("Found fingerprint sensor!\n");
    }
    else
    {
        printf("Did not find fingerprint sensor :(\n");
        while (1)
        {
            ThisThread::sleep_for(1ms);
        }
    }

    printf("Reading sensor parameters\n");
    finger.getParameters();
    printf("Status: 0x%02X\n", finger.status_reg);
    printf("Sys ID: 0x%02X\n", finger.system_id);
    printf("Capacity: %d\n", finger.capacity);
    printf("Security level: %d\n", finger.security_level);
    printf("Device address: %02lX\n", finger.device_addr);
    printf("Packet len: %d\n", finger.packet_len);
    printf("Baud rate: %d\n", finger.baud_rate);

    finger.getTemplateCount();

    if (finger.templateCount == 0)
    {
        printf("Sensor doesn't contain any fingerprint data. Please run the 'Enroll' example.\n");
    }
    else
    {
        printf("Sensor contains %d templates\n", finger.templateCount);
    }

    while (1)
    {
        ThisThread::sleep_for(1s);
    }
}

// fonction qui gère l'enregistrement d'une empreinte et met và jour l'étiquette
void enroll_callback(lv_event_t *e)
{
    printf("Enroll button pressed\n");
    uint8_t result = getFingerprintEnroll();
    if (result == FINGERPRINT_OK)
    {
        lv_label_set_text(status_label, "Empreinte enregistree avec succes!");
    }
    else
    {
        lv_label_set_text(status_label, "Echec de l'enregistrement de l'empreinte");
    }
}

// fonction qui  gère la vérification de l'empreinte digitale et met à jour l'étiquette
void verify_callback(lv_event_t *e)
{
    printf("Verify button pressed\n");
    uint8_t result = getFingerprintID();
    if (result == FINGERPRINT_OK)
    {
        lv_label_set_text(status_label, "Empreinte reconnue!");
        gache = 1;  // Déverouillage de la gâche
        ThisThread::sleep_for(1s);  // Déverouillé pendant 5 secondes
        gache = 0;  // Vérouillage de la gâche 
    }
    else
    {
        lv_label_set_text(status_label, "Empreinte non reconnue");
    }
}

// récupération de l'ID du doigt
uint8_t getFingerprintID()
{
    uint8_t p = finger.getImage();
    switch (p)
    {
    case FINGERPRINT_OK:
        printf("Image taken\n");
        break;
    case FINGERPRINT_NOFINGER:
        printf("No finger detected\n");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        printf("Communication error\n");
        return p;
    case FINGERPRINT_IMAGEFAIL:
        printf("Imaging error\n");
        return p;
    default:
        printf("Unknown error\n");
        return p;
    }

    p = finger.image2Tz();
    switch (p)
    {
    case FINGERPRINT_OK:
        printf("Image converted\n");
        break;
    case FINGERPRINT_IMAGEMESS:
        printf("Image too messy\n");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        printf("Communication error\n");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        printf("Could not find fingerprint features\n");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        printf("Could not find fingerprint features\n");
        return p;
    default:
        printf("Unknown error\n");
        return p;
    }

    p = finger.fingerSearch();
    if (p == FINGERPRINT_OK)
    {
        printf("Found a print match!\n");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        printf("Communication error\n");
        return p;
    }
    else if (p == FINGERPRINT_NOTFOUND)
    {
        printf("Did not find a match\n");
        return p;
    }
    else
    {
        printf("Unknown error\n");
        return p;
    }

    printf("Found ID #%d\n", finger.fingerID);
    printf(" with confidence of %d\n", finger.confidence);

    return FINGERPRINT_OK;
}

// reconnaisance de l'empreinte digital
uint8_t getFingerprintEnroll()
{
    int p = -1;
    printf("Waiting for valid finger to enroll as #%d\n", id);
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            printf("Image taken\n");
            break;
        case FINGERPRINT_NOFINGER:
            printf(".\n");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            printf("Communication error\n");
            break;
        case FINGERPRINT_IMAGEFAIL:
            printf("Imaging error\n");
            break;
        default:
            printf("Unknown error\n");
            break;
        }
    }

    p = finger.image2Tz(1);
    switch (p)
    {
    case FINGERPRINT_OK:
        printf("Image converted\n");
        break;
    case FINGERPRINT_IMAGEMESS:
        printf("Image too messy\n");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        printf("Communication error\n");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        printf("Could not find fingerprint features\n");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        printf("Could not find fingerprint features\n");
        return p;
    default:
        printf("Unknown error\n");
        return p;
    }

    printf("Remove finger\n");
    ThisThread::sleep_for(2s);
    p = 0;
    while (p != FINGERPRINT_NOFINGER)
    {
        p = finger.getImage();
    }
    printf("ID %d\n", id);
    p = -1;
    printf("Place same finger again\n");
    while (p != FINGERPRINT_OK)
    {
        p = finger.getImage();
        switch (p)
        {
        case FINGERPRINT_OK:
            printf("Image taken\n");
            break;
        case FINGERPRINT_NOFINGER:
            printf(".\n");
            break;
        case FINGERPRINT_PACKETRECIEVEERR:
            printf("Communication error\n");
            break;
        case FINGERPRINT_IMAGEFAIL:
            printf("Imaging error\n");
            break;
        default:
            printf("Unknown error\n");
            break;
        }
    }


    p = finger.image2Tz(2);
    switch (p)
    {
    case FINGERPRINT_OK:
        printf("Image converted\n");
        break;
    case FINGERPRINT_IMAGEMESS:
        printf("Image too messy\n");
        return p;
    case FINGERPRINT_PACKETRECIEVEERR:
        printf("Communication error\n");
        return p;
    case FINGERPRINT_FEATUREFAIL:
        printf("Could not find fingerprint features\n");
        return p;
    case FINGERPRINT_INVALIDIMAGE:
        printf("Could not find fingerprint features\n");
        return p;
    default:
        printf("Unknown error\n");
        return p;
    }

    printf("Creating model for #%d\n", id);

    p = finger.createModel();
    if (p == FINGERPRINT_OK)
    {
        printf("Prints matched!\n");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        printf("Communication error\n");
        return p;
    }
    else if (p == FINGERPRINT_ENROLLMISMATCH)
    {
        printf("Fingerprints did not match\n");
        return p;
    }
    else
    {
        printf("Unknown error\n");
        return p;
    }

    printf("ID %d\n", id);
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK)
    {
        printf("Stored!\n");
    }
    else if (p == FINGERPRINT_PACKETRECIEVEERR)
    {
        printf("Communication error\n");
        return p;
    }
    else if (p == FINGERPRINT_BADLOCATION)
    {
        printf("Could not store in that location\n");
        return p;
    }
    else if (p == FINGERPRINT_FLASHERR)
    {
        printf("Error writing to flash\n");
        return p;
    }
    else
    {
        printf("Unknown error\n");
        return p;
    }

    return FINGERPRINT_OK;
}
