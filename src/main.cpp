#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>

#define NUMPIXELS  64  // Number of LEDs
#define NEOPIN 52       // Teensy 3.1 pin for NEOPIXELS
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIN, NEO_GRB + NEO_KHZ800);

#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 128
#define OLED_SDA 20
#define OLED_SCL 21

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 lcd(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Defines for chess pieces and spaces
#define BORDER 255
#define EMPTY 0
#define BLACK_PAWN 1
#define BLACK_KNIGHT 3
#define BLACK_BISHOP 4
#define BLACK_ROOK 5
#define BLACK_QUEEN 9
#define BLACK_KING 16
#define WHITE_PAWN 129
#define WHITE_KNIGHT 131
#define WHITE_BISHOP 132
#define WHITE_ROOK 133
#define WHITE_QUEEN 137
#define WHITE_KING 144

String piece_names[16] = {
  "white pawn",
  "white rook",
  "white knight" ,
  "white bishop",
  "white queen",
  "white king",
  "black pawn",
  "black rook",
  "black knight" ,
  "black bishop",
  "black queen",
  "black king"
};

// Defines for colors
#define RED 1
#define ORANGE 2
#define YELLOW 3
#define GREEN 4
#define CYAN 5
#define BLUE 6
#define VIOLET 7
#define MAGENTA 8

// Define the size of the chessboard
const int boardSize = 8;

// byte piecesHist[8][8] =    {{1, 1, 1, 1, 1, 1, 1, 1}, // [y][x] Previous board before last move
//                             {1, 1, 1, 1, 1, 1, 1, 1},
//                             {0, 0, 0, 0, 0, 0, 0, 0},
//                             {0, 0, 0, 0, 0, 0, 0, 0},
//                             {0, 0, 0, 0, 0, 0, 0, 0},
//                             {0, 0, 0, 0, 0, 0, 0, 0},
//                             {1, 1, 1, 1, 1, 1, 1, 1},
//                             {1, 1, 1, 1, 1, 1, 1, 1}};

byte piecesCurrent[8][8] ={0}; // current matrix being read
byte piecesPrevious[8][8] ={0}; // to compare with current matrix

// Define the chessboard squares and piece positions
char chessboard[boardSize][boardSize] = {
  {'R', 'N', 'B', 'Q', 'K', 'B', 'N', 'R'},
  {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
  {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}
};

char currentBoardLayout[8][8] = {
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
  {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '}
};

// black, red, orange, yellow, green, cyan, blue, violet, magenta
uint32_t colors[9] = { pixels.Color(0, 0, 0),     pixels.Color(255, 0, 0),   pixels.Color(255, 60, 0),
                       pixels.Color(255, 160, 0), pixels.Color(0, 255, 0),   pixels.Color(0, 255, 255), 
                       pixels.Color(0, 0, 255),   pixels.Color(148, 0, 211), pixels.Color(255, 0, 255)};


// set up LED control matrixes - control in groups of 8
int leds[8][8] = { // each LED for each row
  {0,1,2,3,4,5,6,7},
  {15,14,13,12,11,10,9,8 },// reverse-mapped
  {16,17,18,19,20,21,22,23},
  {31,30,29,28,27,26,25,24},// reverse-mapped
  {32,33,34,35,36,37,38,39},
  {47,46,45,44,43,42,41,40}, // reverse-mapped
  {48,49,50,51,52,53,54,55},
  {63,62,61,60,59,58,57,56} // reverse-mapped
};

/******************************************************/
// Read a row of 8 reed switches
int cols[] = {9,8,7,6,22,4,3,2}; // hold the column pins 
int rows[] = {17,16,15,14,13,12,11,10};
unsigned long previous_time = 0;
unsigned long debounce_delay = 200;
bool switch_state;
int count = 0;
String piece_name = "";

void showPieceMovingOLED(String piece){
  // show which piece was lifted
  lcd.setTextColor(SSD1306_WHITE);
  lcd.setTextSize(1);

  lcd.fillRect(70, 0, 60, 64, SSD1306_BLACK);

  lcd.setCursor(70, 20);
  lcd.print(piece); 
  lcd.setCursor(70, 30);
  lcd.print(" Moving");
  lcd.display();

}


// Color a chessboard with blue (for black) and red squares
void showStandBoard(){

  for(int x=0; x<8; x++){
    // for even rows
    if(x %2 == 0){
      for(int y=0; y<8; y++){
        if(y%2 == 0){
          pixels.setPixelColor(leds[x][y], colors[GREEN]); // for BLACK squares
        } else {
          pixels.setPixelColor(leds[x][y], colors[YELLOW]); // for WHITE squares
        }
      }
    } else { // for odd rows
      for(int y=0; y<8; y++){
        if(y%2 == 0){
          pixels.setPixelColor(leds[x][y], colors[YELLOW]); // for white squares
        } else {
          pixels.setPixelColor(leds[x][y], colors[GREEN]); // for black squares
        }
      }

    }

  }

  pixels.show();
}


void compareArray(){
  for(int j=0; j<8; j++){ // row
    for(int k=0; k<8; k++){ // col
      if(piecesCurrent[j][k] != piecesPrevious[j][k]){
        // that piece has changed - rather been moved
        int changed_row = j;
        int changed_col = k;
        Serial.print("Changed row: "); Serial.print(changed_row);
        Serial.print("Changed col: "); Serial.print(changed_col);
        Serial.println();

        // find out which piece was placed at that position
        char piece_placed = chessboard[j][k];
        Serial.println("Piece: " + piece_placed);
        
        delay(1500);

      }
    }
  }

  Serial.println("Current piece array");
  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 8; j++){
      Serial.print(piecesCurrent[i][j]);
    }

    Serial.println();
  }

  Serial.println("Previous piece array");
  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 8; j++){
      Serial.print(piecesPrevious[i][j]);
    }

    Serial.println();
  }

  delay(100);

  for (int i = 0; i < 8; i++){
    for (int j = 0; j < 8; j++){
      char pieceName = chessboard[i][j];
      Serial.println(pieceName);
    }
  
  }
  
}

