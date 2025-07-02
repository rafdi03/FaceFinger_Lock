from flask import Flask, request, jsonify, send_from_directory, render_template
from datetime import datetime
import os
import sqlite3
from face_utils import recognize_face
import base64
import face_recognition
from flask_socketio import SocketIO, emit
import shutil

app = Flask(__name__)
socketio = SocketIO(app)
UPLOAD_FOLDER = 'uploads'
REGISTER_FOLDER = 'register'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
os.makedirs(REGISTER_FOLDER, exist_ok=True)

DB_PATH = 'absensi.db'

def catat_absensi(nama):
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    # Cari user_id berdasarkan nama
    c.execute('SELECT id FROM users WHERE nama=?', (nama,))
    row = c.fetchone()
    if row:
        user_id = row[0]
        c.execute('INSERT INTO absensi (user_id) VALUES (?)', (user_id,))
        conn.commit()
        conn.close()
        return True
    conn.close()
    return False

@app.route('/upload', methods=['POST'])
def upload_image():
    # Ambil data gambar dari body
    img_data = request.data

    if not img_data:
        return jsonify({'status': 'error', 'message': 'No image data'}), 400

    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"{timestamp}.jpg"
    filepath = os.path.join(UPLOAD_FOLDER, filename)

    # Simpan data ke file
    with open(filepath, 'wb') as f:
        f.write(img_data)

    print(f"[INFO] Gambar disimpan: {filename}")
    return jsonify({'status': 'success', 'filename': filename}), 200

@app.route('/absen', methods=['POST'])
def absen():
    img_data = request.data
    if not img_data:
        return jsonify({'status': 'error', 'message': 'No image data'}), 400
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    filename = f"absen_{timestamp}.jpg"
    filepath = os.path.join(UPLOAD_FOLDER, filename)
    with open(filepath, 'wb') as f:
        f.write(img_data)
    # Deteksi wajah
    nama = recognize_face(filepath)
    if nama:
        if catat_absensi(nama):
            # Kirim notifikasi ke client
            socketio.emit('absensi_baru', {'nama': nama, 'waktu': timestamp})
            # Simpan ke dataset_faces
            dataset_dir = 'dataset_faces'
            os.makedirs(dataset_dir, exist_ok=True)
            new_filename = f"{nama}_{timestamp}.jpg"
            new_filepath = os.path.join(dataset_dir, new_filename)
            shutil.copy(filepath, new_filepath)
            # Cari user_id
            conn = sqlite3.connect(DB_PATH)
            c = conn.cursor()
            c.execute('SELECT id FROM users WHERE nama=?', (nama,))
            row = c.fetchone()
            user_id = row[0] if row else None
            conn.close()
            return jsonify({'status': 'success', 'user_id': user_id, 'nama': nama, 'message': f'Absensi berhasil untuk {nama}'}), 200
        else:
            return jsonify({'status': 'error', 'message': 'User tidak ditemukan di database'}), 404
    else:
        # Simpan ke tabel unknown_faces
        conn = sqlite3.connect(DB_PATH)
        c = conn.cursor()
        c.execute('INSERT INTO unknown_faces (filename) VALUES (?)', (filename,))
        conn.commit()
        conn.close()
        return jsonify({'status': 'error', 'message': 'Wajah tidak dikenali, silakan daftar user baru'}), 404

@app.route('/register', methods=['POST'])
def register():
    # Data: nama, image (base64 atau raw)
    nama = request.form.get('nama')
    if 'image' in request.files:
        image_file = request.files['image']
        image_path = os.path.join(REGISTER_FOLDER, f'reg_{nama}.jpg')
        image_file.save(image_path)
    elif 'image' in request.form:
        # Jika image dikirim base64
        img_data = base64.b64decode(request.form['image'])
        image_path = os.path.join(REGISTER_FOLDER, f'reg_{nama}.jpg')
        with open(image_path, 'wb') as f:
            f.write(img_data)
    else:
        return jsonify({'status': 'error', 'message': 'No image data'}), 400
    # Encode wajah
    image = face_recognition.load_image_file(image_path)
    encodings = face_recognition.face_encodings(image)
    if not encodings:
        return jsonify({'status': 'error', 'message': 'Wajah tidak terdeteksi. Pastikan wajah terlihat jelas, tidak buram, dan cukup cahaya.'}), 400
    encoding = encodings[0]
    # Simpan ke database
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute('INSERT INTO users (nama, encoding) VALUES (?, ?)', (nama, encoding.tobytes()))
    conn.commit()
    conn.close()
    return jsonify({'status': 'success', 'message': f'User {nama} terdaftar'}), 200

@app.route('/absensi', methods=['GET'])
def lihat_absensi():
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute('SELECT u.id, u.nama, a.waktu FROM absensi a JOIN users u ON a.user_id = u.id ORDER BY a.waktu DESC')
    data = [{'user_id': row[0], 'nama': row[1], 'waktu': row[2]} for row in c.fetchall()]
    conn.close()
    return jsonify(data)

@app.route('/users', methods=['GET'])
def list_users():
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute('SELECT id, nama FROM users')
    data = [{'id': row[0], 'nama': row[1]} for row in c.fetchall()]
    conn.close()
    return jsonify(data)

@app.route('/uploads/<filename>')
def uploaded_file(filename):
    return send_from_directory(UPLOAD_FOLDER, filename)

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/absen-ui')
def absen_ui():
    return render_template('absen.html')

@app.route('/register-ui')
def register_ui():
    return render_template('register.html')

@app.route('/absensi-ui')
def absensi_ui():
    return render_template('absensi.html')

@app.route('/delete-user', methods=['POST'])
def delete_user():
    user_id = request.form.get('id')
    if not user_id:
        return jsonify({'status': 'error', 'message': 'ID user diperlukan'}), 400
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    # Ambil nama user untuk hapus file foto
    c.execute('SELECT nama FROM users WHERE id=?', (user_id,))
    row = c.fetchone()
    if not row:
        conn.close()
        return jsonify({'status': 'error', 'message': 'User tidak ditemukan'}), 404
    nama = row[0]
    c.execute('DELETE FROM users WHERE id=?', (user_id,))
    conn.commit()
    conn.close()
    # Hapus file foto jika ada
    image_path = os.path.join(REGISTER_FOLDER, f'reg_{nama}.jpg')
    if os.path.exists(image_path):
        os.remove(image_path)
    return jsonify({'status': 'success', 'message': f'User {nama} berhasil dihapus'})

@app.route('/reset-unknown-faces', methods=['POST'])
def reset_unknown_faces():
    # Ambil semua filename dari tabel
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute('SELECT filename FROM unknown_faces')
    files = [row[0] for row in c.fetchall()]
    # Hapus file di uploads
    for filename in files:
        file_path = os.path.join(UPLOAD_FOLDER, filename)
        if os.path.exists(file_path):
            os.remove(file_path)
    # Hapus semua data di tabel
    c.execute('DELETE FROM unknown_faces')
    conn.commit()
    conn.close()
    return jsonify({'status': 'success', 'message': 'Semua wajah tidak dikenali berhasil dihapus'})

@app.route('/unknown-faces', methods=['GET'])
def unknown_faces():
    conn = sqlite3.connect(DB_PATH)
    c = conn.cursor()
    c.execute('SELECT filename, waktu FROM unknown_faces ORDER BY waktu DESC LIMIT 6')
    data = [{'filename': row[0], 'waktu': row[1]} for row in c.fetchall()]
    conn.close()
    return jsonify(data)

if __name__ == '__main__':
    socketio.run(app, host='0.0.0.0', port=5000)
