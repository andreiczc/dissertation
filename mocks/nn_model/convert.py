import tensorflow as tf

converter = tf.lite.TFLiteConverter.from_saved_model('models/model_trained')
converter.optimizations = [tf.lite.Optimize.DEFAULT]

tflite_model = converter.convert()

with open('model.tflite', 'wb') as f:
    f.write(tflite_model)