void playPawn(String side){
  showStandBoard();
  pixels.setBrightness(255);
  if(side == "White"){
    
    pixels.setPixelColor(16, BLUE);
    pixels.setPixelColor(31, BLUE);
    pixels.show();
  } else {
    
    pixels.setPixelColor(47, BLUE);
    pixels.setPixelColor(32, BLUE);
    pixels.show();
  } 
}

void playRook(String side){
  showStandBoard();
  pixels.setBrightness(255);
  if(side == "White"){

    pixels.setPixelColor(1, MAGENTA);
    pixels.setPixelColor(15, MAGENTA);
    pixels.show();
  } else {
    
    pixels.setPixelColor(48, BLUE);
    pixels.setPixelColor(62, BLUE);
    pixels.show();
  } 
}

void playKnight(String side){
  showStandBoard();
  pixels.setBrightness(255);
  if(side == "White"){

    pixels.setPixelColor(1, RED);
    pixels.setPixelColor(15, RED);
    pixels.show();
  } else {
    
    pixels.setPixelColor(48, RED);
    pixels.setPixelColor(62, RED);
    pixels.show();
  } 
}

void playBishop(String side){
  // valid moves for bishop
  showStandBoard();
  pixels.setBrightness(255);

  if(side == "White"){

    pixels.setPixelColor(3, CYAN);
    pixels.setPixelColor(14, CYAN); // TODO: confirm
    pixels.show();
  } else {
    
    pixels.setPixelColor(50, RED);
    pixels.setPixelColor(52, RED);
    pixels.show();
  } 
}

void dump(){

   for(int j=0; j<8; j++){ // row
            for(int k=0; k<8; k++){ // col

              //to know a piece has been lifted, we either check if the count decreases by one
              // or compare the current and previos matrices for a difference
              for (int i = 0; i < 8; i++){ // rows
                for (int m = 0; m < 8; m++){ // cols
                  // check what piece has changed

                  // if the current square value is not the same as previous one
                  if(piecesCurrent[i][m] != piecesPrevious[i][m]){
                    Serial.println("Piece lifted");

                    // get which piece was lifted - from the initial chess board
                    char piece = chessboard[i][m];
                    currentBoardLayout[i][m] = piece;
                    for(int n = 0; n <8; n++){
                      for(int g =0; g< 8; g++){
                        Serial.print(currentBoardLayout[n][g]); Serial.print("  "); 
                      }
                      Serial.println();
                    }

                    //================calculate piece possible moves================

                    switch (piece){
                      case 'p': // pawn
                        // light the LEDs for forward and ff
                        piece_name = piece_names[0];
                        playPawn("white");
                        showPieceMovingOLED(piece_name);
                        
                        break;

                      case 'r': // rook
                        piece_name = piece_names[1];
                        playRook("white");
                        showPieceMovingOLED(piece_name);
                        break;

                      case 'n': // knight
                        piece_name = piece_names[1];
                        playKnight("white");
                        showPieceMovingOLED(piece_name);
                        break;

                      case 'b': // bishop
                        piece_name = piece_names[1];
                        playBishop("white");
                        showPieceMovingOLED(piece_name);
                        break;

                      case 'q': // queen
                        piece_name = piece_names[1];
                        showPieceMovingOLED(piece_name);
                        break;

                      case 'k': // king
                        piece_name = piece_names[1];
                        playKnight("white");
                        showPieceMovingOLED(piece_name);
                        break;

                        // BLACK SIDE

                      case 'P': // black pawn
                        piece_name = piece_names[6];
                        playPawn("black");
                        showPieceMovingOLED(piece_name);
                        break;

                      case 'R': // black rook
                        piece_name = piece_names[7];
                        playRook("black");
                        // playBishop("black");
                        showPieceMovingOLED(piece_name);
                        break;

                      case 'N': // black night
                        piece_name = piece_names[8];
                        playKnight("black");
                        showPieceMovingOLED(piece_name);
                        break;

                      case 'B': // black bishop
                        piece_name = piece_names[9];
                        playBishop("black");
                        showPieceMovingOLED(piece_name);
                        break;

                      case 'Q': // black queen
                        piece_name = piece_names[10];
                        showPieceMovingOLED(piece_name);
                        break;

                      case 'K': // black king
                        piece_name = piece_names[10];
                        showPieceMovingOLED(piece_name);
                        break;

                      
                      default:
                      break;
                    }


                    //=================end of piece possible moves==================

                  } else {
                    showPieceMovingOLED("none");
                    continue;
                  }
                  
                }
                
              }
            
            }
          }

}

