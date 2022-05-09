import tensorflow as tf

converter = tf.lite.TFLiteConverter.from_saved_model('models/model_trained')
converter.target_spec.supported_ops = [tf.lite.OpsSet.TFLITE_BUILTINS, tf.lite.OpsSet.SELECT_TF_OPS]
converter._experimental_lower_tensor_list_ops = False
converter.optimizations = [tf.lite.Optimize.DEFAULT]

tflite_model = converter.convert()

with open('model.tflite', 'wb') as f:
    f.write(tflite_model)