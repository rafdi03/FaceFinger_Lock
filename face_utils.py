import face_recognition
import cv2
import os
import numpy as np
import sqlite3

def load_known_faces():
    known_encodings = []
    known_names = []
    db_path = 'absensi.db'
    if os.path.exists(db_path):
        conn = sqlite3.connect(db_path)
        c = conn.cursor()
        c.execute('SELECT nama, encoding FROM users')
        for nama, encoding_blob in c.fetchall():
            encoding = np.frombuffer(encoding_blob, dtype=np.float64)
            known_encodings.append(encoding)
            known_names.append(nama)
        conn.close()
    # Fallback ke dataset_faces jika database kosong
    if not known_encodings:
        dataset_dir = 'dataset_faces'
        if os.path.exists(dataset_dir):
            for filename in os.listdir(dataset_dir):
                if filename.endswith('.jpg') or filename.endswith('.png'):
                    image_path = os.path.join(dataset_dir, filename)
                    image = face_recognition.load_image_file(image_path)
                    encoding = face_recognition.face_encodings(image)
                    if encoding:
                        known_encodings.append(encoding[0])
                        known_names.append(os.path.splitext(filename)[0])
    return known_encodings, known_names

def recognize_face(image_path, tolerance=0.5):
    known_encodings, known_names = load_known_faces()
    if not known_encodings:
        return None
    unknown_image = face_recognition.load_image_file(image_path)
    face_locations = face_recognition.face_locations(unknown_image)
    face_encodings = face_recognition.face_encodings(unknown_image, face_locations)
    if not face_encodings:
        return None
    for face_encoding in face_encodings:
        matches = face_recognition.compare_faces(known_encodings, face_encoding, tolerance)
        face_distances = face_recognition.face_distance(known_encodings, face_encoding)
        if len(face_distances) == 0:
            return None
        best_match_index = np.argmin(face_distances)
        if matches[best_match_index]:
            return known_names[best_match_index]
    return None
