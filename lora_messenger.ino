#include"TFT_eSPI.h"
#include"Free_Fonts.h"
TFT_eSPI tft;
#include <SPI.h>
#include <LoRa.h>

String message;
int message_id = 0;
String response;

String messages[9];
String messages_owner[9];
int messages_id[9];
String messages_status[9];

String previous_message_to_send;
String message_to_send;

char letters_up[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ',','.','?','!'};
char letters_low[] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', ',','.','?','!'};

boolean is_up = false;
boolean previous_is_up = false;
boolean previous_is_up_button = false;

int previous_letter_index = 0;
int letter_index = 0;

void setup() {
  //  Display
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK); //Black background
 
  // display keyboard
  tft.drawLine(0,200,319,200,TFT_WHITE);

  // set up buttons
  pinMode(WIO_5S_UP, INPUT_PULLUP);
  pinMode(WIO_5S_DOWN, INPUT_PULLUP);
  pinMode(WIO_5S_LEFT, INPUT_PULLUP);
  pinMode(WIO_5S_RIGHT, INPUT_PULLUP);
  pinMode(WIO_5S_PRESS, INPUT_PULLUP);
  pinMode(WIO_KEY_A, INPUT_PULLUP);
  pinMode(WIO_KEY_B, INPUT_PULLUP);
  pinMode(WIO_KEY_C, INPUT_PULLUP);

  //  Set up LoRa
  LoRa.setPins(D0, D1);
  
  if (!LoRa.begin(433E6)) {
    tft.drawString("Starting LoRa failed!", 70, 80);
    while (1);
  }
  
}

void loop() {
  // display keyboard
  display_keyboard(letter_index);
  // display message to send
  display_message_to_send();
  
  // put your main code here, to run repeatedly:
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    message = "";
    
    // read packet
    while (LoRa.available()) {
      message += (char)LoRa.read();
    }

    if ((String)message[0] == "#") {
      tft.setTextColor(TFT_WHITE);
      for (int i=0; i<9; i++) {
        if ((String)messages_id[i] == (String)message[1]) {
          messages_status[i] = "received";
          tft.fillCircle(5, 15*i+31, 3, TFT_GREEN);
        }
      }
    } else {
      response = "#" + (String)message[0];
      delay(500);
      LoRa.beginPacket();
      LoRa.print(response);
      LoRa.endPacket();

      delay(500);

      add_message(message.substring(1), "other");
    }
  }

  // display top
  display_top();
  // display send button
  display_send_button();

  // Buttons
  if (digitalRead(WIO_5S_UP) == LOW) {
    if (letter_index >= 24 && letter_index != 100) {
      letter_index -= 24;  
    } else if (letter_index <24) {
      letter_index = 100;
    }
    delay(200);
  }
  else if (digitalRead(WIO_5S_DOWN) == LOW) {
    if (letter_index == 100) {
      letter_index = 23;
    } else if (letter_index < 30 - 24) {
      letter_index += 24;
    } else {
      letter_index = 29;
    }
    delay(200);
  }
  else if (digitalRead(WIO_5S_LEFT) == LOW) {
    if (letter_index > 0 && letter_index != 100) {
      letter_index -= 1;
    } else if (letter_index == 0) {
      letter_index = 29;
    }
    delay(200);
  }
  else if (digitalRead(WIO_5S_RIGHT) == LOW) {
    if (letter_index < 29) {
      letter_index += 1;
    } else if (letter_index == 29) {
      letter_index = 0;
    }
    delay(200);
  }
  else if (digitalRead(WIO_5S_PRESS) == LOW) {
    if (letter_index != 100 && message_to_send.length() < 26) {
      if (is_up) {
        message_to_send += letters_up[letter_index];
      } else {
        message_to_send += letters_low[letter_index];
      }
      is_up = false;
    }

    if (letter_index == 100) {
      if (message_to_send.length() > 0) {
        add_message(message_to_send, "me");
        // send
        LoRa.beginPacket();
        LoRa.print((String) message_id + message_to_send);
        LoRa.endPacket();

        // clear
        message_to_send = "";
      }
    }
    
    delay(200);
  }
  // DEL
  if (digitalRead(WIO_KEY_A) == LOW) {
    int length = message_to_send.length();
    message_to_send.remove(length - 1, 1);
    delay(200);
  }
  // Insert SPACE
  else if (digitalRead(WIO_KEY_B) == LOW) {
    message_to_send += ' ';
    delay(200);
  }
  // CAP
  else if (digitalRead(WIO_KEY_C) == LOW) {
    is_up = !is_up;
    delay(200);
  }
   
}

