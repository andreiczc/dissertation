from keras import layers
from keras.models import Sequential
from keras.optimizers import adam_v2

SEQUENCE_LENGTH = 4
LEARNING_RATE = 0.001

model = Sequential()
model.add(layers.Input(shape=(SEQUENCE_LENGTH, 1)))
model.add(layers.Conv1D(filters=16, kernel_size=2, activation='relu', padding='same'))
model.add(layers.MaxPool1D(pool_size=2, padding='same'))
model.add(layers.Conv1D(filters=16, kernel_size=2, activation='relu', padding='same'))
model.add(layers.UpSampling1D(2))
model.add(layers.Conv1D(filters=1, kernel_size=1, activation='relu', padding='same'))
model.compile(optimizer=adam_v2.Adam(learning_rate=LEARNING_RATE), loss='mse')

model.save('models/lstm_unweighted')
