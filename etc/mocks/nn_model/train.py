import os
import csv
import numpy as np
from keras.models import load_model
from sklearn import model_selection

os.environ['TF_CPP_MIN_LOG_LEVEL'] = '3'
input_data = []

with open('data.csv') as file:
    reader = csv.reader(file)
    for line in reader:
        input_data.append(np.asarray(line, float))

model = load_model('models/lstm_unweighted')
in_data = np.array(input_data)
train_data, test_data = model_selection.train_test_split(in_data, test_size=0.2, random_state=25)

print('Samples: {}'.format(train_data.shape[0]))
print('Features: {}'.format(train_data.shape[1]))

train_data = np.reshape(train_data, (train_data.shape[0], train_data.shape[1], 1))
history = model.fit(train_data, train_data, epochs=300, verbose=0, validation_split=0.1)

print('History: ', history.history)

model.save('models/model_trained')

results = model.evaluate(test_data, test_data)
print('Evaluation: ', results)
