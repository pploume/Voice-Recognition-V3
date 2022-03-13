/*
A partir du fichier original dispo sur le web de ElecHouse ... mais
avec des noms de variables améliorés (en espérant que ce soit plus explicite !)
et des commentaires/ exemples qui devraient aider à comprendre

Comme Bertrans l'a souligné dans sa vidéo, le terme de 'record' n'est pas formécement très explicite.
J'utilise ici l'expression, plus longue ..., de 'commande Vocale Enregistrée' (raccourcie au besoin en cVE)
Sachant qu'il peut y avoir un maximum de 89 [0] à [79] commandes vocales enregistrées
mais
seulement 7 [0] à [6] commandes vocales actives à l'intant T au sein du Recognizer (coeur de la reconnaissance vocale)

Le cas échéant ...
Je mets en commentaire le terme/le nom original 
Je mets juste en dessous le terme/nom plus explicite que j'utilise en remplacement
Désolé si parfois c'est très long ... :)


Dernier point :
Le programme (sketch) ci-dessous fait référence à 11 commnandes Vocales Enregistrées 
Bien évidemment, pour l'utiliser il faut AU PREALABLE que l'utilisateur (vous ...) ait effectivement enregistré
au moins 11 commandes vocales à l'aide du programme 'vr_sample_train.ino'.
Le 'train' (en anglais : entrainement) est la fonction qui permet de stocker après reconnaissance/identification les commandes vocales dans la mémoire
flash de la plaquette. Il peut y en avoir un maximume de 80, de [0] à [79] ... mais 7 actives (reconnaissables/identifiables) à l'instant T


NB : pas encore reçu la plaquette, donc pas testé ... mais ça devrait le faire
:)

hope it helps

*/

/**
  ******************************************************************************
  * @file    vr_sample_multi_cmd.ino
  * @author  JiapengLi
  * @brief   This file provides a demostration on 
              how to implement a multi voice command project (exceed 7 voice command) 
              by using VoiceRecognitionModule
  ******************************************************************************
  * @note:
        voice control led
  ******************************************************************************
  * @section  HISTORY
    
    2013/06/13    Initial version.
  */
  
#include <SoftwareSerial.h>
#include "VoiceRecognitionV3.h"

/**        
  Connection
  Arduino    VoiceRecognitionModule
   2   ------->     TX
   3   ------->     RX
*/
VR myVR(2,3);    // 2:RX 3:TX, you can choose your favourite pins.

//uint8_t record[7]; // save record
uint8_t cVE_Actives[7];                                         // tablo des cVE qui seront placées (loadées) dans le Recognizer ... et donc actives, reconnaissables

uint8_t buf[64];

int led = 13;

//int group = 0;
int groupeCommandesActives = 0;                                 // par défaut, ce sera 'group0xxx' qui sera chargé dans le Recognizer

//#define switchRecord        (0)
#define cVE_DeChangementDeGroupe   (0)                          // définit le N° [0] de la cVE qui va permettre de changer de groupeCommandesActives pour passer de 'group0xxx' à 'groupe1xxx'
                                                                // dans cet exemple, se sera la son 'change'

/*
        Ce programme (sketch ...) utilise '2 ensembles de définitions' qui sont respectivement nommés 'groupe0xxxx' et 'groupe1xxxx' (remplacez 'xxxxx' par Recordx)
        Chacun de ces #define est égal au numéro d'une 'commande Vocale Enregistrée' stockée dans la mémoire de la plaquette.
        Mais comme ces cVE sont 'juste' dans la mémoire flash, elles ne sont pas actives.
        Pour les activer (c'est à dire les rendre identifiables/reconnaissable par le système), il faut les placer dans le Recognizer (le coeur du système) qui ne peut en contenir
        à l'intant T que 7 de [0] à [6].
        Mais on verra comment cela se fait plus loin dans le programme...

        IMPORTANT : vous remarquerez que chacun des ensembles (group0xxx & group1xxx) ne contient que 6 possibilités !
        En effet, la 1ère ( qui porte le n° 0 ...) DOIT ABSOLUMENT contenir le N° de la cVE qui permet de changer de groupe ... (donc le son 'change' dans cet exemple)
        sinon, impossible d'effectuer le changement ou de revenir au groupe précédent.

        Nous partirons du principe que la cVE N° 0 contient un son équivalent à 'change'

        Pour l'exemple et faciliter la compréhension (j'espère ...), nous partirons du principe que :
                - les cVE de group0xxx sont relatives à de la lumière ...
                - les cVE de group1xxx sont relatives à de la musique ...


*/
//      définition        N° de cVE                             // son de la cVE (durée max 1,5 seconde ... ça doit tenir, en parlant vite)
#define group0Record1       (1)                                 // allume plafond
#define group0Record2       (2)                                 // éteint plafond     
#define group0Record3       (3)                                 // allume escalier
#define group0Record4       (4)                                 // éteint escalier
#define group0Record5       (5)                                 // allume entrée
#define group0Record6       (6)                                 // éteint entrée

