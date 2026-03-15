from flask import Flask, request, jsonify
import subprocess

app = Flask(__name__)

@app.route('/clip', methods=['POST'])
def clip_video():
    data = request.get_json()
    start_time = data.get('start_time')
    duration = data.get('duration')

    start_time = str(round(start_time, 3))  # millisecond precision
    duration = str(round(duration, 2))

    if not start_time or not duration:
        return jsonify({'error': 'Missing start_time or duration'}), 400

    try:
        print("Clipping a video.")
        print(data)
        
        subprocess.run(['./clip_and_send.sh', start_time, duration], check=True)
        return jsonify({'status': 'clip started.'}), 200
    except subprocess.CalledProcessError as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    # Accessible by 192.168.7.1
    app.run(host='0.0.0.0', port=9000)