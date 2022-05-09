from keras.models import Sequential
from keras.layers import LSTM
from keras.layers import Dense
from keras.layers import RepeatVector
from keras.layers import TimeDistributed
from keras.optimizers import adam_v2

SEQUENCE_LENGTH = 9
LEARNING_RATE = 0.001

model = Sequential()
model.add(LSTM(50, activation='relu', input_shape=(SEQUENCE_LENGTH, 1)))  # encode
model.add(RepeatVector(SEQUENCE_LENGTH))  # latent space
model.add(LSTM(50, activation='relu', return_sequences=True))  # decode
model.add(TimeDistributed(Dense(1)))
model.compile(optimizer=adam_v2.Adam(learning_rate=LEARNING_RATE), loss='mse')

model.save('models/lstm_unweighted')