// add message and display
void add_message(String message, String owner) {
  if (message.length() > 0) {
    tft.setFreeFont(FF1);
    // cover the old
    tft.setTextColor(TFT_BLACK);
    for (int i=0; i<9; i++) {
      tft.drawString(messages[i], 13, 15*i + 24);
      if (messages_status[i] == "received") {
        tft.fillCircle(5, 15*i+31, 3, TFT_BLACK);
      }
    }

    for (int i=0; i<8; i++) {
      messages[i] = messages[i+1];
      messages_owner[i] = messages_owner[i+1];
      messages_status[i] = messages_status[i+1];
      messages_id[i] = messages_id[i+1];
    }
    messages[8] = message;
    messages_owner[8] = owner;
    messages_status[8] = "";
    // add to id
    if (message_id < 8) {
      message_id ++;
    } else {
      message_id = 0;
    }
    messages_id[8] = message_id;

    for (int i=0; i<9; i++) {
      if (messages_owner[i] == "me") {
        tft.setTextColor(TFT_YELLOW);
      } else {
        tft.setTextColor(TFT_WHITE);
      }
      tft.drawString(messages[i], 13, 15*i + 24);
      if (messages_status[i] == "received") {
        tft.fillCircle(5, 15*i+31, 3, TFT_GREEN);
      }
    }
  }
}

// display send button
void display_send_button() {
  if (letter_index == 100) {
    tft.fillRect(305, 185, 15, 15, TFT_YELLOW);
  } else {
    tft.fillRect(305, 185, 15, 15, TFT_WHITE);
  }
  tft.setFreeFont(FF5);
  tft.setTextColor(TFT_BLACK);
  tft.drawChar('>', 307, 197);
}

// 
void display_top() {
  tft.setFreeFont(FM9);
  tft.setTextColor(TFT_WHITE);
  // cap
  if (is_up) {
    tft.fillRect(4, 1, 40, 18, TFT_WHITE);
    tft.setTextColor(TFT_BLACK);
    previous_is_up_button = true;
  } else {
    if (previous_is_up_button) {
      tft.fillRect(4, 1, 40, 18, TFT_BLACK);
      previous_is_up_button = false;
    }
    tft.drawRect(4, 1, 40, 18, TFT_WHITE);
  }

  tft.drawString("cap", 8, 3);
  tft.setTextColor(TFT_WHITE);


  // space
  tft.drawRect(52, 1, 40, 18, TFT_WHITE);
  tft.drawString("spc", 56, 3);

  // delete
  tft.drawRect(99, 1, 40, 18, TFT_WHITE);
  tft.drawString("del", 103, 3);
}

// 
void display_message_to_send() {
  tft.setFreeFont(FF1);
  if (previous_message_to_send.compareTo(message_to_send) != 0) {
    tft.setTextColor(TFT_BLACK);
    tft.drawString(previous_message_to_send + '_', 3, 182);
    previous_message_to_send = message_to_send;
  }

  tft.setTextColor(TFT_WHITE);
  tft.drawString(message_to_send + '_', 3, 182);
}

// 
void display_keyboard(int letter_index) {
  
  //  cursor
  // remove the previous cursor
  if (previous_letter_index != letter_index && previous_letter_index != 100) {
    if (previous_letter_index < 24) {
      tft.drawRect(previous_letter_index*13+3, 203,14,15,TFT_BLACK);
    } else {
      tft.drawRect((previous_letter_index-24)*13+3, 218, 14,15,TFT_BLACK);
    }  
  }
  previous_letter_index = letter_index;
  
  // dipslay new cursor
  if (letter_index != 100) {
    if (letter_index < 24) {
      tft.drawRect(letter_index*13+3, 203,14,15,TFT_YELLOW);
    } else {
      tft.drawRect((letter_index-24)*13+3, 218, 14,15,TFT_YELLOW);
    }
  }
  

  //  letters
  if (previous_is_up != is_up) {
    
    tft.setTextColor(TFT_BLACK);
    tft.setFreeFont(FF5);
    for (int i=0; i<30; i++) {
      if (previous_is_up) {
        if (i < 24) {
          tft.drawChar(letters_up[i], 13*i+4, 215);
        } else {
          tft.drawChar(letters_up[i], 13*(i-24)+4, 230);
        }
      } else {
        if (i < 24) {
          tft.drawChar(letters_low[i], 13*i+4, 215);
        } else {
          tft.drawChar(letters_low[i], 13*(i-24)+4, 230);
        }
      }
    }

    previous_is_up = is_up;
  }
  
  tft.setFreeFont(FF5);
  tft.setTextColor(TFT_WHITE);
  for (int i=0; i<30; i++) {
    
    if (is_up) {
      if (i < 24) {
        tft.drawChar(letters_up[i], 13*i+4, 215);
      } else {
        tft.drawChar(letters_up[i], 13*(i-24)+4, 230);
      }
    } else {
      if (i < 24) {
        tft.drawChar(letters_low[i], 13*i+4, 215);
      } else {
        tft.drawChar(letters_low[i], 13*(i-24)+4, 230);
      }
    }
  }
}