#define group1Record1       (7)                                 // monte le son
#define group1Record2       (8)                                 // baisse le son
#define group1Record3       (9)                                 // joue FranceTV
#define group1Record4       (10)                                // joue Netflix
#define group1Record5       (11)                                // allume TV
#define group1Record6       (12)                                // éteint TV

void setup()
{
  /** initialize */
  myVR.begin(9600);
  
  Serial.begin(115200);
  Serial.println("Elechouse Voice Recognition V3 Module\r\nMulti Commands sample");
  
  pinMode(led, OUTPUT);
    
  if(myVR.clear() == 0){                                        // si on réussit à effacer la mémoire du Recognizer ( ... qui contient les cVE actives)
    Serial.println("Recognizer cleared.");
  }else{
    Serial.println("Not find VoiceRecognitionModule.");
    Serial.println("Please check connection and restart Arduino.");
    while(1);
  }
  
  /*
  record[0] = switchRecord;
  record[1] = group0Record1;
  record[2] = group0Record2;
  record[3] = group0Record3;
  record[4] = group0Record4;
  record[5] = group0Record5;
  record[6] = group0Record6;
  */
  // changement du nom 'record' pour 'cVE_Actives' ... en espérant que c'est plus clair
  cVE_Active[0] = cVE_DeChangementDeGroupe;                     // en 1er (position [0], le N° de cVE qui permet de changer de groupe --> 'change' ) 
  cVE_Active[1] = group0Record1;                                // le N° de cVE qui contient le son 'allume plafond'     
  cVE_Active[2] = group0Record2;                                // le N° de cVE qui contient le son 'éteint plafond'
  cVE_Active[3] = group0Record3;                                // le N° de cVE qui contient le son 'allume escalier'
  cVE_Active[4] = group0Record4;                                // le N° de cVE qui contient le son 'éteint escalier'
  cVE_Active[5] = group0Record5;                                // le N° de cVE qui contient le son 'allume entrée'
  cVE_Active[6] = group0Record6;                                // le N° de cVE qui contient le son 'éteint entrée'


  //group = 0;
  // changement du nom 'group' pour groupeCommandesActives ... en espérant que c'est plus clair
  groupeCommandesActives = 0;                                   // par défaut, ce sont les cVE de group0xxx qui seront activées ...

/*
  if(myVR.load(record, 7) >= 0){
    printRecord(record, 7);
    Serial.println(F("loaded."));
  }
*/
  if(myVR.load(cVE_Actives, 7) >= 0){                           // ... c'est maintenant que l'on charge les cVE "Lumière" dans le Recognizer, les unes après les autres
    printRecord(cVE_Actives, 7);                                // affichage ...
    Serial.println(F("loaded."));                               // affichage ...
  }

}

