#Flaskとrender_template（HTMLを表示させるための関数）をインポート
from flask import Flask,render_template
from flask import Flask, request, jsonify
from subprocess import Popen, PIPE

#Flaskオブジェクトの生成
app = Flask(__name__)

@app.route("/")
def index():
    return render_template("index.html")

@app.route('/start_audio_stream', methods=['GET'])
def start_audio_stream():
    port = request.args.get('port')
    ip_address = request.args.get('ip_addr')
    print('-------ipaddress-----------', request.args)

    if ip_address:
        cmd = f"rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone {ip_address} {port} | play -t raw -b 16 -c 1 -e s -r 44100 -"
    else:
        cmd = f"rec -t raw -b 16 -c 1 -e s -r 44100 - | ./phone {port} | play -t raw -b 16 -c 1 -e s -r 44100 -"
    
    # 実行可能ファイルを起動
    process = Popen(cmd, shell=True)

    return jsonify(message="Audio Stream started"), 200