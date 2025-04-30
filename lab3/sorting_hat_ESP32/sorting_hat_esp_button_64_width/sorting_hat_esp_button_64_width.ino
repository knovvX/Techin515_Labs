#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "sorting_hat_model.h"  // ML Model

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button Pins
#define BUTTON_A 1  // Button for Answer A
#define BUTTON_B 2  // Button for Answer B
#define BUTTON_C 3  // Button for Answer C
#define BUTTON_D 4  // Button for Answer D

char selectedAnswer = ' ';
// Sorting Hat Questions
const char* questions[] = {
  "1. What do you value?",
  "2. What to do if someone cheats?",
  "3. Favorite subject?",
  "4. How do you face challenges?",
  "5. How do friends describe you?",
  "6. What to do with a mystery book?",
  "7. Preferred pet?",
  "8. How do you solve problems?",
  "9. What kind of friends do you like?",
  "10. Dream career?"
};

// Answer Options
const char* options[][4] = {
  { "A) Bravery", "B) Loyalty", "C) Intelligence", "D) Ambition" },
  { "A) Call them out", "B) Let them be", "C) Inform teacher", "D) Gain from it" },
  { "A) Defense Arts", "B) Herbology", "C) Charms", "D) Potions" },
  { "A) Face head-on", "B) Team up", "C) Plan first", "D) Outsmart it" },
  { "A) Bold", "B) Kind", "C) Smart", "D) Resourceful" },
  { "A) Read it now", "B) Check safety", "C) Study it", "D) Use for gain" },
  { "A) Owl", "B) Toad", "C) Cat", "D) Phoenix" },
  { "A) Act fast", "B) Find a compromise", "C) Analyze first", "D) Outsmart" },
  { "A) Adventurous", "B) Loyal", "C) Thoughtful", "D) Powerful" },
  { "A) Auror", "B) Healer", "C) Scholar", "D) Minister" }
};

// Store user responses (1-4)
int responses[10] = { 0 };
int questionIndex = 0;  // Current question index

// Initialize Machine Learning Model
Eloquent::ML::Port::DecisionTree clf;

void setup() {
  Serial.begin(115200);
  Wire.begin(5, 6);
  Serial.println("in setup");
  // Initialize OLED
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("SSD1306 allocation failed");
    for (;;)
      ;
  }
  display.clearDisplay();

  // Welcome Message
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(10, 10);
  display.println("Sorting Hat Ready!");
  display.display();
  delay(2000);

  // Initialize buttons as input
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  pinMode(BUTTON_D, INPUT_PULLUP);

  // Show the first question
  showQuestion();
  Serial.println("finish setup");
}

void loop() {
  checkButtons();  // Check for user input
}

// Function to display questions on OLED
void showQuestion() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(questions[questionIndex]);

  // Display answer choices
  for (int i = 0; i < 4; i++) {
    display.setCursor(10, 20 + (i * 10));
    display.println(options[questionIndex][i]);
  }

  display.display();
}

// Function to check button presses and record response
// Your should complete this function to handle button presses
void checkButtons() {
  // bool buttonPressed = false;

  // while (!buttonPressed) {  // Keep displaying the question until a button is pressed

  //     delay(50);  // Small delay to prevent button bouncing
  // }

  // nextQuestion();  // Move to the next question
  bool buttonPressed = false;

  while (!buttonPressed) {  
    // Keep displaying the question until a button is pressed
    if (digitalRead(BUTTON_A) == LOW) {
      delay(80);  // Button A pressed
      selectedAnswer = '1';
      buttonPressed = true;
    } else if (digitalRead(BUTTON_B) == LOW) {
      delay(80);  // Button B pressed
      selectedAnswer = '2';
      buttonPressed = true;
    } else if (digitalRead(BUTTON_C) == LOW) {
      delay(80);  // Button C pressed
      selectedAnswer = '3';
      buttonPressed = true;
    } else if (digitalRead(BUTTON_D) == LOW) {
      delay(80);  // Button D pressed
      selectedAnswer = '4';
      buttonPressed = true;
    }
    responses[questionIndex] = selectedAnswer; // save the answer
    delay(80);  // Small delay to prevent bouncing
  }
  nextQuestion();  // Move to the next question
  Serial.print("Question ");
  Serial.print(questionIndex + 1);
  Serial.print(": Selected ");
  Serial.println(selectedAnswer);
}



// Move to the next question or classify the house
void nextQuestion() {
  questionIndex++;
  if (questionIndex < 10) {
    showQuestion();  // Show next question
  } else {
    classifyHouse();  // Run ML model prediction
  }
}

// Function to classify the user into a Hogwarts house
void classifyHouse() {
  display.clearDisplay();
  display.setCursor(10, 10);
  display.println("Sorting...");

  // Convert responses into ML input format
  float features[] = {
    (float)responses[0], (float)responses[1], (float)responses[2], (float)responses[3],
    (float)responses[4], (float)responses[5], (float)responses[6], (float)responses[7],
    (float)responses[8], (float)responses[9]
  };

  int house = clf.predict(features);  // Use ML model

  // Display house result
  display.setCursor(10, 30);
  display.print("House: ");
  switch (house) {
    case 0: display.println("Gryffindor"); break;
    case 1: display.println("Hufflepuff"); break;
    case 2: display.println("Ravenclaw"); break;
    case 3: display.println("Slytherin"); break;
  }

  display.display();
  Serial.println("Sorting complete!");
  Serial.print("Predicted House: ");
  switch (house) {
    case 0: Serial.println("Gryffindor"); break;
    case 1: Serial.println("Hufflepuff"); break;
    case 2: Serial.println("Ravenclaw"); break;
    case 3: Serial.println("Slytherin"); break;
  }
}
