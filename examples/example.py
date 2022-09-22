import wave
import sonic
import librosa
import numpy as np


def save_wav(filename, samples, framerate=22050):
    with wave.open(filename, "w") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(framerate)
        f.writeframes(samples.tobytes())


ps = sonic.Sonic(22050, 1)
ps.set_speed(2.0)
ps.set_volume(0.5)
ps.set_pitch(1.5)
# ps.set_quality(1)

samples, _ = librosa.load('test.wav')
samples = (32767 * samples).astype(np.int16)
processed_samples = None
chunk_length = 6400
n = len(samples) // chunk_length
remaining = len(samples) - n * chunk_length
for i in range(n):
    chunk_samples = samples[i * chunk_length:i * chunk_length + chunk_length]
    ps.write_short(chunk_samples.tobytes())
    processed_chunk_buffer = b""
    while ps.samples_available() > 0:
        processed_chunk_buffer += ps.read_short(chunk_length)
    processed_chunk_samples = np.frombuffer(processed_chunk_buffer, dtype=np.int16)
    if processed_samples is None:
        processed_samples = processed_chunk_samples
    else:
        processed_samples = np.concatenate((processed_samples, processed_chunk_samples))

ps.flush()
print("available samples: ", ps.samples_available())
processed_chunk_buffer = b""
while ps.samples_available() > 0:
    processed_chunk_buffer += ps.read_short(chunk_length)
if len(processed_chunk_buffer) > 0:
    processed_chunk_samples = np.frombuffer(processed_chunk_buffer, dtype=np.int16)
    processed_samples = np.concatenate((processed_samples, processed_chunk_samples))
print("writing result into out.wav...")
save_wav('out.wav', processed_samples)
