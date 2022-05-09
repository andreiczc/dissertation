import keras.losses
import numpy as np
from keras.models import load_model

model = load_model('models/model_trained')

data = [23.5, 23.7, 23.3, 23.5, 23.1, 23.3, 23.7, 23.9, 23.5]
data = np.array(data)
data = data.reshape(1, 9)
prediction = model.predict(data)
loss = keras.losses.mse(data, prediction)
print(loss)