void loop()
{
  int ret;
  ret = myVR.recognize(buf, 50);
  if(ret>0){                                                    // si le système a bien reconnu une cVE ... ret est supérieur à 0
    switch(buf[1]){                                             // le 2ème caractère (position [1]) de buffer contient le N° de la cVE reconnue ... donc, teste sa valeur
      //case switchRecord:                                      
      case cVE_DeChangementDeGroupe:                           // si cette valeur est égale à la cVE N° 0 (c'est à dire au son 'change' ...)
        /** turn on LED */
        if(digitalRead(led) == HIGH){                           //
          digitalWrite(led, LOW);                               //
        }else{                                                  // on change l'allumage de la led de l'Arduino ...
          digitalWrite(led, HIGH);                              // on peut simplifier par : digitalWrite(led, !digitalRead(led)); ... sauf erreur
        }                                                       //
        //if(group == 0){
        if (groupeCommandesActives == 0){                       // si les 6 (de [1] à [7]) cVE sont celles de la "Lumière"
          //group = 1;
          groupeCommandesActives = 1;                           // on change pour les cVE de la "Musique"
          myVR.clear();                                         // on efface les cVE actives dans le Recognizer
          cVE_Actives[0] = cVE_DeChangementDeGroupe;            // IMPORTANT : on place en 1er (position [0]) la cVE dont le son est 'change' !
          cVE_Actives[1] = group1Record1;                       // c.à.d : 'monte le son'
          cVE_Actives[2] = group1Record2;                       // c.à.d : 'baisse le son'   
          cVE_Actives[3] = group1Record3;                       // c.à.d : 'joue FranceTV' 
          cVE_Actives[4] = group1Record4;                       // c.à.d : 'Joue Netflix'
          cVE_Actives[5] = group1Record5;                       // c.à.d : 'allume TV'
          cVE_Actives[6] = group1Record6;                       // c.à.d : 'éteint TV'

          if(myVR.load(cVE_Actives, 7) >= 0){                   // on charge dans le Recognizer les cVE relative à la "Musique" (sauf celle en [0] ... qui est 'change' !)
            printRecord(cVE_Actives, 7);                        // affichage
            Serial.println(F("loaded."));                       // affichage
          }
          /*
          record[0] = switchRecord;                             
          record[1] = group1Record1;                            
          record[2] = group1Record2;                            
          record[3] = group1Record3;                            
          record[4] = group1Record4;
          record[5] = group1Record5;
          record[6] = group1Record6;
          
          if(myVR.load(record, 7) >= 0){                        //
            printRecord(record, 7);                             //
            Serial.println(F("loaded."));                       //
          }
          */                                                    //
        }else{                                                  // ... groupeCommandesActives est égal à 1 
          //group = 0;
          groupeCommandesActives = 0;                           // on change pour les cVE de la "Lumière"
          myVR.clear();                                         // on efface le Recognizer
          cVE_Actives[0] = cVE_DeChangementDeGroupe;            // IMPORTANT : on place en 1er (position [0]) la cVE dont le son est 'change' !
          cVE_Actives[1] = group0Record1;                       // c.à.d : 'Allume plafond'
          cVE_Actives[2] = group0Record2;                       // c.à.d : 'éteint plafond'   
          cVE_Actives[3] = group0Record3;                       // c.à.d : 'allume escalier' 
          cVE_Actives[4] = group0Record4;                       // c.à.d : 'éteint escalier'
          cVE_Actives[5] = group0Record5;                       // c.à.d : 'allume entrée'
          cVE_Actives[6] = group0Record6;                       // c.à.d : 'éteint entrée'

          if(myVR.load(cVE_Actives, 7) >= 0){                   // on charge dans le Recognizer les cVE relative à la "Lumière" (sauf celle en [0] ... qui est 'change' !)
            printRecord(cVE_Actives, 7);                        // affichage
            Serial.println(F("loaded."));                       // affichage
          }
          /*
          record[0] = switchRecord;
          record[1] = group0Record1;
          record[2] = group0Record2;
          record[3] = group0Record3;
          record[4] = group0Record4;
          record[5] = group0Record5;
          record[6] = group0Record6;
          if(myVR.load(record, 7) >= 0){
            printRecord(record, 7);
            Serial.println(F("loaded."));
          }
          */
        }
        break;

      /*
        C'est ici que seront ajoutées/traitées les cVE autres que 'change' (cVE en position [0])

        Il faudra, bien entendu, aussi effectuer un test sur la valeur de 'groupeCommandesActives' pour savoir si le n0 de cVE
        retourné dans 'ret' est relatif à la "Luière" ou à la "Musique"
        ...
        genre :
      */
      case 1:
        if (groupeCommandesActives == 0)                        // on est dans la "Lumière"
        {
                // commande vers Arduino pour 'allume le plafond'
        }
        else                                                    // on est dans la musique
        {
                // commande vers Arduino pour 'monte le son'
        }
      default:
        break;
    }
    /** voice recognized */
    printVR(buf);
  }
}

/**
  @brief   Print signature, if the character is invisible, 
           print hexible value instead.
  @param   buf     --> command length
           len     --> number of parameters
*/
void printSignature(uint8_t *buf, int len)
{
  int i;
  for(i=0; i<len; i++){
    if(buf[i]>0x19 && buf[i]<0x7F){
      Serial.write(buf[i]);
    }
    else{
      Serial.print("[");
      Serial.print(buf[i], HEX);
      Serial.print("]");
    }
  }
}

/**
  @brief   Print signature, if the character is invisible, 
           print hexible value instead.
  @param   buf  -->  VR module return value when voice is recognized.
             buf[0]  -->  Group mode(FF: None Group, 0x8n: User, 0x0n:System
             buf[1]  -->  number of record which is recognized. 
             buf[2]  -->  Recognizer index(position) value of the recognized record.
             buf[3]  -->  Signature length
             buf[4]~buf[n] --> Signature
*/
void printVR(uint8_t *buf)
{
  Serial.println("VR Index\tGroup\tRecordNum\tSignature");

  Serial.print(buf[2], DEC);
  Serial.print("\t\t");

  if(buf[0] == 0xFF){
    Serial.print("NONE");
  }
  else if(buf[0]&0x80){
    Serial.print("UG ");
    Serial.print(buf[0]&(~0x80), DEC);
  }
  else{
    Serial.print("SG ");
    Serial.print(buf[0], DEC);
  }
  Serial.print("\t");

  Serial.print(buf[1], DEC);
  Serial.print("\t\t");
  if(buf[3]>0){
    printSignature(buf+4, buf[3]);
  }
  else{
    Serial.print("NONE");
  }
//  Serial.println("\r\n");
  Serial.println();
}

void printRecord(uint8_t *buf, uint8_t len)
{
  Serial.print(F("Record: "));
  for(int i=0; i<len; i++){
    Serial.print(buf[i], DEC);
    Serial.print(", ");
  }
}