void printCurrentMatrix(){
  for(int row_index=0; row_index<8; row_index++){
    if(row_index < 10){
      Serial.print(F("0"));
    }
    Serial.print(row_index); Serial.print(F(":"));

    // check each column
    for(int col_index=0; col_index<8; col_index++){
      Serial.print(piecesCurrent[row_index][col_index], DEC);
      if(col_index < 8){
        Serial.print(F(", "));
      }
    }
    Serial.println();
  }
  Serial.println();

}

int readMatrix(){
  Serial.println("calling");
  count = 0;
  
  for(int row = 0; row < 8; row++){
    pinMode(rows[row], OUTPUT); // set the row to be an output
    digitalWrite(rows[row], LOW); // program the row to be LOW, everything else to be an input

    for(int col = 0; col < 8; col++){
      
        //------------------------ no debounce - this piece works better-------------------
        switch_state = digitalRead(cols[col]); // read the column, if reads LOW, then it is pressed
        if(switch_state == false){

          piecesCurrent[row][col] = !switch_state; // when a piece is placed on a square, it pulls that column to ground, invert to 
          count += !switch_state;
         
        } 
        //------------------------- end of no-debounce ------------

    } // end of for each column

    pinMode(rows[row], INPUT); // set that row back to input
  } // end of for each row 

  return count;
}

 
void findMoves(){
  for(int row_index=0; row_index<8; row_index++){
    if(row_index < 10){
      Serial.print(F("0"));
    }
    Serial.print(row_index); Serial.print(F(":"));

    // check each column
    for(int col_index=0; col_index<8; col_index++){
      // Serial.print(piecesCurrent[row_index][col_index], DEC);
      if(piecesCurrent[row_index][col_index] != piecesPrevious[row_index][col_index]){

        // a piece has been lifted
        Serial.println("Piece lifted");


      } else {
        // no piece lifted
        Serial.println("No change");
      }

      // if(col_index < 8){
      //   Serial.print(F(", "));
      // }
    }
    // Serial.println();
  }
  // Serial.println();

}


void drawChessboard() {
  int squareSize = SCREEN_HEIGHT / boardSize;

  for (int y = 0; y < boardSize; y++) {
    for (int x = 0; x < boardSize; x++) {
      if ((x + y) % 2 == 0) {
        lcd.fillRect(x * squareSize, y * squareSize, squareSize, squareSize, BLACK);
      } else {
        lcd.fillRect(x * squareSize, y * squareSize, squareSize, squareSize,MAGENTA);
      }
      lcd.setCursor(x * squareSize + 3, y * squareSize + 3);
      lcd.setTextSize(1);
      lcd.setTextColor(SSD1306_BLACK);
      lcd.print(chessboard[y][x]);
    }
  }
}

void showCount(int count){
  lcd.setTextColor(SSD1306_WHITE);
  lcd.setCursor(70, 10);
  lcd.print("Total");
  lcd.setCursor(70, 20);
  lcd.print("Pieces");
  lcd.setTextSize(2);
  lcd.setCursor(70, 35);
  lcd.print(count);
  lcd.display();
}

void showMove(){
  //  

}

void initialize_board(){
  // set the pieces on their initial positions


}


void setup() {
  Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!lcd.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  Serial.println("Display found");

  // Clear the buffer
  lcd.clearDisplay();
  lcd.setTextColor(SSD1306_WHITE);

  lcd.setCursor(0, 0);
  lcd.print("Smart Chessboard");
  lcd.display();

  // These lines read the columns set up column pins as INPUTS WITH PULLUP
  for(int i=0; i<8; i++){
    pinMode(cols[i], INPUT_PULLUP); // set columnn as input, always sees HIGH
  }

  pinMode(52, OUTPUT);        // Drives the RGB LED strip
  pinMode(13, OUTPUT); 

  pixels.begin();             // This initializes the NeoPixel library.
  pixels.setBrightness(200);    // Brightness can be set from 0 - 255.

  showStandBoard();
  delay(2000);
  drawChessboard();
  delay(2000);
  
}

void loop() {
  showStandBoard();
  delay(100);
  // read the matrix
  readMatrix();

  // findMoves();
  printCurrentMatrix();

  dump();

  // copy the current matrix into previous one
//  for(int row_index=0; row_index<8; row_index++){
//     if(row_index < 10){
//       Serial.print(F("0"));
//     }
//     Serial.print(row_index); Serial.print(F(":"));

//     // check each column
//     for(int col_index=0; col_index<8; col_index++){
//       piecesPrevious[row_index][col_index] = piecesCurrent[row_index][col_index];
      
//     }

//   }


  lcd.clearDisplay();
  drawChessboard();
  lcd.display();

  showCount(count);

  delay(2000);
}

