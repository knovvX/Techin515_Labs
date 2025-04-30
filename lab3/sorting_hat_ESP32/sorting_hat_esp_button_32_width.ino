#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "sorting_hat_model.h"  // ML Model

// OLED Configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Button Pins
#define BUTTON_A  4   // Button for Answer A
#define BUTTON_B  16  // Button for Answer B
#define BUTTON_C  17  // Button for Answer C
#define BUTTON_D  18  // Button for Answer D

const int buttons[] = {BUTTON_A, BUTTON_B, BUTTON_C, BUTTON_D};
const int buttonLabel[] = {1, 2, 3, 4};
const int numButtons = 4;

int buttonStates[numButtons];
int lastButtonStates[numButtons];
unsigned long lastDebounceTimes[numButtons];
const unsigned long debounceDelay = 50;

// Store user responses (1-4)
int responses[10] = {0};
int questionIndex = 0;

unsigned long lastScrollTime = 0;
const unsigned long scrollInterval = 800;
int scrollOffset = 0;
bool scrollingDown = true;

bool inQuestionPhase = true;
bool quizFinished = false;

// Initialize Machine Learning Model
Eloquent::ML::Port::DecisionTree clf;

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
    "9. Preferred Friend Quality?",
    "10. Dream career?"
};
// Answer Options
const char* options[][4] = {
    {"A) Bravery", "B) Loyalty", "C) Intelligence", "D) Ambition"},
    {"A) Call them out", "B) Let them be", "C) Inform teacher", "D) Gain from it"},
    {"A) Defense Arts", "B) Herbology", "C) Charms", "D) Potions"},
    {"A) Face head-on", "B) Team up", "C) Plan first", "D) Outsmart it"},
    {"A) Bold", "B) Kind", "C) Smart", "D) Resourceful"},
    {"A) Read it now", "B) Check safety", "C) Study it", "D) Use for gain"},
    {"A) Owl", "B) Toad", "C) Cat", "D) Phoenix"},
    {"A) Act fast", "B) Find compromise", "C) Analyze first", "D) Outsmart"},
    {"A) Adventurous", "B) Loyal", "C) Thoughtful", "D) Powerful"},
    {"A) Auror", "B) Healer", "C) Scholar", "D) Minister"}
};



void setup() {
    Serial.begin(115200);

    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("SSD1306 allocation failed");
        for (;;);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(10, 10);
    display.println("Sorting Hat Ready!");
    display.display();
    delay(2000);

    for (int i = 0; i < numButtons; i++) {
        pinMode(buttons[i], INPUT_PULLUP);
        buttonStates[i] = HIGH;
        lastButtonStates[i] = HIGH;
        lastDebounceTimes[i] = 0;
    }

    showQuestion();
}

void loop() {
    
    if (inQuestionPhase && !quizFinished) {
        scrollAnswers();
    }
    
    checkButtons();

}

void showQuestion() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(questions[questionIndex]);

    for (int i = 0; i < 2; i++) {
        int optionIndex = scrollOffset + i;
        if (optionIndex < 4) {
            display.setCursor(10, 15 + (i * 10));
            display.println(options[questionIndex][optionIndex]);
        }
    }

    display.display();
}

void scrollAnswers() {
    if (millis() - lastScrollTime >= scrollInterval) {
        lastScrollTime = millis();

        if (scrollingDown) {
            scrollOffset++;
            if (scrollOffset > 2) {
                scrollOffset = 2;
                scrollingDown = false;
            }
        } else {
            scrollOffset--;
            if (scrollOffset < 0) {
                scrollOffset = 0;
                scrollingDown = true;
            }
        }

        showQuestion();
    }
}

// Function to check button presses and record response
// Your should complete this function to handle button presses
void checkButtons() {
    bool buttonPressed = false; // Track if any button was pressed during this check

    for (int i = 0; i < numButtons; i++) {
        int reading = digitalRead(buttons[i]); // Read the current state of button i
        
        // If the reading changed from the last time, reset the debounce timer
        if (reading != lastButtonStates[i]) {
            lastDebounceTimes[i] = millis();
        }

        // Only act if enough time has passed (debounceDelay) since the last state change
        if ((millis() - lastDebounceTimes[i]) > debounceDelay) {
            if (reading != buttonStates[i]) { // If the button state has changed
                buttonStates[i] = reading; // Save the new stable state

                if (buttonStates[i] == LOW) { // If the button is pressed (LOW because of INPUT_PULLUP)
                     // Save the user's answer
                    
                    // Note that a button was pressed
                }
            }
        }

        lastButtonStates[i] = reading; // Save the reading for next loop
    }

    if (buttonPressed) { // If any button was pressed (and debounced), move to next question
        
    }
}

void nextQuestion() {
    questionIndex++;
    scrollOffset = 0;
    scrollingDown = true;

    if (questionIndex < 10) {
        showQuestion();
    } else {
        inQuestionPhase = false;  
        delay(50);
        display.clearDisplay();
        classifyHouse();
    }
}

void classifyHouse() {
    display.clearDisplay();
    display.setCursor(10, 10);
    display.print("Sorting");
    display.display();
    for (int i = 0; i < 3; i++){
        delay(500);
        display.print(".");
        display.display();
    }

    float features[] = {
        (float)responses[0], (float)responses[1], (float)responses[2], (float)responses[3],
        (float)responses[4], (float)responses[5], (float)responses[6], (float)responses[7],
        (float)responses[8], (float)responses[9]
    };

    int house = clf.predict(features);
    
    delay(500);
    display.clearDisplay();
    display.setCursor(10, 10);
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

    quizFinished = true;
}
