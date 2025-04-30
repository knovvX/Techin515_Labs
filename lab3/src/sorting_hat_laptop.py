# Import necessary libraries
from sklearn.model_selection import train_test_split
from sklearn.tree import DecisionTreeClassifier
from sklearn.preprocessing import OrdinalEncoder
from micromlgen import port
import pandas as pd

# Load dataset
df = pd.read_csv("Sorting_Dataset.csv")

# Prepare dataset
X = df.drop(columns=["House"])  # Features
y = df["House"].astype('category').cat.codes  # Convert House names to numeric labels

# Split into training and testing sets
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
print("before training")
# Train a Decision Tree model
encoder = OrdinalEncoder()
X_train = encoder.fit_transform(X_train)
X_test = encoder.transform(X_test)
model = DecisionTreeClassifier(max_depth=8, random_state=42)
model.fit(X_train, y_train)

# Evaluate model accuracy
accuracy = model.score(X_test, y_test)
print(f"Model Accuracy: {accuracy:.2f}")

# Convert trained model to C++ for ESP32
c_code = port(model)

# Save C++ model to a file for ESP32 deployment
model_filename = "sorting_hat_model.h"
with open(model_filename, "w") as f:
    f.write(c_code)

print(f"Model successfully converted and saved as {model_filename}!")
